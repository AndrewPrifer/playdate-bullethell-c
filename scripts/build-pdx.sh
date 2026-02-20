#!/usr/bin/env bash
set -euo pipefail

if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: 'cmake' is required but not installed."
  exit 1
fi

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  echo "Error: 'arm-none-eabi-gcc' is required for Playdate device builds."
  exit 1
fi

if [ -z "${PLAYDATE_SDK_PATH:-}" ]; then
  DEFAULT_PLAYDATE_SDK_PATH="${DEFAULT_PLAYDATE_SDK_PATH:-$HOME/Developer/PlaydateSDK}"
  export PLAYDATE_SDK_PATH="$DEFAULT_PLAYDATE_SDK_PATH"
  echo "PLAYDATE_SDK_PATH not set; defaulting to $PLAYDATE_SDK_PATH"
fi

if [ ! -d "$PLAYDATE_SDK_PATH" ]; then
  echo "Error: PLAYDATE_SDK_PATH does not exist: $PLAYDATE_SDK_PATH"
  exit 1
fi

TOOLCHAIN_FILE="$PLAYDATE_SDK_PATH/C_API/buildsupport/arm.cmake"
if [ ! -f "$TOOLCHAIN_FILE" ]; then
  echo "Error: missing toolchain file: $TOOLCHAIN_FILE"
  exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build-device}"
GAME_NAME="${GAME_NAME:-bullethell}"
OUTPUT_PDX="$PROJECT_ROOT/${GAME_NAME}.pdx"
OUTPUT_PDX_DEVICE="$PROJECT_ROOT/${GAME_NAME}_DEVICE.pdx"

echo "Configuring device build in: $BUILD_DIR"
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
  -DCMAKE_BUILD_TYPE=Release

echo "Building device target..."
cmake --build "$BUILD_DIR" --config Release

FINAL_PDX=""
if [ -d "$OUTPUT_PDX_DEVICE" ]; then
  FINAL_PDX="$OUTPUT_PDX_DEVICE"
elif [ -d "$OUTPUT_PDX" ]; then
  FINAL_PDX="$OUTPUT_PDX"
fi

if [ -z "$FINAL_PDX" ]; then
  echo "Error: expected bundle not found:"
  echo "  $OUTPUT_PDX_DEVICE"
  echo "  $OUTPUT_PDX"
  exit 1
fi

if [ ! -f "$FINAL_PDX/pdex.bin" ]; then
  echo "Error: build finished but no Playdate device binary found at:"
  echo "  $FINAL_PDX/pdex.bin"
  exit 1
fi

echo "Success: $FINAL_PDX"
echo "Device binary: $FINAL_PDX/pdex.bin"
