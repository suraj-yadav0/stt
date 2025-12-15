#ifndef SPEECHRECOGNIZER_PLUGIN_H
#define SPEECHRECOGNIZER_PLUGIN_H

#include <QQmlExtensionPlugin>

class SpeechRecognizerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // SPEECHRECOGNIZER_PLUGIN_H
