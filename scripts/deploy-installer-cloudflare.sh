#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="$ROOT_DIR/apps/installer"
DIST_DIR="$APP_DIR/dist"

if [[ -f "$ROOT_DIR/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$ROOT_DIR/.env"
  set +a
fi

PROJECT_NAME="${CF_PAGES_PROJECT_NAME:-meshled-installer}"
BRANCH="${CF_PAGES_BRANCH:-production}"
INSTALL_DEPS=1
SKIP_BUILD=0

usage() {
  cat <<'EOF'
Deploy the MeshLED installer to Cloudflare Pages.

Usage:
  ./scripts/deploy-installer-cloudflare.sh [options]

Options:
  --project-name <name>  Cloudflare Pages project name (default: meshled-installer)
  --branch <name>        Pages deployment branch (default: production)
  --skip-build           Skip npm install/build and deploy existing dist/
  --no-install           Skip npm ci before build
  -h, --help             Show this help

Environment:
  .env (optional)        Loaded automatically from repository root
  CLOUDFLARE_API_TOKEN   API token for non-interactive auth (recommended in CI)
  CLOUDFLARE_ACCOUNT_ID  Account ID (recommended in CI)
  CF_API_TOKEN           Alias for CLOUDFLARE_API_TOKEN
  CF_ACCOUNT_ID          Alias for CLOUDFLARE_ACCOUNT_ID
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --project-name)
      [[ $# -ge 2 ]] || {
        echo "Missing value for --project-name" >&2
        exit 1
      }
      PROJECT_NAME="$2"
      shift 2
      ;;
    --branch)
      [[ $# -ge 2 ]] || {
        echo "Missing value for --branch" >&2
        exit 1
      }
      BRANCH="$2"
      shift 2
      ;;
    --skip-build)
      SKIP_BUILD=1
      shift
      ;;
    --no-install)
      INSTALL_DEPS=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if ! command -v npm >/dev/null 2>&1; then
  echo "npm is required but not found in PATH." >&2
  exit 1
fi

if ! command -v npx >/dev/null 2>&1; then
  echo "npx is required but not found in PATH." >&2
  exit 1
fi

if [[ -n "${CF_API_TOKEN:-}" && -z "${CLOUDFLARE_API_TOKEN:-}" ]]; then
  export CLOUDFLARE_API_TOKEN="$CF_API_TOKEN"
fi

if [[ -n "${CF_ACCOUNT_ID:-}" && -z "${CLOUDFLARE_ACCOUNT_ID:-}" ]]; then
  export CLOUDFLARE_ACCOUNT_ID="$CF_ACCOUNT_ID"
fi

if [[ "$SKIP_BUILD" -eq 0 ]]; then
  if [[ "$INSTALL_DEPS" -eq 1 ]]; then
    echo "Installing installer dependencies..."
    npm --prefix "$APP_DIR" ci
  fi

  echo "Building installer..."
  npm --prefix "$APP_DIR" run build
fi

if [[ ! -d "$DIST_DIR" ]]; then
  echo "Build output not found at $DIST_DIR. Run without --skip-build first." >&2
  exit 1
fi

if [[ -z "${CLOUDFLARE_API_TOKEN:-}" ]]; then
  echo "CLOUDFLARE_API_TOKEN is not set; Wrangler may ask for interactive login."
fi

echo "Deploying $DIST_DIR to Cloudflare Pages project '$PROJECT_NAME' on branch '$BRANCH'..."
npx --yes wrangler pages deploy "$DIST_DIR" --project-name "$PROJECT_NAME" --branch "$BRANCH"

echo "Cloudflare Pages deploy complete."
