---
title: "Core Build and Reproducibility Guide"
---

Date: 2026-02-23
Scope: shared engine in `packages/lightgraph/src`

## What “core” means in this repo

The core is the shared C++ light engine used by:

- firmware (`firmware/esp`)
- simulator (`apps/simulator`)
- standalone host build (`packages/lightgraph/CMakeLists.txt`)

In this monorepo, `packages/lightgraph` is a git submodule sourced from `git@github.com:kasparsj/lightgraph.git`.

The host build is intended for reproducible compile/test checks in CI without requiring openFrameworks or Arduino SDKs.

For architecture details (module responsibilities, update loop, ownership), see `/core-architecture/`.

## Prerequisites

- Git submodules initialized (required for `packages/lightgraph/vendor/ofxColorTheory`)
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

- `packages/lightgraph/vendor/ofxColorTheory` exists and has content.

## 2) Build and test core on host (standalone)

```bash
cmake -S packages/lightgraph -B packages/lightgraph/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/lightgraph/build
ctest --test-dir packages/lightgraph/build --output-on-failure
```

Expected:

- static library target builds
- smoke/regression tests pass

Optional ASan run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-asan -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_ASAN=ON
cmake --build packages/lightgraph/build-asan
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir packages/lightgraph/build-asan --output-on-failure
```

Optional UBSan run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-ubsan -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_UBSAN=ON
cmake --build packages/lightgraph/build-ubsan
ctest --test-dir packages/lightgraph/build-ubsan --output-on-failure
```

Optional strict warnings run:

```bash
CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-warnings -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_STRICT_WARNINGS=ON
cmake --build packages/lightgraph/build-warnings
ctest --test-dir packages/lightgraph/build-warnings --output-on-failure
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

The firmware consumes core sources and vendored dependencies through symlinks:

```bash
ls -l firmware/esp/src
ls -l firmware/esp/vendor
ls -l firmware/esp/lightgraph
```

Expected:

- `firmware/esp/src -> ../../packages/lightgraph/src`
- `firmware/esp/vendor -> ../../packages/lightgraph/vendor`
- `firmware/esp/lightgraph -> ../../packages/lightgraph/include/lightgraph`

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

- Symptom: include errors under `packages/lightgraph/vendor/ofxColorTheory`
- Fix: run submodule init/update command

Broken symlink on clone:

- Symptom: firmware cannot include `src/...` shared headers
- Fix: recreate `firmware/esp/src` symlink to `../../packages/lightgraph/src`

Missing vendor symlink on clone:

- Symptom: Arduino IDE/CLI include failures for `../../vendor/ofxColorTheory/...`
- Fix: recreate `firmware/esp/vendor` symlink to `../../packages/lightgraph/vendor`

Missing lightgraph include symlink on clone:

- Symptom: Arduino IDE/CLI include failures for `<lightgraph/...>`
- Fix: recreate `firmware/esp/lightgraph` symlink to `../../packages/lightgraph/include/lightgraph`

Sketch too large for ESP32-C3 default partition:

- Symptom: `Sketch too big` / `text section exceeds available space in board`
- Fix: select a larger partition scheme (for example `PartitionScheme=min_spiffs` or `PartitionScheme=huge_app`)

## Repro checklist for contributors

- submodules initialized
- host build + tests pass
- firmware symlinks present (`src`, `vendor`, and `lightgraph`)
- `pio run -e esp32dev -t compiledb` passes
- simulator dry-run works when `OF_ROOT` is valid
