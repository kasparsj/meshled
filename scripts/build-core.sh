#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CORE_DIR="$ROOT_DIR/packages/core"
PROFILE="${1:-default}"

configure_compiler() {
  if command -v clang >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then
    export CC="${CC:-clang}"
    export CXX="${CXX:-clang++}"
  fi
}

run_profile() {
  local profile="$1"
  local build_dir="$CORE_DIR/build"
  local -a cmake_args=(-DLIGHTPATH_CORE_BUILD_TESTS=ON)

  case "$profile" in
    default)
      build_dir="$CORE_DIR/build"
      ;;
    asan)
      configure_compiler
      build_dir="$CORE_DIR/build-asan"
      cmake_args+=(-DLIGHTPATH_CORE_ENABLE_ASAN=ON)
      export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
      ;;
    ubsan)
      configure_compiler
      build_dir="$CORE_DIR/build-ubsan"
      cmake_args+=(-DLIGHTPATH_CORE_ENABLE_UBSAN=ON)
      ;;
    warnings)
      configure_compiler
      build_dir="$CORE_DIR/build-warnings"
      cmake_args+=(-DLIGHTPATH_CORE_ENABLE_STRICT_WARNINGS=ON)
      ;;
    *)
      echo "Unknown profile '$profile'. Expected: default|asan|ubsan|warnings|all" >&2
      exit 1
      ;;
  esac

  echo "==> Configuring core profile '$profile' in '$build_dir'"
  cmake -S "$CORE_DIR" -B "$build_dir" "${cmake_args[@]}"
  echo "==> Building core profile '$profile'"
  cmake --build "$build_dir"
  echo "==> Testing core profile '$profile'"
  ctest --test-dir "$build_dir" --output-on-failure
}

if [[ "$PROFILE" == "all" ]]; then
  for profile in default asan ubsan warnings; do
    run_profile "$profile"
  done
else
  run_profile "$PROFILE"
fi
