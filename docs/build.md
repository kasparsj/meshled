# Lightgraph Build and Verification

Date: 2026-02-22

## Repository bootstrap

If this repo is used as a submodule, ensure nested submodules are initialized:

```bash
git submodule update --init --recursive
```

`ofxColorTheory` is required at `./vendor/ofxColorTheory`.

Core-specific reproducibility notes live in `docs/core-build.md`.
Core architecture notes live in `docs/core-architecture.md`.

Optional helper:

```bash
./scripts/bootstrap.sh
```

Optional crash-decoder helper:

```bash
./tools/esp-stacktrace-decoder/esp32decoder.sh <addr2line> <firmware.elf> <dump.txt>
```

## Firmware (`firmware/esp`)

Prerequisites:

- Python 3
- PlatformIO (`pip install platformio`)

Build:

```bash
cd firmware/esp
pio run -e esp32dev
```

Alternative target:

```bash
pio run -e esp32-s3-devkitc-1
```

## React control app (`apps/control-panel`)

Prerequisites:

- Node.js 20+ (18+ works, CI uses 20)

Install and verify:

```bash
cd apps/control-panel
npm ci --legacy-peer-deps
npm run lint
npm run build
```

Local dev server:

```bash
npm run dev
```

## Simulator (`packages/simulator`)

Prerequisites:

- openFrameworks checkout

Default expectation from simulator directory:

- `OF_ROOT=../../../../openframeworks`

Dry-run make:

```bash
cd packages/simulator
make -n
```

If openFrameworks is elsewhere:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

## Core host build (`packages/core`)

Prerequisites:

- CMake 3.20+
- C++17 compiler

Build and test:

```bash
cmake -S packages/core -B packages/core/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/core/build
ctest --test-dir packages/core/build --output-on-failure
```

## CI coverage

GitHub Actions workflow: `.github/workflows/ci.yml`

Jobs:

1. `Core (host build + tests)`: CMake configure/build + `ctest` for the shared native engine.
2. `Core (ASan)`: host build/test with AddressSanitizer enabled.
3. `Core (UBSan)`: host build/test with UndefinedBehaviorSanitizer enabled.
4. `Core (Warnings)`: host build/test with strict warnings enabled (`LIGHTGRAPH_CORE_ENABLE_STRICT_WARNINGS=ON`) and warnings treated as errors.
5. `Web (React)`: `npm ci --legacy-peer-deps`, lint, build.
6. `Firmware (PlatformIO smoke)`: generates `compile_commands.json` for `esp32dev` (`pio run -e esp32dev -t compiledb`) to validate dependency resolution and toolchain setup.
7. `Simulator (Scoped smoke)`: project/config integrity checks, plus optional `make -n` when `OF_ROOT` is provided in CI environment.

Note:

- Simulator CI is intentionally scoped because runner images do not include openFrameworks by default.

## Root helper scripts

```bash
./scripts/build-control-panel.sh
./scripts/build-firmware.sh esp32dev compiledb
```
