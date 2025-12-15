#include "speech_recognizer.h"
#include "vosk_api.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QAudioDeviceInfo>
#include <QCoreApplication>

SpeechRecognizer::SpeechRecognizer(QObject *parent)
    : QObject(parent)
{
    // Set up audio format for Vosk (16kHz, mono, 16-bit PCM)
    m_audioFormat.setSampleRate(SAMPLE_RATE);
    m_audioFormat.setChannelCount(CHANNELS);
    m_audioFormat.setSampleSize(SAMPLE_SIZE);
    m_audioFormat.setCodec("audio/pcm");
    m_audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    m_audioFormat.setSampleType(QAudioFormat::SignedInt);

    // Set up process timer to handle audio data periodically
    m_processTimer.setInterval(100); // Process every 100ms
    connect(&m_processTimer, &QTimer::timeout, this, &SpeechRecognizer::processAudioData);

    // Set up duration timer
    m_durationTimer.setInterval(1000);
    connect(&m_durationTimer, &QTimer::timeout, this, &SpeechRecognizer::updateRecordingDuration);

    // Suppress Vosk debug output
    vosk_set_log_level(-1);

    setStatus("Ready");
    
    // Try to load model automatically
    QString modelPath = findModelPath();
    if (!modelPath.isEmpty()) {
        loadModel(modelPath);
    }
}

SpeechRecognizer::~SpeechRecognizer()
{
    stopRecording();
    
    if (m_recognizer) {
        vosk_recognizer_free(m_recognizer);
        m_recognizer = nullptr;
    }
    
    if (m_model) {
        vosk_model_free(m_model);
        m_model = nullptr;
    }
}

QString SpeechRecognizer::findModelPath()
{
    QStringList searchPaths;
    
    // App installation directory (for bundled model)
    QString appDir = QCoreApplication::applicationDirPath();
    searchPaths << appDir + "/model";
    searchPaths << appDir + "/../model";
    searchPaths << appDir + "/../share/stt.surajyadav/model";
    
    // User data directory
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    searchPaths << dataDir + "/model";
    
    // Common model names to look for
    QStringList modelNames = {
        "vosk-model-small-en-us-0.15",
        "vosk-model-small-en-in-0.4",
        "model"
    };
    
    for (const QString &path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            // Check if this directory itself is a model
            if (QFile::exists(path + "/am/final.mdl") || 
                QFile::exists(path + "/graph/HCLG.fst")) {
                qDebug() << "Found model at:" << path;
                return path;
            }
            
            // Look for named model subdirectories
            for (const QString &modelName : modelNames) {
                QString modelPath = path + "/" + modelName;
                if (QDir(modelPath).exists()) {
                    qDebug() << "Found model at:" << modelPath;
                    return modelPath;
                }
            }
        }
    }
    
    qDebug() << "No model found in search paths:" << searchPaths;
    return QString();
}

bool SpeechRecognizer::loadModel(const QString &modelPath)
{
    QString path = modelPath.isEmpty() ? findModelPath() : modelPath;
    
    if (path.isEmpty()) {
        emit errorOccurred("No speech recognition model found. Please install a Vosk model.");
        setStatus("No model found");
        return false;
    }
    
    setStatus("Loading model...");
    qDebug() << "Loading Vosk model from:" << path;
    
    // Free existing model if any
    if (m_recognizer) {
        vosk_recognizer_free(m_recognizer);
        m_recognizer = nullptr;
    }
    if (m_model) {
        vosk_model_free(m_model);
        m_model = nullptr;
    }
    
    // Load the model
    m_model = vosk_model_new(path.toUtf8().constData());
    
    if (!m_model) {
        emit errorOccurred("Failed to load speech recognition model from: " + path);
        setStatus("Model load failed");
        m_isModelLoaded = false;
        emit isModelLoadedChanged();
        return false;
    }
    
    // Create recognizer
    m_recognizer = vosk_recognizer_new(m_model, static_cast<float>(SAMPLE_RATE));
    
    if (!m_recognizer) {
        vosk_model_free(m_model);
        m_model = nullptr;
        emit errorOccurred("Failed to create speech recognizer");
        setStatus("Recognizer creation failed");
        m_isModelLoaded = false;
        emit isModelLoadedChanged();
        return false;
    }
    
    // Enable word timing (optional, for better UX)
    vosk_recognizer_set_words(m_recognizer, 1);
    
    m_isModelLoaded = true;
    emit isModelLoadedChanged();
    setStatus("Ready");
    qDebug() << "Model loaded successfully";
    
    return true;
}

void SpeechRecognizer::initAudio()
{
    // Clean up existing audio input
    if (m_audioInput) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = nullptr;
    }
    
    // Get default audio input device
    QAudioDeviceInfo inputDevice = QAudioDeviceInfo::defaultInputDevice();
    
    if (inputDevice.isNull()) {
        emit errorOccurred("No audio input device found");
        setStatus("No microphone");
        return;
    }
    
    qDebug() << "Using audio device:" << inputDevice.deviceName();
    
    // Check if format is supported
    if (!inputDevice.isFormatSupported(m_audioFormat)) {
        qWarning() << "Audio format not supported, trying nearest format";
        m_audioFormat = inputDevice.nearestFormat(m_audioFormat);
        qDebug() << "Using format: rate=" << m_audioFormat.sampleRate() 
                 << "channels=" << m_audioFormat.channelCount()
                 << "size=" << m_audioFormat.sampleSize();
    }
    
    m_audioInput = new QAudioInput(inputDevice, m_audioFormat, this);
}

