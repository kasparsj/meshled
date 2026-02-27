#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <version-tag> [output-dir]"
  echo "Example: $0 v0.1.0"
  exit 1
fi

VERSION_TAG="$1"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FW_DIR="$ROOT_DIR/firmware/esp"
OUT_DIR="${2:-$ROOT_DIR/dist/installer-firmware/$VERSION_TAG}"

if ! command -v pio >/dev/null 2>&1; then
  echo "PlatformIO not found. Install with: pip install platformio"
  exit 1
fi

if ! python3 -m esptool version >/dev/null 2>&1; then
  echo "Python esptool not found. Install with: pip install esptool"
  exit 1
fi

BOOT_APP0="$HOME/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
if [ ! -f "$BOOT_APP0" ]; then
  echo "Missing boot_app0.bin at: $BOOT_APP0"
  echo "Run one PlatformIO firmware build first to download framework assets."
  exit 1
fi

mkdir -p "$OUT_DIR"

generate_merged_firmware() {
  local env_name="$1"
  local chip_name="$2"
  local output_name="$3"
  local build_dir="$FW_DIR/.pio/build/$env_name"

  echo "Building firmware environment: $env_name"
  pio run -d "$FW_DIR" -e "$env_name"

  local bootloader_bin="$build_dir/bootloader.bin"
  local partitions_bin="$build_dir/partitions.bin"
  local app_bin="$build_dir/firmware.bin"
  local merged_out="$OUT_DIR/$output_name"

  if [ ! -f "$bootloader_bin" ] || [ ! -f "$partitions_bin" ] || [ ! -f "$app_bin" ]; then
    echo "Missing one or more build outputs in $build_dir"
    exit 1
  fi

  echo "Merging image: $merged_out"
  python3 -m esptool \
    --chip "$chip_name" \
    merge_bin \
    --output "$merged_out" \
    --flash_mode keep \
    --flash_freq keep \
    --flash_size keep \
    0x1000 "$bootloader_bin" \
    0x8000 "$partitions_bin" \
    0xe000 "$BOOT_APP0" \
    0x10000 "$app_bin"
}

generate_merged_firmware "esp32dev-release" "esp32" "meshled-${VERSION_TAG}-esp32.bin"
generate_merged_firmware "esp32-s3-devkitc-1-release" "esp32s3" "meshled-${VERSION_TAG}-esp32s3.bin"

echo "Installer firmware artifacts exported to: $OUT_DIR"
