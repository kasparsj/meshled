# MeshLED

MeshLED is a networked light system with three runnable surfaces: a shared C++ core model/engine, ESP32 firmware that exposes OSC + HTTP control endpoints, and a React control panel for managing devices on a local network.

## Quickstart

### 1) Bootstrap dependencies

```bash
git submodule update --init --recursive
```

This initializes `packages/lightpath` from [lightpath](https://github.com/kasparsj/lightpath) and its nested `ofxColorTheory` submodule.

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

### 4) Core host smoke (`packages/lightpath`)

```bash
cmake -S packages/lightpath -B packages/lightpath/build -DLIGHTPATH_CORE_BUILD_TESTS=ON
cmake --build packages/lightpath/build
ctest --test-dir packages/lightpath/build --output-on-failure
```

### 5) Simulator / core smoke (`apps/simulator`)

```bash
cd apps/simulator
make -n
```

If openFrameworks is not in the default location:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

### 6) Optional root helper scripts

```bash
./scripts/build-core.sh all
./scripts/build-control-panel.sh
./scripts/build-firmware.sh esp32dev compiledb
```

## Repository Structure

```text
.
├── apps/
│   ├── control-panel/     # React + Vite control UI
│   └── simulator/         # openFrameworks desktop simulator
├── firmware/
│   └── esp/               # ESP32/ESP32-S3 firmware (PlatformIO + Arduino)
├── packages/
│   └── lightpath/         # git submodule: github.com/kasparsj/lightpath
│       ├── src/           # shared C++ core model/engine
│       └── vendor/
│           └── ofxColorTheory/  # nested submodule used by lightpath
├── tools/
│   └── esp-stacktrace-decoder/
├── docs/                  # build/API/contract docs
└── .github/workflows/ci.yml
```

## Key Docs

- Build/setup: `docs/build.md`
- Core build/repro guide: `docs/core-build.md`
- Core architecture primer: `docs/core-architecture.md`
- Third-party licenses/provenance: `docs/third-party-licenses.md`
- Firmware HTTP API: `docs/firmware-api.md`
- OSC contract: `docs/osc-contract.md`
- UI/firmware compatibility: `docs/ui-firmware-compat.md`

## Documentation Site (GitHub Pages)

This repository can publish docs with MkDocs + Material:

```bash
python3 -m venv .venv-docs
source .venv-docs/bin/activate
pip install mkdocs-material
mkdocs serve
```

Configuration: `mkdocs.yml`  
Pages workflow: `.github/workflows/docs-pages.yml`

## Project Policies

- License: `LICENSE`
- Contribution guide: `CONTRIBUTING.md`
- Code of conduct: `CODE_OF_CONDUCT.md`
- Security policy: `SECURITY.md`
- Changelog: `CHANGELOG.md`
