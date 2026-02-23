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

## How it works (internal overview)

### High-level architecture (text diagram)

```text
Inputs
  -> Stable API caller (lightpath::Engine)
  -> Simulator OSC (/emit, /note_on, /command, ...)
  -> Firmware OSC + HTTP + WLED API (+ React control panel over HTTP)

Ingress adapters
  -> EmitParams / EmitCommand + runtime mutations (layer visibility, palette, flags, offsets)

Core engine (packages/lightpath/src)
  -> TopologyObject graph (Intersection <-> Connection via Port)
  -> Model/Weight rules choose where lights go at intersections
  -> State owns active LightLists (layers)
  -> RuntimeLight/Light advance every frame
  -> Pixel accumulators + blend modes produce final RGB per logical pixel

Outputs
  -> Firmware LED drivers (NeoPixelBus/FastLED) from State::getPixel(...)
  -> Simulator drawing from State::getPixel(...)
  -> HTTP JSON snapshots (/get_colors, /get_model) for UI and tooling
```

Primary sources:
- `packages/lightpath/src/runtime/State.cpp`
- `packages/lightpath/src/topology/Intersection.cpp`
- `firmware/esp/LEDLib.h`
- `apps/simulator/src/ofApp.cpp`

### Key concepts glossary

| Concept | What it means here | Defined in |
| --- | --- | --- |
| `TopologyObject` | The full graph for one fixture layout. Holds intersections, connections, models, gaps, pixel counts. | `packages/lightpath/src/topology/TopologyObject.h` |
| `Intersection` | Graph node where routing decisions happen (weighted/random port choice). | `packages/lightpath/src/topology/Intersection.h` |
| `Connection` | Directed segment between intersections; maps motion position to LED indices. | `packages/lightpath/src/topology/Connection.h` |
| `Port` | Endpoint metadata linking intersections and connections; can be internal or external. | `packages/lightpath/src/topology/Port.h` |
| `Model` / `Weight` | Routing weight table used when choosing outgoing ports at intersections. | `packages/lightpath/src/topology/Model.h`, `packages/lightpath/src/topology/Weight.h` |
| `EmitParams` | Legacy integration command schema (speed, length, palette, flags, groups, source). | `packages/lightpath/src/runtime/EmitParams.h` |
| `State` | Frame orchestrator; owns active lists and pixel accumulators. | `packages/lightpath/src/runtime/State.h` |
| `LightList` | One animation layer/list (speed/ease/fade/palette/blend/behavior + light instances). | `packages/lightpath/src/runtime/LightList.h` |
| `RuntimeLight` / `Light` | Per-light runtime state advanced each frame. | `packages/lightpath/src/runtime/RuntimeLight.h`, `packages/lightpath/src/runtime/Light.h` |
| `Palette` | Color list + positions + interpolation/wrap/rule metadata. | `packages/lightpath/src/rendering/Palette.h` |
| `Groups` | Bitmask buckets (`GROUP1..GROUP8`) used for emit scopes and routing domains. | `packages/lightpath/src/core/Types.h` |
| `Gaps` | Logical pixel ranges excluded from real output mapping (e.g. non-LED spans). | `packages/lightpath/src/topology/TopologyObject.cpp` |

### Data model: how objects connect

1. A concrete object (`Line`, `Cross`, `Triangle`, `Heptagon*`) builds a `TopologyObject` graph by creating intersections, connections, and models with weights.
2. `State` owns up to `MAX_LIGHT_LISTS` active `LightList*` (slot `0` reserved for background).
3. Each `LightList` owns a dynamic array of `RuntimeLight*`.
4. Each light has a current `Owner` (`Intersection` or `Connection`) and optional in/out ports.
5. On each update, owner logic advances position/routing; state blends resulting pixel colors.

Primary sources:
- `packages/lightpath/src/objects/Line.cpp`
- `packages/lightpath/src/runtime/State.cpp`
- `packages/lightpath/src/runtime/LightList.cpp`
- `packages/lightpath/src/topology/Owner.cpp`

### Pipeline: input to output

There are three practical pipelines in this repo.

1. Stable host API pipeline (`lightpath::Engine`)
- Caller constructs `EngineConfig` + `EmitCommand`.
- `Engine::emit` validates command, maps into `EmitParams`, then calls `State::emit`.
- `Engine::tick` / `Engine::update` sets `gMillis`, calls `State::autoEmit`, `State::update`.
- Caller reads pixels with `Engine::pixel`.
- Source: `packages/lightpath/src/api/Engine.cpp`.

