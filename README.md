# Bullethell (Playdate C)

Crank-aimed 2D bullethell prototype for the Playdate console, written with the Playdate C API.

## Project Summary

- D-pad movement in a large square arena with camera follow.
- Crank controls firing direction; weapon auto-fires.
- Enemy and powerup spawning with minimum distance from player.
- Passive powerups apply immediately (move speed, fire rate, multishot).
- Single-slot active powerup (triggered with A button).
- Basic HUD with HP, wave, score, and active power status.

## Requirements

- Playdate SDK installed.
- `cmake` in `PATH`.
- For device builds: GNU Arm Embedded toolchain (`arm-none-eabi-gcc`) in `PATH`.
- For watch script: `rg` (ripgrep) in `PATH`.

Set SDK path (if not already set):

```bash
export PLAYDATE_SDK_PATH="$HOME/Developer/PlaydateSDK"
```

Both scripts also default to that SDK path when `PLAYDATE_SDK_PATH` is missing.

## Build for Simulator

Configure and build simulator target:

```bash
cmake -S . -B build
cmake --build build
```

This generates a simulator `.pdx` bundle in the project root (typically `bullethell.pdx`).

## Watch and Rebuild on Changes

Use the watcher script to rebuild when source files change:

```bash
./scripts/watch-build.sh
```

The watcher also auto-opens/reloads the simulator bundle after rebuilds.

Optional polling interval (seconds):

```bash
WATCH_INTERVAL=2 ./scripts/watch-build.sh
```

## Build Playdate Device Bundle (`pdex.bin`)

Build an ARM Playdate-compatible bundle:

```bash
./scripts/build-pdx.sh
```

Default output is usually:

- `bullethell_DEVICE.pdx`
- `bullethell_DEVICE.pdx/pdex.bin`

You can override output/game name if needed:

```bash
GAME_NAME=bullethell BUILD_DIR=build-device ./scripts/build-pdx.sh
```

## Running

- Simulator: if using `watch-build.sh`, the `.pdx` is auto-opened/reloaded; otherwise open the generated `.pdx` bundle manually.
- Device: sideload the generated `.pdx` containing `pdex.bin`.
