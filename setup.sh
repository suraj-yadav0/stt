#!/bin/bash

# Setup script for STT (Speech-to-Text) App
# This downloads the required Vosk library and model for the target architecture

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

ARCH="${1:-arm64}"
VOSK_VERSION="0.3.45"
ARCH_FILE="libs/vosk/.arch"

echo "Setting up STT app for architecture: $ARCH"

# Create directories
mkdir -p libs/vosk model

# Check current architecture of installed library
CURRENT_ARCH=""
if [ -f "$ARCH_FILE" ]; then
    CURRENT_ARCH=$(cat "$ARCH_FILE")
fi

# Determine if we need to download
NEED_DOWNLOAD=false
if [ ! -f "libs/vosk/libvosk.so" ]; then
    NEED_DOWNLOAD=true
elif [ "$CURRENT_ARCH" != "$ARCH" ]; then
    echo "Switching architecture from $CURRENT_ARCH to $ARCH"
    NEED_DOWNLOAD=true
elif [ "$2" = "--force" ]; then
    NEED_DOWNLOAD=true
fi

# Download Vosk library based on architecture
if [ "$NEED_DOWNLOAD" = true ]; then
    echo "Downloading Vosk library..."
    case "$ARCH" in
        arm64|aarch64)
            VOSK_URL="https://github.com/alphacep/vosk-api/releases/download/v${VOSK_VERSION}/vosk-linux-aarch64-${VOSK_VERSION}.zip"
            VOSK_DIR="vosk-linux-aarch64-${VOSK_VERSION}"
            ;;
        armhf|armv7l)
            VOSK_URL="https://github.com/alphacep/vosk-api/releases/download/v${VOSK_VERSION}/vosk-linux-armv7l-${VOSK_VERSION}.zip"
            VOSK_DIR="vosk-linux-armv7l-${VOSK_VERSION}"
            ;;
        amd64|x86_64)
            VOSK_URL="https://github.com/alphacep/vosk-api/releases/download/v${VOSK_VERSION}/vosk-linux-x86_64-${VOSK_VERSION}.zip"
            VOSK_DIR="vosk-linux-x86_64-${VOSK_VERSION}"
            ;;
        *)
            echo "Error: Unsupported architecture: $ARCH"
            echo "Supported architectures: arm64, armhf, amd64"
            exit 1
            ;;
    esac
    
    # Remove any existing library for different architecture
    rm -f libs/vosk/libvosk.so
    
    wget -q "$VOSK_URL" -O /tmp/vosk.zip
    unzip -q /tmp/vosk.zip -d /tmp/vosk
    cp "/tmp/vosk/${VOSK_DIR}/libvosk.so" libs/vosk/
    echo "$ARCH" > "$ARCH_FILE"
    rm -rf /tmp/vosk /tmp/vosk.zip
    echo "Vosk library downloaded for $ARCH"
else
    echo "Vosk library already exists for $ARCH, skipping (use --force to re-download)"
fi

# Download Vosk model if not present
echo "Checking Vosk model..."
if [ ! -d "model/vosk-model-small-en-us-0.15" ]; then
    echo "Downloading Vosk model (this may take a while)..."
    wget -q "https://alphacephei.com/vosk/models/vosk-model-small-en-us-0.15.zip" -O /tmp/model.zip
    unzip -q -o /tmp/model.zip -d model/
    rm -f /tmp/model.zip
    echo "Vosk model downloaded"
else
    echo "Vosk model already exists"
fi

echo ""
echo "Setup complete!"
echo ""
echo "To build for Ubuntu Touch ($ARCH):"
echo "  clickable build --arch $ARCH"
echo ""
echo "To test on desktop:"
echo "  # First setup for amd64 if needed"
echo "  ./setup.sh amd64"
echo "  clickable desktop"
echo ""
echo "To install on device:"
echo "  clickable install --arch $ARCH"
