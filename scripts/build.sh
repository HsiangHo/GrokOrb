#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SKETCH="$ROOT/GrokStopWatch"
FQBN='m5stack:esp32:m5stack_stopwatch:PSRAM=opi,FlashMode=qio,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,USBMode=hwcdc,CDCOnBoot=cdc'

python3 "$ROOT/tools/generate_grok_assets.py"
mkdir -p "$ROOT/build"

arduino-cli compile \
  --fqbn "$FQBN" \
  --libraries "$ROOT/.deps" \
  --output-dir "$ROOT/build" \
  --warnings all \
  "$SKETCH"

