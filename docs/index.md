# meshled

## What It Is

meshled is a complete programmable lighting system for LED installations.

It is designed for non-linear LED geometries, where light moves through branches, intersections, loops, and custom shapes rather than simple strip layouts. It is suitable for light sculpture, architectural lighting, and generative light work.

meshled combines hardware, software, and simulation into one workflow so you can design, test, and run the same lighting logic from development to live installation.

## The Three Core Parts

### 1. Firmware

The firmware runs on ESP32-based hardware nodes.

It executes the [LightGraph](https://github.com/kasparsj/lightgraph) engine in real time, drives LED output, and receives configuration/control data over network interfaces.

### 2. Control Panel

The control panel is a web-based interface.

It is used to configure geometry, effects, and routing behavior, connect to hardware nodes on the network, and orchestrate the system from one place.

### 3. Simulator

The simulator runs the same [LightGraph](https://github.com/kasparsj/lightgraph) engine used by firmware.

It lets you test behavior without hardware, mirror firmware behavior during development, and plan/debug installations before deployment.

Firmware and simulator both run on top of [LightGraph](https://github.com/kasparsj/lightgraph).

## System Architecture

Core engine in this flow is [LightGraph](https://github.com/kasparsj/lightgraph).

```text
User -> Control Panel -> Core Engine -> Firmware Nodes -> LEDs
                           \-> Simulator (same engine)
```

## Why This Matters

Most LED tooling assumes linear strips and channel-based control.

meshled, powered by [LightGraph](https://github.com/kasparsj/lightgraph), supports arbitrary graph/mesh geometries where routing and movement are part of the composition itself. The same core logic runs in simulation and on real hardware, so installation-scale systems can be developed as programmable infrastructure instead of one-off wiring logic.

## How It Relates to [LightGraph](https://github.com/kasparsj/lightgraph)

- [LightGraph](https://github.com/kasparsj/lightgraph) is the core engine.
- meshled is the full-stack system built around that engine.
- Firmware executes [LightGraph](https://github.com/kasparsj/lightgraph) for live hardware output.
- Simulator executes the same [LightGraph](https://github.com/kasparsj/lightgraph) engine for development/testing.
- Control panel configures and orchestrates the engine across devices.

LightGraph repository: [kasparsj/lightgraph](https://github.com/kasparsj/lightgraph)

## Future Vision

meshled is intended to evolve as:

- programmable lighting infrastructure for real environments,
- a platform for digital light art,
- a hardware + software stack for production installations,
- and an open ecosystem for reusable lighting components, tools, and workflows.

---

## Quick Start (Recommended)

If you want the fastest verification path, run the shared core smoke first:

```bash
git submodule update --init --recursive
cmake -S packages/lightgraph -B packages/lightgraph/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/lightgraph/build --parallel
ctest --test-dir packages/lightgraph/build --output-on-failure
```

Expected result: `ctest` completes without failures.

## Choose Your Workflow

- Full repository build and verification (firmware + control panel + simulator + core): [build.md](build.md)
- Core engine reproducibility and sanitizer profiles: [core-build.md](core-build.md)
- Core architecture deep dive: [core-architecture.md](core-architecture.md)
- Firmware HTTP API contract: [firmware-api.md](firmware-api.md)
- OSC contract: [osc-contract.md](osc-contract.md)
- UI/firmware compatibility notes: [ui-firmware-compat.md](ui-firmware-compat.md)
- Third-party licenses and provenance: [third-party-licenses.md](third-party-licenses.md)
- [LightGraph](https://github.com/kasparsj/lightgraph) API inventory snapshot in this repo: [lightgraph-api-inventory.md](lightgraph-api-inventory.md)

## Repository Surfaces

- Firmware: [firmware/esp](https://github.com/kasparsj/meshled/tree/main/firmware/esp)
- Control panel: [apps/control-panel](https://github.com/kasparsj/meshled/tree/main/apps/control-panel)
- Simulator: [apps/simulator](https://github.com/kasparsj/meshled/tree/main/apps/simulator)
- Core engine submodule: [packages/lightgraph](https://github.com/kasparsj/meshled/tree/main/packages/lightgraph) ([LightGraph repo](https://github.com/kasparsj/lightgraph))

## Documentation Site (GitHub Pages)

- Docs index: [index.md](index.md)
- Config: [mkdocs.yml](https://github.com/kasparsj/meshled/blob/main/mkdocs.yml)
- Pages workflow: [.github/workflows/docs-pages.yml](https://github.com/kasparsj/meshled/blob/main/.github/workflows/docs-pages.yml)

Local docs preview:

```bash
python3 -m venv .venv-docs
source .venv-docs/bin/activate
pip install mkdocs-material
mkdocs serve
```

## Project Policies

- License: [LICENSE](https://github.com/kasparsj/meshled/blob/main/LICENSE)
- Contribution guide: [CONTRIBUTING.md](https://github.com/kasparsj/meshled/blob/main/CONTRIBUTING.md)
- Code of conduct: [CODE_OF_CONDUCT.md](https://github.com/kasparsj/meshled/blob/main/CODE_OF_CONDUCT.md)
- Security policy: [SECURITY.md](https://github.com/kasparsj/meshled/blob/main/SECURITY.md)
- Changelog: [CHANGELOG.md](https://github.com/kasparsj/meshled/blob/main/CHANGELOG.md)
