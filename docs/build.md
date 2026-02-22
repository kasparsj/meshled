# Lightgraph Build and Verification

Date: 2026-02-22

## Repository bootstrap

If this repo is used as a submodule, ensure nested submodules are initialized:

```bash
git submodule update --init --recursive
```

`ofxColorTheory` is required at `./vendor/ofxColorTheory`.

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

## CI coverage

GitHub Actions workflow: `.github/workflows/ci.yml`

Jobs:

1. `Web (React)`: `npm ci --legacy-peer-deps`, lint, build.
2. `Firmware (PlatformIO smoke)`: generates `compile_commands.json` for `esp32dev` (`pio run -e esp32dev -t compiledb`) to validate dependency resolution and toolchain setup.
3. `Simulator (Scoped smoke)`: project/config integrity checks, plus optional `make -n` when `OF_ROOT` is provided in CI environment.

Note:

- Simulator CI is intentionally scoped because runner images do not include openFrameworks by default.

## Root helper scripts

```bash
./scripts/build-control-panel.sh
./scripts/build-firmware.sh esp32dev compiledb
```
