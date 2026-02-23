# Core Build and Reproducibility Guide

Date: 2026-02-23
Scope: shared engine in `packages/core/src`

## What “core” means in this repo

The core is the shared C++ light engine used by:

- firmware (`firmware/esp`)
- simulator (`packages/simulator`)

There is currently no standalone build target under `packages/core`; verification happens through those adapters.

## Prerequisites

- Git submodules initialized (required for `vendor/ofxColorTheory`)
- For firmware-path verification:
- Python 3
- PlatformIO
- For simulator-path verification:
- openFrameworks checkout

## 1) Bootstrap submodules

```bash
git submodule update --init --recursive
```

Expected:

- `vendor/ofxColorTheory` exists and has content.

## 2) Verify firmware wiring to shared core

The firmware consumes core sources through a symlink:

```bash
ls -l firmware/esp/src
```

Expected:

- `firmware/esp/src -> ../../packages/core/src`

## 3) Core compile smoke through firmware toolchain

```bash
cd firmware/esp
pio run -e esp32dev -t compiledb
```

Expected:

- command succeeds
- `firmware/esp/compile_commands.json` is generated

## 4) Simulator-path verification (openFrameworks required)

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
- firmware symlink present
- `pio run -e esp32dev -t compiledb` passes
- simulator dry-run works when `OF_ROOT` is valid