2. Simulator pipeline (desktop)
- `apps/simulator/src/main.cpp` starts `ofApp`.
- `ofApp::updateOsc` maps OSC messages to `EmitParams`.
- `state->emit(...)`, `state->autoEmit(...)`, `state->update()`.
- `ofApp::draw` renders intersections/connections using `state->getPixel(...)`.
- Source: `apps/simulator/src/ofApp.cpp`.

3. Firmware + control-panel pipeline (embedded + web)
- `firmware/esp/homo_deus.ino` loop runs OSC/Web handlers, then `updateLEDs()` + `drawLEDs()`.
- OSC handlers (`OSCLib.h`) parse numeric param IDs into `EmitParams`.
- HTTP handlers (`WebServer*.h`) mutate layers/palettes/model and expose `/get_model`, `/get_colors`.
- LED backend translates logical pixels to strips (`translateToLogicalPixel`) and sends via NeoPixelBus/FastLED.
- React control panel calls those HTTP routes (`apps/control-panel/src/hooks/*.js`).
- Sources: `firmware/esp/OSCLib.h`, `firmware/esp/WebServerSetup.h`, `firmware/esp/NeoPixelBusLib.h`, `apps/control-panel/src/hooks/useLayers.js`.

### Where routing/pathfinding happens

There is no global shortest-path/TSP preprocessing in current core.

Routing is local and per-step:
1. At an intersection, `Intersection::choosePort(...)` evaluates model weights for candidate ports.
2. It supports two strategies: weighted-random (default) and deterministic highest-weight tie-broken by port id.
3. If total weight is zero, it falls back to constrained random choice (`randomPort`).
4. The chosen port sends the light into the next connection/intersection.

Source:
- `packages/lightpath/src/topology/Intersection.cpp`
- `packages/lightpath/src/topology/Model.h`
- `packages/lightpath/src/topology/Weight.cpp`

Complexity notes:
- Per intersection decision is proportional to port count (`O(numPorts)`), with small constant factors because ports are usually 2-4.
- Weight lookups use `std::unordered_map` in model/weight tables (average `O(1)` lookup).

### Performance notes and constraints

Hard limits and assumptions come from `core/Limits.h`:
- `MAX_GROUPS=5`
- `MAX_LIGHT_LISTS=20`
- `MAX_TOTAL_LIGHTS=1500`
- `CONNECTION_MAX_LEDS=48`
- `OUT_PORTS_MEMORY=3`

Runtime characteristics:
- Frame-driven engine; no internal fixed timestep integrator (`gMillis` is external).
- Duration-to-frame math assumes `EmitParams::DURATION_FPS = 62.5`.
- Main hotspot is `State::update` (full per-frame accumulator clear + per-light update + blend).
- Blend mode implementation is in `State::setPixel` and can dominate CPU with many active layers/lights.
- Core internals are single-threaded; only the high-level `Engine` facade adds a mutex for thread safety.

Source:
- `packages/lightpath/src/core/Limits.h`
- `packages/lightpath/src/runtime/EmitParams.cpp`
- `packages/lightpath/src/runtime/State.cpp`
- `packages/lightpath/src/api/Engine.cpp`

### Extending the system

1. Add a new geometry source/object:
- Create a new `TopologyObject` subclass in `packages/lightpath/src/objects/`.
- Implement `setup`, `getMirroredPixels`, `getModelParams`.
- Optionally expose through `lightpath::Engine` object enum and integration factory.
- Files to update: `packages/lightpath/src/objects/*`, `packages/lightpath/src/api/Engine.cpp`, `packages/lightpath/include/lightpath/types.hpp`, `packages/lightpath/include/lightpath/integration/factory.hpp`.

2. Add a new routing/path strategy:
- Extend `Model`/`Weight` semantics or introduce a strategy layer used by `Intersection::choosePort`.
- Keep default behavior unchanged for compatibility.
- Files to update: `packages/lightpath/src/topology/Intersection.cpp`, `packages/lightpath/src/topology/Model.h`, `packages/lightpath/src/topology/Weight.h`.

3. Add a new exporter/output adapter:
- Reuse `State::getPixel` in a new adapter loop (same pattern as simulator draw and firmware LED backends).
- Files to model after: `firmware/esp/NeoPixelBusLib.h`, `firmware/esp/FastLEDLib.h`, `apps/simulator/src/ofApp.cpp`.

