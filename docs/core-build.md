# Core Build and Reproducibility Guide

Date: 2026-02-23
Scope: shared engine in `packages/core/src`

## What “core” means in this repo

The core is the shared C++ light engine used by:

- firmware (`firmware/esp`)
- simulator (`packages/simulator`)
- standalone host build (`packages/core/CMakeLists.txt`)

The host build is intended for reproducible compile/test checks in CI without requiring openFrameworks or Arduino SDKs.

## Prerequisites

- Git submodules initialized (required for `vendor/ofxColorTheory`)
- For firmware-path verification:
- Python 3
- PlatformIO
- For host verification:
- CMake 3.20+
- C++17 compiler
- For simulator-path verification:
- openFrameworks checkout

## 1) Bootstrap submodules

```bash
git submodule update --init --recursive
```

Expected:

- `vendor/ofxColorTheory` exists and has content.

## 2) Build and test core on host (standalone)

```bash
cmake -S packages/core -B packages/core/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/core/build
ctest --test-dir packages/core/build --output-on-failure
```

Expected:

- static library (`lightgraph_core`) builds
- smoke/regression tests pass (`lightgraph_core_smoke`, `lightgraph_core_regression`)

Optional ASan run:

```bash
CC=clang CXX=clang++ cmake -S packages/core -B packages/core/build-asan -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_ASAN=ON
cmake --build packages/core/build-asan
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir packages/core/build-asan --output-on-failure
```

Optional UBSan run:

```bash
CC=clang CXX=clang++ cmake -S packages/core -B packages/core/build-ubsan -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_UBSAN=ON
cmake --build packages/core/build-ubsan
ctest --test-dir packages/core/build-ubsan --output-on-failure
```

## 3) Verify firmware wiring to shared core

The firmware consumes core sources through a symlink:

```bash
ls -l firmware/esp/src
```

Expected:

- `firmware/esp/src -> ../../packages/core/src`

## 4) Core compile smoke through firmware toolchain

```bash
cd firmware/esp
pio run -e esp32dev -t compiledb
```

Expected:

- command succeeds
- `firmware/esp/compile_commands.json` is generated

## 5) Simulator-path verification (openFrameworks required)

```bash
cd packages/simulator
OF_ROOT=/path/to/openframeworks make -n
```

Notes:

- default `OF_ROOT` expected by repo: `../../../../openframeworks`
- `make -n` only verifies project wiring; use `make` for full build

## Common failures

Missing OF checkout:

- Symptom: `compile.project.mk: No such file or directory`
- Fix: set `OF_ROOT` to a valid openFrameworks root

Missing submodule:

- Symptom: include errors under `vendor/ofxColorTheory`
- Fix: run submodule init/update command

Broken symlink on clone:

- Symptom: firmware cannot include `src/...` shared headers
- Fix: recreate `firmware/esp/src` symlink to `../../packages/core/src`

## Repro checklist for contributors

- submodules initialized
- host build + tests pass
- firmware symlink present
- `pio run -e esp32dev -t compiledb` passes
- simulator dry-run works when `OF_ROOT` is valid
