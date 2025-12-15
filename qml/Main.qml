import QtQuick 2.12
import Lomiri.Components 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0

import SpeechRecognizer 1.0

MainView {
    id: root
    objectName: 'mainView'
    applicationName: 'stt.surajyadav'
    automaticOrientation: true

    width: units.gu(45)
    height: units.gu(75)

    // Theme colors
    readonly property color primaryColor: "#6C63FF"
    readonly property color primaryDarkColor: "#5248CC"
    readonly property color accentColor: "#FF6584"
    readonly property color backgroundColor: "#1A1A2E"
    readonly property color surfaceColor: "#16213E"
    readonly property color textColor: "#FFFFFF"
    readonly property color textSecondaryColor: "#B8B8D1"

    Page {
        id: mainPage
        anchors.fill: parent

        // Dark background
        Rectangle {
            anchors.fill: parent
            color: backgroundColor
        }

        header: PageHeader {
            id: header
            title: i18n.tr('Speech to Text')
            
            StyleHints {
                foregroundColor: textColor
                backgroundColor: surfaceColor
            }

            trailingActionBar.actions: [
                Action {
                    iconName: "delete"
                    text: i18n.tr("Clear")
                    onTriggered: {
                        SpeechRecognizer.clearTranscription()
                        partialText.text = ""
                    }
                }
            ]
        }

        ColumnLayout {
            anchors {
                top: header.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                margins: units.gu(2)
            }
            spacing: units.gu(2)

            // Status indicator
            Label {
                id: statusLabel
                Layout.alignment: Qt.AlignHCenter
                text: SpeechRecognizer.status
                color: textSecondaryColor
                font.pixelSize: units.gu(1.5)
            }

            // Model status warning
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: units.gu(5)
                color: "#FF6584"
                radius: units.gu(1)
                visible: !SpeechRecognizer.isModelLoaded

                Label {
                    anchors.centerIn: parent
                    text: i18n.tr("No model loaded. Please install a Vosk model.")
                    color: textColor
                    font.pixelSize: units.gu(1.5)
                }
            }

            // Transcription area
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: surfaceColor
                radius: units.gu(1.5)
                border.color: primaryColor
                border.width: 1

                Flickable {
                    id: transcriptionFlickable
                    anchors {
                        fill: parent
                        margins: units.gu(2)
                    }
                    contentHeight: transcriptionColumn.height
                    clip: true

                    ColumnLayout {
                        id: transcriptionColumn
                        width: parent.width
                        spacing: units.gu(1)

                        // Final transcription
                        Label {
                            id: transcriptionText
                            Layout.fillWidth: true
                            text: SpeechRecognizer.transcription || i18n.tr("Tap the microphone and start speaking...")
                            color: SpeechRecognizer.transcription ? textColor : textSecondaryColor
                            font.pixelSize: units.gu(2)
                            wrapMode: Text.WordWrap
                            font.italic: !SpeechRecognizer.transcription
                        }

                        // Partial result (live)
                        Label {
                            id: partialText
                            Layout.fillWidth: true
                            visible: text.length > 0
                            color: primaryColor
                            font.pixelSize: units.gu(1.8)
                            font.italic: true
                            wrapMode: Text.WordWrap
                            opacity: 0.8
                        }
                    }

                    // Auto-scroll to bottom
                    onContentHeightChanged: {
                        if (contentHeight > height) {
                            contentY = contentHeight - height
                        }
                    }
                }

                // Scroll indicator
                Rectangle {
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        margins: units.gu(0.5)
                    }
                    width: units.gu(0.5)
                    radius: width / 2
                    color: primaryColor
                    opacity: 0.3
                    visible: transcriptionFlickable.contentHeight > transcriptionFlickable.height
                }
            }

            // Recording duration
            Label {
                id: durationLabel
                Layout.alignment: Qt.AlignHCenter
                text: formatDuration(SpeechRecognizer.recordingDuration)
                color: SpeechRecognizer.isRecording ? accentColor : textSecondaryColor
                font.pixelSize: units.gu(2)
                visible: SpeechRecognizer.isRecording || SpeechRecognizer.recordingDuration > 0

                function formatDuration(seconds) {
                    var mins = Math.floor(seconds / 60)
                    var secs = seconds % 60
                    return mins.toString().padStart(2, '0') + ':' + secs.toString().padStart(2, '0')
                }
            }

            // Microphone button
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: units.gu(12)
                Layout.preferredHeight: units.gu(12)

                // Pulsing circle animation (behind the button)
                Rectangle {
                    id: pulseCircle
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    radius: width / 2
                    color: "transparent"
                    border.color: accentColor
                    border.width: units.gu(0.3)
                    opacity: 0
                    scale: 1

                    SequentialAnimation {
                        id: pulseAnimation
                        running: SpeechRecognizer.isRecording
                        loops: Animation.Infinite

                        ParallelAnimation {
                            NumberAnimation {
                                target: pulseCircle
                                property: "scale"
                                from: 1
                                to: 1.5
                                duration: 1000
                                easing.type: Easing.OutQuad
                            }
                            NumberAnimation {
                                target: pulseCircle
                                property: "opacity"
                                from: 0.8
                                to: 0
                                duration: 1000
                                easing.type: Easing.OutQuad
                            }
                        }
                    }
                }

                // Main microphone button
                Rectangle {
                    id: micButton
                    anchors.centerIn: parent
                    width: units.gu(10)
                    height: units.gu(10)
                    radius: width / 2
                    color: SpeechRecognizer.isRecording ? accentColor : primaryColor

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }

                    Behavior on scale {
                        NumberAnimation { duration: 100 }
                    }

                    // Microphone icon
                    Icon {
                        anchors.centerIn: parent
                        width: units.gu(4)
                        height: units.gu(4)
                        name: SpeechRecognizer.isRecording ? "media-playback-stop" : "audio-input-microphone-symbolic"
                        color: textColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: SpeechRecognizer.isModelLoaded
                        
                        onPressed: {
                            micButton.scale = 0.95
                        }
                        
                        onReleased: {
                            micButton.scale = 1.0
                        }
                        
                        onClicked: {
                            if (SpeechRecognizer.isRecording) {
                                SpeechRecognizer.stopRecording()
                                partialText.text = ""
                            } else {
                                SpeechRecognizer.startRecording()
                            }
                        }
                    }

                    // Shadow effect
                    layer.enabled: true
                    layer.effect: Item {
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: -units.gu(0.5)
                            radius: width / 2
                            color: "transparent"
                        }
                    }
                }
            }

            // Instructions
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: SpeechRecognizer.isRecording 
                    ? i18n.tr("Tap to stop recording") 
                    : i18n.tr("Tap microphone to start")
                color: textSecondaryColor
                font.pixelSize: units.gu(1.5)
            }

            Item {
                Layout.preferredHeight: units.gu(2)
            }
        }
    }

    // Connect to SpeechRecognizer signals
    Connections {
        target: SpeechRecognizer

        function onPartialResult(text) {
            partialText.text = text
        }

        function onFinalResult(text) {
            partialText.text = ""
        }

        function onErrorOccurred(error) {
            console.log("Speech recognition error:", error)
        }
    }

    Component.onCompleted: {
        console.log("STT App started")
        console.log("Model loaded:", SpeechRecognizer.isModelLoaded)
    }
}
