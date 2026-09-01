#!/bin/sh
set -eu

if [ "$#" -ne 2 ]; then
  printf 'Usage: %s PORT FACTORY_IMAGE.bin\n' "$0" >&2
  exit 2
fi

PORT=$1
IMAGE=$2

if [ ! -f "$IMAGE" ]; then
  printf 'Factory image not found: %s\n' "$IMAGE" >&2
  exit 2
fi

printf 'This writes the complete 16MB factory image to %s.\n' "$PORT"
printf 'Press Ctrl-C within 5 seconds to cancel.\n'
sleep 5
esptool --port "$PORT" --baud 460800 write-flash 0x0 "$IMAGE"

