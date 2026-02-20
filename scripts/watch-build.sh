#!/usr/bin/env bash
set -u

if ! command -v rg >/dev/null 2>&1; then
  echo "Error: 'rg' (ripgrep) is required but not installed."
  exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: 'cmake' is required but not installed."
  exit 1
fi

WATCH_INTERVAL="${WATCH_INTERVAL:-1}"

if [ -z "${PLAYDATE_SDK_PATH:-}" ]; then
  DEFAULT_PLAYDATE_SDK_PATH="${DEFAULT_PLAYDATE_SDK_PATH:-$HOME/Developer/PlaydateSDK}"
  export PLAYDATE_SDK_PATH="$DEFAULT_PLAYDATE_SDK_PATH"
  if [ -d "$PLAYDATE_SDK_PATH" ]; then
    echo "PLAYDATE_SDK_PATH not set; defaulting to $PLAYDATE_SDK_PATH"
  else
    echo "PLAYDATE_SDK_PATH not set; defaulting to $PLAYDATE_SDK_PATH (directory not found)"
  fi
fi

snapshot() {
  {
    rg --files src 2>/dev/null
    printf '%s\n' CMakeLists.txt
  } | LC_ALL=C sort | while IFS= read -r file; do
    [ -e "$file" ] || continue
    mtime="$(stat -f '%m' "$file" 2>/dev/null || stat -c '%Y' "$file" 2>/dev/null || echo 0)"
    printf '%s %s\n' "$mtime" "$file"
  done | shasum | awk '{print $1}'
}

build_once() {
  echo "[$(date '+%H:%M:%S')] Building..."
  if cmake --build build; then
    echo "[$(date '+%H:%M:%S')] Build succeeded."
  else
    echo "[$(date '+%H:%M:%S')] Build failed (watch continues)."
  fi
}

if [ ! -d build ]; then
  echo "Build directory missing. Configuring with CMake first..."
  cmake -S . -B build || exit 1
fi

echo "Watching src/ and CMakeLists.txt (interval: ${WATCH_INTERVAL}s)."
echo "Press Ctrl+C to stop."

last_snapshot="$(snapshot)"
build_once

while true; do
  sleep "$WATCH_INTERVAL"
  current_snapshot="$(snapshot)"
  if [ "$current_snapshot" != "$last_snapshot" ]; then
    echo "[$(date '+%H:%M:%S')] Change detected."
    build_once
    last_snapshot="$current_snapshot"
    open bullethell.pdx
  fi
done
