# GrokOrb StopWatch Firmware

Native ESP32-S3 firmware for the M5Stack StopWatch. It renders a full-screen
black character with centered white eyes directly through M5GFX; no JavaScript
or SVG runtime is used on the device.

## Repository layout

- `GrokStopWatch/`: Arduino sketch and native renderer.
- `assets/geometry-data.js`: source geometry used to regenerate the compact
  C++ eye data.
- `scripts/`: toolchain setup, build, upload, monitor, and restore helpers.
- `tools/`: deterministic geometry generator.
- `backups/`: factory-backup documentation. Binary images are ignored by Git.

## Pinned toolchain

- M5Stack Arduino core: `3.3.9`
- M5Unified commit: `8530f5377d782e4a25a6c482de2e71c3f75ca8eb`
- M5GFX commit: `d91077b9a607b59404e4e4a49f775c792bfae382`
- Board: `m5stack:esp32:m5stack_stopwatch`
- Flash: 16MB, OPI PSRAM enabled

StopWatch support landed after the latest tagged M5Unified/M5GFX releases, so
the bootstrap script uses exact official Git commits instead of release tags.

## Verified hardware deployment

Flashed and validated on September 1, 2026:

```text
Device: ESP32-S3 revision 0.2
MAC: 28:84:85:43:a0:4c
Display: 468x468
Flash: 16777216 bytes
PSRAM: 8388608 bytes
Detected: touch, IMU, RTC, microphone, speaker
Firmware: 584736 bytes
Runtime: 29-30 FPS, stable heap around 334 KB
```

The original 16MB factory image is documented in `backups/README.md` and can
be restored with `scripts/restore_factory.sh`.

## Build and upload

Run these commands from the repository root:

```sh
./scripts/bootstrap.sh
./scripts/build.sh
./scripts/upload.sh /dev/cu.usbmodem1101
./scripts/monitor.sh /dev/cu.usbmodem1101
```

The upload script never erases the whole chip. Save a full factory image under
`backups/` before the first upload; binary backups are ignored by Git.

## Controls

- Touch and drag: move the centered eyes with short haptic feedback.
- Tap: happy hop; double tap: excited spin and bounce.
- Long press: suspicious stare; release: angry shake.
- Flick left/right: playful spin in the gesture direction.
- Flick up: surprised hop; flick down: drowsy reaction.
- Button A: cycle through `sleeping`, `waking`, `idle`, `listening`,
  `thinking`, `searching`, and `working`.
- Button B: enter a persistent auto-showcase alternating `idle` with lifecycle
  and expression animations.
- IMU: subtly steers the idle gaze.

Buttons and touch gestures use short, non-blocking vibration pulses. The
extracted character geometry in this repository is for personal study; replace
the name and character assets before distributing or commercializing it.

## Acknowledgements

Special thanks to [blessonism/grok-icon-study](https://github.com/blessonism/grok-icon-study)
for the animation study and motion references that inspired this firmware.
