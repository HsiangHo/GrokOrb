#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
DEPS="$ROOT/.deps"
M5UNIFIED_COMMIT=8530f5377d782e4a25a6c482de2e71c3f75ca8eb
M5GFX_COMMIT=d91077b9a607b59404e4e4a49f775c792bfae382

arduino-cli config add board_manager.additional_urls \
  https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
arduino-cli core update-index
arduino-cli core install m5stack:esp32@3.3.9

mkdir -p "$DEPS"

clone_pinned() {
  name=$1
  url=$2
  commit=$3
  path="$DEPS/$name"

  if [ ! -d "$path/.git" ]; then
    git clone --filter=blob:none "$url" "$path"
  fi

  git -C "$path" fetch --depth 1 origin "$commit"
  git -C "$path" checkout --detach "$commit"

  actual=$(git -C "$path" rev-parse HEAD)
  if [ "$actual" != "$commit" ]; then
    printf 'Expected %s at %s, got %s\n' "$name" "$commit" "$actual" >&2
    exit 1
  fi
}

clone_pinned M5GFX https://github.com/m5stack/M5GFX.git "$M5GFX_COMMIT"
clone_pinned M5Unified https://github.com/m5stack/M5Unified.git "$M5UNIFIED_COMMIT"

python3 "$ROOT/tools/generate_grok_assets.py"

printf '\nToolchain ready.\n'

