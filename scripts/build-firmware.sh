#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FW_DIR="$ROOT_DIR/firmware/esp"
ENV_NAME="${1:-esp32dev}"
TARGET="${2:-compiledb}"

echo "Building firmware environment '$ENV_NAME' (target: '$TARGET')..."
(
  cd "$FW_DIR"
  pio run -e "$ENV_NAME" -t "$TARGET"
)

echo "Firmware build check complete."
