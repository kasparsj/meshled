# Lightgraph

Lightgraph is a networked light system with three runnable surfaces: a shared C++ core model/engine, ESP32 firmware that exposes OSC + HTTP control endpoints, and a React control panel for managing devices on a local network.

## Quickstart

### 1) Bootstrap dependencies

```bash
git submodule update --init --recursive
```

### 2) Control panel (`apps/control-panel`)

```bash
cd apps/control-panel
npm ci --legacy-peer-deps
npm run dev
```

### 3) Firmware (`firmware/esp`)

```bash
cd firmware/esp
pio run -e esp32dev
```

Alternative board:

```bash
pio run -e esp32-s3-devkitc-1
```

### 4) Simulator / core smoke (`packages/simulator`)

```bash
cd packages/simulator
make -n
```

If openFrameworks is not in the default location:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

### 5) Optional root helper scripts

```bash
./scripts/build-control-panel.sh
./scripts/build-firmware.sh esp32dev compiledb
```

## Repository Structure

```text
.
├── apps/
│   └── control-panel/     # React + Vite control UI
├── firmware/
│   └── esp/               # ESP32/ESP32-S3 firmware (PlatformIO + Arduino)
├── packages/
│   ├── core/
│   │   └── src/           # shared C++ core model/engine
│   └── simulator/         # openFrameworks desktop simulator
├── vendor/
│   └── ofxColorTheory/    # color-theory dependency (git submodule)
├── docs/                  # build/API/contract docs
└── .github/workflows/ci.yml
```

## Key Docs

- Build/setup: `docs/build.md`
- Firmware HTTP API: `docs/firmware-api.md`
- OSC contract: `docs/osc-contract.md`
- UI/firmware compatibility: `docs/ui-firmware-compat.md`
- Reorg plan and phase tracking: `docs/CODEX_REORG_REPORT.md`
