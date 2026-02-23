# Core Build and Reproducibility Guide

Date: 2026-02-23
Scope: shared engine in `packages/lightpath/src`

## What “core” means in this repo

The core is the shared C++ light engine used by:

- firmware (`firmware/esp`)
- simulator (`apps/simulator`)
- standalone host build (`packages/lightpath/CMakeLists.txt`)

In this monorepo, `packages/lightpath` is a git submodule sourced from `git@github.com:kasparsj/lightpath.git`.

The host build is intended for reproducible compile/test checks in CI without requiring openFrameworks or Arduino SDKs.

For architecture details (module responsibilities, update loop, ownership), see `docs/core-architecture.md`.

## Prerequisites

- Git submodules initialized (required for `packages/lightpath/vendor/ofxColorTheory`)
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

- `packages/lightpath/vendor/ofxColorTheory` exists and has content.

## 2) Build and test core on host (standalone)

```bash
cmake -S packages/lightpath -B packages/lightpath/build -DLIGHTPATH_CORE_BUILD_TESTS=ON
cmake --build packages/lightpath/build
ctest --test-dir packages/lightpath/build --output-on-failure
```

Expected:

- static library target builds
- smoke/regression tests pass

Optional ASan run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-asan -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_ASAN=ON
cmake --build packages/lightpath/build-asan
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir packages/lightpath/build-asan --output-on-failure
```

Optional UBSan run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-ubsan -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_UBSAN=ON
cmake --build packages/lightpath/build-ubsan
ctest --test-dir packages/lightpath/build-ubsan --output-on-failure
```

Optional strict warnings run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-warnings -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_STRICT_WARNINGS=ON
cmake --build packages/lightpath/build-warnings
ctest --test-dir packages/lightpath/build-warnings --output-on-failure
```

Optional script helper (runs one profile or all):

```bash
./scripts/build-core.sh default
./scripts/build-core.sh asan
./scripts/build-core.sh ubsan
./scripts/build-core.sh warnings
./scripts/build-core.sh all
```

## 3) Verify firmware wiring to shared core

The firmware consumes core sources through a symlink:

```bash
ls -l firmware/esp/src
```

Expected:

- `firmware/esp/src -> ../../packages/lightpath/src`

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
cd apps/simulator
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

- Symptom: include errors under `packages/lightpath/vendor/ofxColorTheory`
- Fix: run submodule init/update command

Broken symlink on clone:

- Symptom: firmware cannot include `src/...` shared headers
- Fix: recreate `firmware/esp/src` symlink to `../../packages/lightpath/src`

## Repro checklist for contributors

- submodules initialized
- host build + tests pass
- firmware symlink present
- `pio run -e esp32dev -t compiledb` passes
- simulator dry-run works when `OF_ROOT` is valid
