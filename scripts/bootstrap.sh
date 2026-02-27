#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Initializing git submodules..."
git -C "$ROOT_DIR" submodule update --init --recursive

if command -v npm >/dev/null 2>&1; then
  echo "Installing control panel dependencies..."
  npm --prefix "$ROOT_DIR/apps/control-panel" ci --legacy-peer-deps

  echo "Installing installer dependencies..."
  npm --prefix "$ROOT_DIR/apps/installer" ci
else
  echo "npm not found; skipping web dependency installs."
fi

if ! command -v pio >/dev/null 2>&1; then
  echo "PlatformIO not found. Install with: pip install platformio"
fi

echo "Bootstrap complete."
