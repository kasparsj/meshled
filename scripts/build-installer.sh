#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="$ROOT_DIR/apps/installer"

echo "Installing installer dependencies..."
npm --prefix "$APP_DIR" ci

echo "Linting installer web app..."
npm --prefix "$APP_DIR" run lint

echo "Building installer web app..."
npm --prefix "$APP_DIR" run build

echo "Installer build checks complete."
