#include <QtQml>
#include <QtQml/QQmlContext>

#include "plugin.h"
#include "speech_recognizer.h"

void SpeechRecognizerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("SpeechRecognizer"));
    
    // Register SpeechRecognizer as a singleton
    qmlRegisterSingletonType<SpeechRecognizer>(
        uri, 1, 0, "SpeechRecognizer",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return new SpeechRecognizer();
        }
    );
}
