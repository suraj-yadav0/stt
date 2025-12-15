# STT - Speech to Text for Ubuntu Touch

An offline speech recognition app for Ubuntu Touch using the Vosk API. Converts speech to text without requiring an internet connection.

## Features

- **Fully Offline**: Works without internet connection after initial setup
- **Real-time Transcription**: See your words as you speak
- **Modern UI**: Dark theme with animated microphone button
- **Vosk-powered**: Uses the efficient Vosk speech recognition engine

## Requirements

- Ubuntu Touch device (arm64)
- Clickable 8.0.0 or later
- wget and unzip (for setup script)

## Quick Start

### 1. Setup (Download Dependencies)

For Ubuntu Touch (arm64):
```bash
./setup.sh arm64
```

For desktop testing (amd64):
```bash
./setup.sh amd64
```

### 2. Build

For Ubuntu Touch:
```bash
clickable build --arch arm64
```

For desktop:
```bash
clickable build --arch amd64
```

### 3. Test on Desktop

```bash
clickable desktop
```

### 4. Install on Device

```bash
clickable install --arch arm64
```

## Architecture

The app consists of:

- **SpeechRecognizer Plugin**: C++ plugin that:
  - Captures audio from the microphone using Qt Multimedia
  - Processes audio through Vosk for real-time transcription
  - Exposes QML-friendly API for the UI

- **QML UI**: Modern Lomiri-based interface with:
  - Animated microphone button
  - Live transcription display
  - Recording duration timer

- **Vosk Engine**: Offline speech recognition with small (~40MB) model

## Model

The default model is `vosk-model-small-en-us-0.15` (English US). To use a different language:

1. Download a model from [Vosk Models](https://alphacephei.com/vosk/models)
2. Extract it to the `model/` directory
3. Rebuild the app

Recommended models:
- English (US): `vosk-model-small-en-us-0.15` (~40MB)
- English (Indian): `vosk-model-small-en-in-0.4` (~36MB)
- Other languages: See [Vosk Models](https://alphacephei.com/vosk/models)

## Permissions

The app requires the following permissions:
- **audio**: For audio playback
- **microphone**: For recording speech

The app is **unconfined** to access the necessary audio APIs.

## Troubleshooting

### "No model found" error
Make sure the model is downloaded by running `./setup.sh <arch>` before building.

### Build fails with "libvosk.so not found"
Run `./setup.sh <arch>` to download the Vosk library for your target architecture.

### No audio on device
Ensure the app has microphone permissions. You may need to grant permissions manually on first run.

## License

MIT License

## Credits

- [Vosk](https://alphacephei.com/vosk/) - Offline speech recognition toolkit
- [UBports](https://ubports.com/) - Ubuntu Touch community
