#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="$ROOT_DIR/apps/control-panel"

echo "Installing control panel dependencies..."
npm --prefix "$APP_DIR" ci --legacy-peer-deps

echo "Linting control panel..."
npm --prefix "$APP_DIR" run lint

echo "Building control panel..."
npm --prefix "$APP_DIR" run build

echo "Control panel build checks complete."
