#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DECODER_JAR="${DECODER_JAR:-$SCRIPT_DIR/EspStackTraceDecoder.jar}"

usage() {
  cat <<'USAGE'
Usage:
  esp32decoder.sh <addr2line_path> <firmware_elf> <dump_file>

Environment:
  DECODER_JAR   Optional path to EspStackTraceDecoder.jar.
                Default: tools/esp-stacktrace-decoder/EspStackTraceDecoder.jar

Example:
  ./tools/esp-stacktrace-decoder/esp32decoder.sh \
    ~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line \
    .pio/build/esp32dev/firmware.elf \
    ./crash-dump.txt
USAGE
}

if [ "$#" -ne 3 ]; then
  usage
  exit 1
fi

ADDR2LINE="$1"
ELF_FILE="$2"
DUMP_FILE="$3"

if [ ! -f "$DECODER_JAR" ]; then
  echo "Decoder jar not found: $DECODER_JAR"
  exit 1
fi

if [ ! -x "$ADDR2LINE" ]; then
  echo "addr2line binary not executable: $ADDR2LINE"
  exit 1
fi

if [ ! -f "$ELF_FILE" ]; then
  echo "Firmware ELF not found: $ELF_FILE"
  exit 1
fi

if [ ! -f "$DUMP_FILE" ]; then
  echo "Dump file not found: $DUMP_FILE"
  exit 1
fi

java -jar "$DECODER_JAR" "$ADDR2LINE" "$ELF_FILE" "$DUMP_FILE"
