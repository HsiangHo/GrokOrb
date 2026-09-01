#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PORT=${1:-/dev/cu.usbmodem1101}
FQBN='m5stack:esp32:m5stack_stopwatch:PSRAM=opi,FlashMode=qio,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,USBMode=hwcdc,CDCOnBoot=cdc'

if [ ! -f "$ROOT/build/GrokStopWatch.ino.bin" ]; then
  "$ROOT/scripts/build.sh"
fi

arduino-cli upload \
  --port "$PORT" \
  --fqbn "$FQBN" \
  --input-dir "$ROOT/build" \
  "$ROOT/GrokStopWatch"

