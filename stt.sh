#!/bin/bash
# Wrapper script to set LD_LIBRARY_PATH for bundled libraries
APP_DIR="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="${APP_DIR}/libs/vosk:${LD_LIBRARY_PATH}"
exec "${APP_DIR}/stt.bin" "$@"