4. Add a new control ingress:
- Parse into `EmitParams` and/or constrained layer updates.
- Files to model after: `firmware/esp/OSCLib.h`, `firmware/esp/WebServerSetup.h`.

## Repository tour

- `packages/lightpath/src/`: core engine internals (topology, runtime, rendering, objects).
- `packages/lightpath/include/lightpath/`: stable API + source-integration headers.
- `packages/lightpath/src/api/Engine.cpp`: stable `lightpath::Engine` facade implementation.
- `apps/simulator/src/main.cpp`: simulator process entry point.
- `apps/simulator/src/ofApp.cpp`: simulator runtime loop + OSC ingress + rendering.
- `firmware/esp/homo_deus.ino`: firmware process entry point (`setup`/`loop`).
- `firmware/esp/LEDLib.h`: object/state setup and LED update/draw loop.
- `firmware/esp/OSCLib.h`: OSC-to-EmitParams mapping.
- `firmware/esp/WebServerSetup.h`: HTTP API and model/layer mutation endpoints.
- `apps/control-panel/src/main.jsx`: React app entry point.
- `apps/control-panel/src/hooks/`: HTTP client hooks for firmware APIs (`/get_layers`, `/get_model`, `/get_colors`, etc.).
- `scripts/`: reproducible helper scripts for bootstrap/core/firmware/control-panel builds.
- `.github/workflows/ci.yml`: CI coverage for core, web, firmware smoke, simulator layout checks.

## Minimal working example

This is the smallest verified runnable path in this repository: build and run the host core example.

```bash
git submodule update --init --recursive
cmake -S packages/lightpath -B packages/lightpath/build-readme -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_BUILD_EXAMPLES=ON
cmake --build packages/lightpath/build-readme --target lightpath_core_minimal_example --parallel
./packages/lightpath/build-readme/lightpath_core_minimal_example
```

Expected output pattern (verified locally): `Pixel(0): <r>,<g>,<b>`

Optional verified follow-up:

```bash
cmake --build packages/lightpath/build-readme --parallel
ctest --test-dir packages/lightpath/build-readme --output-on-failure
```

What is not runnable as-is in a clean environment:
- `apps/simulator` requires a local openFrameworks checkout at `OF_ROOT` (default `../../../../openframeworks`).
- Without that checkout, `make -n` fails with missing `compile.project.mk`.
- Fix: provide a valid `OF_ROOT` or install openFrameworks at the expected location.

<details>
<summary>Investigation log (files inspected and conclusions)</summary>

- `README.md`, `docs/core-architecture.md`, `docs/core-build.md`, `docs/lightpath-api-inventory.md`: existing docs are good on build and broad architecture, but they do not provide one contributor-focused end-to-end internals walkthrough in root README.
- `packages/lightpath/src/runtime/State.cpp`, `LightList.cpp`, `RuntimeLight.cpp`, `Light.cpp`: verified update lifecycle, layer ownership, blend pipeline, and frame flow.
- `packages/lightpath/src/topology/Intersection.cpp`, `Connection.cpp`, `Model.h`, `Weight.cpp`: verified routing is local (weighted-random or deterministic), with no global shortest-path preprocessing.
- `packages/lightpath/src/objects/*.cpp`: verified geometry is code-defined through built-in object constructors and model weights.
- `packages/lightpath/src/api/Engine.cpp`, `include/lightpath/*`: verified stable API facade and source-integration split.
- `apps/simulator/src/ofApp.cpp`: verified simulator OSC ingress and draw loop are direct core adapters.
- `firmware/esp/homo_deus.ino`, `LEDLib.h`, `OSCLib.h`, `WebServerSetup.h`, `WebServerLayers.h`, `WLEDApiLib.h`, `FSLib.h`: verified firmware runtime loop, transport adapters, persistence formats, and model/palette/layer endpoints.
- `apps/control-panel/src/*`: verified UI reads `/get_layers`, `/get_model`, `/get_colors` and writes layer/palette/settings mutations to firmware HTTP endpoints.
- Verified commands in this environment:
  - Built and ran `lightpath_core_minimal_example`.
  - Built and ran `lightpath_core_integration_example`.
  - Full `ctest` pass on `packages/lightpath/build-readme` after full build.
  - `apps/simulator make -n` fails without openFrameworks checkout (`OF_ROOT` dependency).

</details>

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
