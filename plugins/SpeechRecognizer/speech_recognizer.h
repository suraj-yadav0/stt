#ifndef SPEECHRECOGNIZER_H
#define SPEECHRECOGNIZER_H

#include <QObject>
#include <QAudioInput>
#include <QAudioFormat>
#include <QIODevice>
#include <QBuffer>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>

// Forward declarations for Vosk types
struct VoskModel;
struct VoskRecognizer;

class SpeechRecognizer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY isRecordingChanged)
    Q_PROPERTY(bool isModelLoaded READ isModelLoaded NOTIFY isModelLoadedChanged)
    Q_PROPERTY(QString transcription READ transcription NOTIFY transcriptionChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int recordingDuration READ recordingDuration NOTIFY recordingDurationChanged)

public:
    explicit SpeechRecognizer(QObject *parent = nullptr);
    ~SpeechRecognizer();

    bool isRecording() const { return m_isRecording; }
    bool isModelLoaded() const { return m_isModelLoaded; }
    QString transcription() const { return m_transcription; }
    QString status() const { return m_status; }
    int recordingDuration() const { return m_recordingDuration; }

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void clearTranscription();
    Q_INVOKABLE bool loadModel(const QString &modelPath = QString());

signals:
    void isRecordingChanged();
    void isModelLoadedChanged();
    void transcriptionChanged();
    void statusChanged();
    void recordingDurationChanged();
    void partialResult(const QString &text);
    void finalResult(const QString &text);
    void errorOccurred(const QString &error);

private slots:
    void processAudioData();
    void updateRecordingDuration();

private:
    void initAudio();
    void processBuffer(const QByteArray &buffer);
    QString findModelPath();
    void setStatus(const QString &status);

    // Audio components (Qt5 style)
    QAudioInput *m_audioInput = nullptr;
    QIODevice *m_audioDevice = nullptr;
    QBuffer m_audioBuffer;
    QAudioFormat m_audioFormat;

    // Vosk components
    VoskModel *m_model = nullptr;
    VoskRecognizer *m_recognizer = nullptr;

    // State
    bool m_isRecording = false;
    bool m_isModelLoaded = false;
    QString m_transcription;
    QString m_status;
    int m_recordingDuration = 0;

    // Timers
    QTimer m_processTimer;
    QTimer m_durationTimer;
    QElapsedTimer m_elapsedTimer;

    // Audio settings
    static constexpr int SAMPLE_RATE = 16000;
    static constexpr int CHANNELS = 1;
    static constexpr int SAMPLE_SIZE = 16;
};

#endif // SPEECHRECOGNIZER_H
