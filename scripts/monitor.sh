#!/bin/sh
set -eu

PORT=${1:-/dev/cu.usbmodem1101}
exec arduino-cli monitor --port "$PORT" --config baudrate=115200

