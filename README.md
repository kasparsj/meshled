# meshled

## What It Is

meshled is a complete programmable lighting system for LED installations.

It is designed for non-linear LED geometries, where light moves through branches, intersections, loops, and custom shapes rather than simple strip layouts. It is suitable for light sculpture, architectural lighting, and generative light work.

meshled combines hardware, software, and simulation into one workflow so you can design, test, and run the same lighting logic from development to live installation.

## The Three Core Parts

### 1. Firmware

The firmware runs on ESP32-based hardware nodes.

It executes the LightGraph engine in real time, drives LED output, and receives configuration/control data over network interfaces.

### 2. Control Panel

The control panel is a web-based interface.

It is used to configure geometry, effects, and routing behavior, connect to hardware nodes on the network, and orchestrate the system from one place.

### 3. Simulator

The simulator runs the same LightGraph engine used by firmware.

It lets you test behavior without hardware, mirror firmware behavior during development, and plan/debug installations before deployment.

Firmware and simulator both run on top of LightGraph.

## System Architecture

```text
User -> Control Panel -> LightGraph Engine -> Firmware Nodes -> LEDs
                           \-> Simulator (same engine)
```

## Why This Matters

Most LED tooling assumes linear strips and channel-based control.

meshled, powered by LightGraph, supports arbitrary graph/mesh geometries where routing and movement are part of the composition itself. The same core logic runs in simulation and on real hardware, so installation-scale systems can be developed as programmable infrastructure instead of one-off wiring logic.

## How It Relates to LightGraph

- LightGraph is the core engine.
- meshled is the full-stack system built around that engine.
- Firmware executes LightGraph for live hardware output.
- Simulator executes the same LightGraph engine for development/testing.
- Control panel configures and orchestrates the engine across devices.

LightGraph repository: [kasparsj/lightgraph](https://github.com/kasparsj/lightgraph)

## Future Vision

meshled is intended to evolve as:

- programmable lighting infrastructure for real environments,
- a platform for digital light art,
- a hardware + software stack for production installations,
- and an open ecosystem for reusable lighting components, tools, and workflows.

---

## Getting Started (Technical)

### 1) Bootstrap dependencies

```bash
git submodule update --init --recursive
```

This initializes [packages/lightgraph](packages/lightgraph) and its nested `ofxColorTheory` submodule.

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

### 4) LightGraph core smoke (`packages/lightgraph`)

```bash
cmake -S packages/lightgraph -B packages/lightgraph/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/lightgraph/build
ctest --test-dir packages/lightgraph/build --output-on-failure
```

### 5) Simulator smoke (`apps/simulator`)

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
│   ├── control-panel/     # Web UI
│   └── simulator/         # openFrameworks simulator (runs LightGraph)
├── firmware/
│   └── esp/               # ESP32 firmware (runs LightGraph)
├── packages/
│   └── lightgraph/        # Core engine submodule
├── tools/
│   └── esp-stacktrace-decoder/
├── docs/                  # Build/API/contract documentation
└── .github/workflows/
```

## Key Docs

- Build/setup: [docs/build.md](docs/build.md)
- Core build/repro guide: [docs/core-build.md](docs/core-build.md)
- Core architecture primer: [docs/core-architecture.md](docs/core-architecture.md)
- Third-party licenses/provenance: [docs/third-party-licenses.md](docs/third-party-licenses.md)
- Firmware HTTP API: [docs/firmware-api.md](docs/firmware-api.md)
- OSC contract: [docs/osc-contract.md](docs/osc-contract.md)
- UI/firmware compatibility: [docs/ui-firmware-compat.md](docs/ui-firmware-compat.md)
- LightGraph API inventory snapshot: [docs/lightgraph-api-inventory.md](docs/lightgraph-api-inventory.md)

## Documentation Site (GitHub Pages)

This repository can publish docs with MkDocs + Material:

```bash
python3 -m venv .venv-docs
source .venv-docs/bin/activate
pip install mkdocs-material
mkdocs serve
```

- Config: [mkdocs.yml](mkdocs.yml)
- Pages workflow: [.github/workflows/docs-pages.yml](.github/workflows/docs-pages.yml)

## Project Policies

- License: [LICENSE](LICENSE)
- Contribution guide: [CONTRIBUTING.md](CONTRIBUTING.md)
- Code of conduct: [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)
- Security policy: [SECURITY.md](SECURITY.md)
- Changelog: [CHANGELOG.md](CHANGELOG.md)