void SpeechRecognizer::startRecording()
{
    if (m_isRecording) {
        qDebug() << "Already recording";
        return;
    }
    
    if (!m_isModelLoaded) {
        emit errorOccurred("Model not loaded. Please load a model first.");
        return;
    }
    
    // Reset recognizer for new session
    if (m_recognizer) {
        vosk_recognizer_reset(m_recognizer);
    }
    
    initAudio();
    
    if (!m_audioInput) {
        emit errorOccurred("Failed to initialize audio input");
        return;
    }
    
    // Clear the buffer
    m_audioBuffer.close();
    m_audioBuffer.setData(QByteArray());
    m_audioBuffer.open(QIODevice::ReadWrite);
    
    // Start audio capture
    m_audioDevice = m_audioInput->start();
    
    if (!m_audioDevice) {
        emit errorOccurred("Failed to start audio capture");
        setStatus("Audio error");
        return;
    }
    
    // Connect to read audio data
    connect(m_audioDevice, &QIODevice::readyRead, this, [this]() {
        if (m_audioDevice) {
            QByteArray data = m_audioDevice->readAll();
            if (!data.isEmpty()) {
                m_audioBuffer.write(data);
            }
        }
    });
    
    m_isRecording = true;
    m_recordingDuration = 0;
    m_elapsedTimer.start();
    m_processTimer.start();
    m_durationTimer.start();
    
    emit isRecordingChanged();
    emit recordingDurationChanged();
    setStatus("Listening...");
    
    qDebug() << "Recording started";
}

void SpeechRecognizer::stopRecording()
{
    if (!m_isRecording) {
        return;
    }
    
    m_processTimer.stop();
    m_durationTimer.stop();
    
    if (m_audioInput) {
        m_audioInput->stop();
        delete m_audioInput;
        m_audioInput = nullptr;
    }
    
    m_audioDevice = nullptr;
    
    // Process any remaining audio
    if (m_audioBuffer.size() > 0) {
        m_audioBuffer.seek(0);
        QByteArray remainingData = m_audioBuffer.readAll();
        if (!remainingData.isEmpty()) {
            processBuffer(remainingData);
        }
    }
    
    // Get final result
    if (m_recognizer) {
        const char *result = vosk_recognizer_final_result(m_recognizer);
        if (result) {
            QJsonDocument doc = QJsonDocument::fromJson(QByteArray(result));
            QJsonObject obj = doc.object();
            QString text = obj.value("text").toString().trimmed();
            
            if (!text.isEmpty()) {
                if (!m_transcription.isEmpty()) {
                    m_transcription += " ";
                }
                m_transcription += text;
                emit transcriptionChanged();
                emit finalResult(text);
            }
        }
    }
    
    m_audioBuffer.close();
    m_isRecording = false;
    
    emit isRecordingChanged();
    setStatus("Ready");
    
    qDebug() << "Recording stopped";
}

void SpeechRecognizer::clearTranscription()
{
    m_transcription.clear();
    emit transcriptionChanged();
}

void SpeechRecognizer::processAudioData()
{
    if (!m_isRecording || !m_recognizer) {
        return;
    }
    
    // Read available data from buffer
    qint64 currentPos = m_audioBuffer.pos();
    m_audioBuffer.seek(0);
    QByteArray data = m_audioBuffer.readAll();
    
    if (data.isEmpty()) {
        return;
    }
    
    // Clear the buffer for new data
    m_audioBuffer.close();
    m_audioBuffer.setData(QByteArray());
    m_audioBuffer.open(QIODevice::ReadWrite);
    
    processBuffer(data);
}

void SpeechRecognizer::processBuffer(const QByteArray &buffer)
{
    if (!m_recognizer || buffer.isEmpty()) {
        return;
    }
    
    // Feed audio data to Vosk
    int accepted = vosk_recognizer_accept_waveform(
        m_recognizer, 
        buffer.constData(), 
        buffer.size()
    );
    
    if (accepted) {
        // We have a complete utterance
        const char *result = vosk_recognizer_result(m_recognizer);
        if (result) {
            QJsonDocument doc = QJsonDocument::fromJson(QByteArray(result));
            QJsonObject obj = doc.object();
            QString text = obj.value("text").toString().trimmed();
            
            if (!text.isEmpty()) {
                if (!m_transcription.isEmpty()) {
                    m_transcription += " ";
                }
                m_transcription += text;
                emit transcriptionChanged();
                emit finalResult(text);
            }
        }
    } else {
        // Get partial result for live feedback
        const char *partial = vosk_recognizer_partial_result(m_recognizer);
        if (partial) {
            QJsonDocument doc = QJsonDocument::fromJson(QByteArray(partial));
            QJsonObject obj = doc.object();
            QString text = obj.value("partial").toString().trimmed();
            
            if (!text.isEmpty()) {
                emit partialResult(text);
            }
        }
    }
}

void SpeechRecognizer::updateRecordingDuration()
{
    m_recordingDuration = static_cast<int>(m_elapsedTimer.elapsed() / 1000);
    emit recordingDurationChanged();
}

void SpeechRecognizer::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}
