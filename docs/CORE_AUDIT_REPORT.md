# Lightgraph Core Audit Report

Date: 2026-02-23
Scope: `packages/core/src` and its direct runtime adapters (`packages/simulator`, firmware integration points only where needed to explain core behavior)

## 1) Executive Summary

### What the core appears to do

- Implements a graph-based light propagation engine over `Intersection` and `Connection` topology objects, with per-path weighted routing via `Model`/`Weight`.
- Maintains runtime animation layers (`LightList`) that emit and update light particles (`LPLight`/`Light`) every frame.
- Applies color/palette interpolation (including `ofxColorTheory` color rules) and per-layer blend modes into shared pixel accumulators in `State`.
- Supports behavior flags for movement/brightness/mirroring/segment rendering (`BehaviourFlags` in `Config.h`) that alter propagation and rendering semantics.
- Is reused in two surfaces: desktop simulator (openFrameworks + OSC input) and ESP firmware (Arduino + OSC/HTTP control), with firmware compiling core through a symlink (`firmware/esp/src -> ../../packages/core/src`).

### Readiness score (0-10)

| Area | Score | Rationale |
|---|---:|---|
| Buildability | 5 | Core compiles in firmware path (`pio run -e esp32dev -t compiledb` succeeds), but there is no standalone core target and simulator build depends on external OF layout.
| Reliability | 4 | Several concrete correctness risks in core (missing return, wrong assignment, unchecked buffer writes, null/modulo edge cases).
| Docs | 4 | High-level repo docs exist, but core-specific architecture/build/debug docs are thin.
| Tests | 1 | No unit/integration tests for core logic or topology/animation invariants.
| Portability | 4 | Dual Arduino/openFrameworks abstraction exists, but host build is OF-coupled and firmware integration relies on symlink behavior.
| Maintainability | 4 | Centralized state and heavy raw-pointer ownership with weak ownership contracts increase change risk.

### 5 highest-impact wins

- Add a standalone core build target (host static lib + minimal test harness) to decouple from simulator/firmware scaffolding.
- Fix correctness bugs in `LightList`/`State`/object model selection and add guardrails for null/div-by-zero paths.
- Replace fixed pixel buffer assumptions in `LPLight` segment/link rendering with bounded storage (or explicit clipping).
- Add baseline tests for topology routing, emit/update lifecycle, and palette interpolation behavior.
- Publish core architecture + integration contract docs (engine loop, ownership, protocol mapping from OSC/HTTP to `EmitParams`).

## 2) Codebase Map

### Directory/module map

- `packages/core/src`
- Purpose: shared native engine used by firmware and simulator.
- Notable files:
- `State.*`: frame update, emission, blending, layer lifecycle.
- `LightList.*`: layer config, light allocation, emitting, per-list state.
- `LPLight.*` and `Light.*`: particle data and per-frame updates.
- `LPObject.*`, `Intersection.*`, `Connection.*`, `Port.*`, `Model.*`, `Weight.*`: graph topology and weighted traversal.
- `EmitParams.*`, `Behaviour.*`, `Config.h`: parameter schema and behavior flags.
- `Palette.*`, `Palettes.*`: palette generation/interpolation and preset palettes.
- `objects/*`: concrete topologies (`Heptagon919`, `Heptagon3024`, `Line`, `Cross`, `Triangle`).
- `packages/simulator/src`
- Purpose: openFrameworks adapter around core.
- `main.cpp`: app entrypoint.
- `ofApp.*`: OSC receiver, command parsing, core updates, visualization.

### Key classes/modules and responsibilities

- `State` (`packages/core/src/State.h`): owns active `LightList`s and pixel accumulators; orchestrates `autoEmit`, `emit`, `update`, and final pixel sampling.
- `LightList` (`packages/core/src/LightList.h`): per-layer configuration/state (speed, easing, fade, palette, behavior, blend mode), light allocation/reset/update.
- `LPLight`/`Light` (`packages/core/src/LPLight.h`, `packages/core/src/Light.h`): per-light progression on topology and color/brightness computation.
- `LPObject` (`packages/core/src/LPObject.h`): topology container for intersections/connections/models and command presets.
- `Intersection`/`Connection`/`Port`: route lights across graph edges and optionally out to external sinks.
- `Model`/`Weight`: weighted routing table per model.
- `Palette`/`Palettes`: color handling, interpolation and presets, optional color-rule generation through `ofxColorTheory`.

### Data flow diagram (text)

```text
Inputs
  -> Simulator: UDP OSC messages (/emit, /note_on, /command, ...)
  -> Firmware: OSC + HTTP handlers mapped to EmitParams/layer mutations

Parse/Map
  -> EmitParams + direct layer/state changes

Core Engine (per frame)
  -> State::autoEmit()
  -> State::emit() -> get/create LightList -> choose emitter (intersection/connection)
  -> State::update()
     -> each LightList::update()
     -> each LPLight owner update (Intersection/Connection traversal)
     -> brightness/color resolution + blend into pixel accumulators

Outputs
  -> Firmware: LED drivers consume State::getPixel()
  -> Simulator: ofApp::draw() renders graph circles/segments from State::getPixel()
  -> Optional external propagation via ExternalPort + sendLightViaESPNow callback
```

### Addons/plugins and third-party integration

- `ofxColorTheory` submodule (`vendor/ofxColorTheory`) is used directly in `Palette.h` and `LightList.cpp` for color rules/schemes.
- `ofxOsc` is used in simulator (`packages/simulator/addons.make`, `packages/simulator/src/ofApp.h`) for OSC input.
- `ofxEasing.h` is vendored in core and used heavily for timing/easing.
- `FastNoise.*` is vendored in core for noise-driven behavior.

## 3) Build & Runtime

### Build systems detected

- No standalone core build system in `packages/core` (no CMake/meson/bazel for core itself).
- openFrameworks simulator build surfaces:
- GNU Make (`packages/simulator/Makefile` + `config.make` + `addons.make`).
- QBS (`packages/simulator/simulator.qbs`).
- Xcode project (`packages/simulator/simulator.xcodeproj`).
- Firmware build surface: PlatformIO (`firmware/esp/platformio.ini`), compiling core via symlinked source dir.

### Supported platforms (inferred)

- Confirmed targets in repo tooling:
- ESP32, ESP32-S3 via PlatformIO environments.
- macOS explicitly supported by simulator Xcode config.
- Linux/Windows simulator support is plausible through openFrameworks make/qbs but not confirmed in this audit.

### Dependencies

- Required external:
- openFrameworks checkout for simulator.
- `vendor/ofxColorTheory` submodule.
- PlatformIO + Arduino framework for firmware integration build path.
- Core-local/vendored:
- `ofxEasing.h`
- `FastNoise.*`

### Entrypoints and runtime model

- Simulator entrypoint: `packages/simulator/src/main.cpp` -> `ofRunApp(new ofApp())`.
- Simulator loop:
- `ofApp::update()` -> OSC polling -> `State::autoEmit()` -> `State::update()`.
- `ofApp::draw()` visualizes topology/pixels.
- Firmware integration entrypoint:
- `homo_deus.ino` setup/loop calls `setupState()`, `updateLEDs()`, `drawLEDs()`.
- Core runtime reused identically with Arduino timing/logging macros.

### Current build verification from this audit

- `pio run -e esp32dev -t compiledb` succeeded (core sources compile under firmware toolchain).
- `make -n` in `packages/simulator` failed locally due missing openFrameworks path at expected default (`../../../../openframeworks`).

### Typical failure modes and debugging

- Missing OF checkout or wrong `OF_ROOT` causes simulator make failure.
- Missing/uninitialized submodule (`vendor/ofxColorTheory`) causes compile failures in core palette code.
- Firmware-core path depends on symlink (`firmware/esp/src`) which can be problematic on environments that do not preserve symlinks.
- Debug visibility:
- `LP_LOG*` macros route to `Serial` (Arduino) or `ofLog` (OF).
- `LPDebugger` can visualize intersections/connections/model weights in simulator and firmware (when enabled).

## 4) Architecture & Implementation Analysis

### Update/render pipeline

- Frame-based pipeline centered in `State::update()`:
- clears RGB/divisor accumulators.
- updates each active `LightList`.
- advances each light via owner (`Intersection`/`Connection`).
- resolves color + blend mode per pixel.
- Consumers read final pixels through `State::getPixel()`.

### Timing model

- Frame-driven, variable-step timing using global `gMillis`.
- Duration defaults assume `EmitParams::DURATION_FPS = 62.5` for frame-to-ms conversions.
- No fixed-step integrator; behavior depends on caller loop cadence.

### State management model

- Centralized mutable state object (`State`) with global statics (`gMillis`, `gPerlinNoise`).
- Layer-centric model (`LightList`) plus topology model (`LPObject` + graph components).
- No ECS/scene-graph abstraction; direct object graph with pointer ownership.

### Concurrency model

- Single-threaded by design in both simulator and firmware loop usage.
- No internal locking; static pools/globals are not thread-safe.

### Memory/resource management

- Predominantly raw `new`/`delete` ownership.
- Ownership boundaries are implicit and in places incomplete (see risks in section 6).
- `State` destructor does not release `lightLists` (`packages/core/src/State.h:36`), implying leaks on teardown/object switches.

### Config/parameter system

- `EmitParams` is the core command schema (model/speed/fade/palette/behavior/emit groups, etc.).
- `Config.h` defines behavior/easing/blend enums and compile-time limits.
- Firmware/simulator parse OSC/HTTP into `EmitParams` and mutate layer fields directly.

## 5) Networking / Device Control

### Protocols used (where core is integrated)

- Core itself does not open sockets or host endpoints.
- Simulator adapter uses OSC/UDP receive (`ofxOscReceiver`, port 54321 default).
- Firmware adapter exposes OSC + HTTP (and mDNS/ESP-NOW in firmware layer) and maps those controls to core state.

### Discovery mechanism

- Not in core; discovery (mDNS, etc.) exists in firmware layer.

### Message formats and key integration points

- OSC pair/triplet param contract maps to `EmitParam` IDs and `EmitParams` fields.
- HTTP control panel endpoints (e.g. `/get_layers`, `/update_palette`, `/update_behaviour_flags`) mutate `state->lightLists[...]` fields directly in firmware handlers.
- Core supports external propagation hook via `ExternalPort` + `sendLightViaESPNow` function pointer (`packages/core/src/Port.h:62`).

### Reliability considerations

- No delivery/ack/retry semantics in core external-port callback path.
- Emission path assumes non-empty intersection/connection sets; zero-count cases can lead to invalid modulo/division behavior in `State::getEmitter`.
- Input validation is mostly at adapter layer; core trusts many incoming values.

### Security notes (practical scope)

- Core has no authn/authz layer; trust boundary is firmware/simulator ingress.
- Firmware OSC/HTTP flows are plaintext/unauthenticated by default; core executes mapped commands directly.

## 6) Quality Signals

### Style and consistency

- Mixed modern C++ and C-style patterns (raw arrays/macros/manual memory).
- Naming and layering are mostly understandable but not uniformly encapsulated.

### Error handling patterns

- Primarily log-and-return patterns (`LP_LOGF`, `-1` returns).
- Limited defensive checks in key paths.

### Logging/telemetry

- Basic logging available through compile-time macros.
- No structured telemetry or persistent runtime diagnostics in core itself.

### Testability assessment

- Medium-to-low testability in current form:
- strong coupling to globals/macros.
- no standalone build target.
- topology setup code is deterministic and testable once harness exists.

### Existing tests and coverage gaps

- No unit tests found for core logic.
- No regression tests for emission/routing/blending/palette logic.

### Likely performance hotspots

- `State::update()` per-frame full pixel clear + blend across all active lights.
- Blend-mode switch in `State::setPixel()` can be heavy under large layer counts.
- Palette interpolation and color-rule generation can allocate/work significantly when reconfigured frequently.

### High-signal code risks identified

- Missing return in `LightList::addLightFromMsg` (`packages/core/src/LightList.cpp:81`).
- Wrong assignment in `LightList::setDuration` (`this->duration = duration`) (`packages/core/src/LightList.cpp:91`).
- `LightList::doEmit` uses `uint8_t` counters for potentially larger lists (`packages/core/src/LightList.cpp:224`).
- Potential buffer overflow in segment/link pixel packing (`packages/core/src/LPLight.h:67`, `packages/core/src/LPLight.cpp:82`).
- Potential null dereference in `Intersection::update` (`port->isExternal()` before null check) (`packages/core/src/Intersection.cpp:62`).
- Potential divide/modulo by zero when no emit candidates exist (`packages/core/src/State.cpp:99`, `packages/core/src/State.cpp:106`).
- Const `HashMap::operator[]` is recursively defined (`packages/core/src/HashMap.h:41`).
- Model selection modulo logic likely off-by-one in `Line`, `Cross`, `Triangle` (`packages/core/src/objects/Line.h:31`, `packages/core/src/objects/Cross.h:40`, `packages/core/src/objects/Triangle.h:46`).

## 7) Open Source Readiness Assessment

### Missing for “build core from scratch”

- No standalone core build target independent of simulator/firmware adapters.
- No core-specific quickstart that clearly states supported host compilers/targets and expected toolchain versions.
- Simulator build prerequisites depend on user-managed openFrameworks path/version without explicit pin.

### Docs needed

- `Core Quickstart`: build matrix (firmware-integrated build vs simulator), prerequisites, smoke checks.
- `Core Architecture`: object graph, update loop, ownership/lifecycle.
- `Core Troubleshooting`: submodule/symlink/OF_ROOT issues and common runtime assertions.
- `Core API reference`: `EmitParams`, behavior flags, blend modes, object model command mappings.

### Licensing concerns to verify

- `ofxColorTheory` submodule has LICENSE, but root docs should explicitly call out third-party license obligations.
- `FastNoise.*` includes MIT header, good.
- `ofxEasing.h` has no visible license header in-file; provenance/license should be documented.
- Palette set in `Palettes.*` says derived from FastLED/cpt-city; attribution and license compatibility should be documented explicitly.

### Dev tooling suggestions (without overengineering)

- Add `clang-format` config for `packages/core/src` + basic format check in CI.
- Add a minimal core host test job (even one executable) once standalone target exists.
- Add optional static analysis pass (`clang-tidy` or compiler warnings-as-errors for core only).

## 8) Risks and Unknowns

### Assumptions made

- `packages/core/src` is canonical core source used by both firmware and simulator.
- Firmware symlink (`firmware/esp/src`) remains the intended integration mechanism.
- Simulator is intended as the primary host runtime for visual verification of core behavior.

### Top unknowns blocking confidence and fastest confirmations

- Exact openFrameworks version compatibility:
- Fast check: pin and document a known-good OF version; run simulator build in CI with that image/cache.
- Whether simulator GNU Make path currently compiles core sources (not only headers):
- Fast check: perform full `make` with real OF checkout and inspect linked objects.
- Intended bounds for segment/link rendering buffers:
- Fast check: add assertions/tests for max connection length vs `CONNECTION_MAX_LEDS`.
- Intended model-selection semantics for non-heptagon objects:
- Fast check: run command-model mapping tests for `Line/Cross/Triangle` and validate expected model IDs.
- Ownership model for `Connection`/`Port` lifecycle:
- Fast check: run ASan-enabled host harness and inspect leaks/use-after-free across create/destroy cycles.

## 9) Roadmap (Phased, Concrete, Prioritized)

## Phase 0: documentation + reproducible build (low risk)

### Task 0.1
Goal: document core build surfaces and prerequisites clearly.
Files/modules: `docs/CORE_AUDIT_REPORT.md`, `docs/build.md` (and optional new `docs/core-build.md`).
Acceptance criteria: contributor can follow documented steps to verify core through firmware-compiledb and simulator prerequisites without guesswork.
Complexity: S.
Dependencies: none.

### Task 0.2
Goal: codify integration invariants for shared core wiring.
Files/modules: CI workflow notes, docs.
Acceptance criteria: docs explicitly state symlink requirement and submodule requirement with verification commands.
Complexity: S.
Dependencies: 0.1.

### Task 0.3
Goal: add architecture primer for newcomers.
Files/modules: docs.
Acceptance criteria: one concise architecture document covering update loop, topology, and ingress mapping to `EmitParams`.
Complexity: S.
Dependencies: 0.1.

## Phase 1: safe cleanup and modularization (low/medium risk)

### Task 1.1
Goal: fix low-risk correctness bugs without changing architecture.
Files/modules: `LightList.cpp`, `State.cpp`, `objects/Line.h`, `objects/Cross.h`, `objects/Triangle.h`, `Intersection.cpp`.
Acceptance criteria: missing-return/assignment/modulo/null checks fixed; firmware build and simulator compile still pass.
Complexity: M.
Dependencies: 0.1.

### Task 1.2
Goal: enforce safe bounds for pixel packing paths.
Files/modules: `LPLight.h`, `LPLight.cpp`, `Config.h`.
Acceptance criteria: no out-of-bounds writes in segment/link mode for worst-case connections; behavior defined when over limit.
Complexity: M.
Dependencies: 1.1.

### Task 1.3
Goal: clarify and harden ownership semantics.
Files/modules: `State.h/.cpp`, `Connection.h/.cpp`, `Model.h`.
Acceptance criteria: deterministic cleanup path documented and implemented for core-owned resources.
Complexity: M.
Dependencies: 1.1.

## Phase 2: tests, CI, reliability improvements (medium)

### Task 2.1
Goal: add standalone core host target for tests.
Files/modules: new `packages/core` build files (e.g., CMake or simple host make), minimal harness.
Acceptance criteria: core compiles outside OF/Arduino adapters on CI host.
Complexity: M.
Dependencies: 1.1.

### Task 2.2
Goal: add regression tests for routing and lifecycle.
Files/modules: test sources around `State`, `LPObject`, `LightList`, `Palette`.
Acceptance criteria: tests cover emit/update expiry, model routing, palette interpolation, and edge cases (empty emit groups).
Complexity: M.
Dependencies: 2.1.

### Task 2.3
Goal: add sanitizer/strict-warning CI lane for core host target.
Files/modules: CI workflow.
Acceptance criteria: ASan/UBSan pass on core test suite; warnings tracked as actionable.
Complexity: M.
Dependencies: 2.1, 2.2.

## Phase 3: architectural refactors (optional/high risk)

### Task 3.1
Goal: decouple platform abstractions from `Config.h` include model.
Files/modules: `Config.h` and platform adapter headers.
Acceptance criteria: host/firmware builds select platform adapters explicitly, reducing macro-driven coupling.
Complexity: L.
Dependencies: 2.1.

### Task 3.2
Goal: modernize ownership model.
Files/modules: core graph/layer classes.
Acceptance criteria: replace fragile raw ownership with RAII containers/smart pointers where ownership is clear.
Complexity: L.
Dependencies: 3.1.

### Task 3.3
Goal: split engine from transport/control contracts.
Files/modules: engine modules vs adapter modules.
Acceptance criteria: clean boundaries where OSC/HTTP/ESP-NOW mappings live outside engine internals.
Complexity: L.
Dependencies: 3.1.

---

## Changes made

### Phase 0 docs-only

- Added `docs/core-build.md` with explicit shared-core verification steps:
- submodule init
- firmware symlink check
- firmware `compiledb` smoke command
- simulator `OF_ROOT` dry-run guidance
- Updated `docs/build.md` to point to `docs/core-build.md`.
- Updated `README.md` key docs list to include `docs/core-build.md`.

### Phase 1 safe runtime fixes (implemented)

- Fixed `LightList::addLightFromMsg` missing return and `LightList::setDuration` wrong self-assignment.
- Removed `uint8_t` truncation risk in `LightList::doEmit` loop counters.
- Added emit-time guard for invalid model IDs in `State::emit`.
- Fixed `LP_OSC_REPLY` index variable usage (`index` instead of undefined `i`).
- Prevented `Behaviour` leak on early max-light rejection in `State::setupListFrom`.
- Added zero-candidate guards in `State::getEmitter` to avoid modulo/division by zero.
- Fixed null-dereference path in `Intersection::update` when no port is chosen.
- Added bounds clipping in `LPLight` segment/link pixel packing to prevent writes past `pixels`.
- Fixed off-by-one model modulo in `Line`, `Cross`, `Triangle` `getModelParams`.
- Fixed recursive const access bug in `HashMap::operator[]` and made lookup helpers const.

Validation after Phase 1:
- `pio run -e esp32dev -t compiledb` succeeded in `firmware/esp`.
- `make -n` in `packages/simulator` still fails only due missing local openFrameworks path (`../../../../openframeworks`), unchanged from pre-fix state.

## Appendix A: Search-driven findings

Keyword scans performed across `packages/core` and `packages/simulator`:

- Networking terms (`udp|osc|websocket|http|mdns|ssdp|espnow|tcp`): simulator OSC usage and core `ExternalPort` callback hook were found; no core-native socket stack.
- Parameter/config terms (`param|settings|json|palette|EmitParam|behaviourFlags`): `EmitParams`, palette system, behavior flags, and simulator parser paths confirmed.
- Timing terms (`frame|millis|fps|update`): frame-based timing found in `State`, `LightList`, `LPDebugger`, simulator `ofApp`.
- Framework/addon terms (`ofx|openFrameworks|glm|ofMain|Arduino`): dual-target abstraction in `Config.h`, OF integration in simulator, direct `ofxColorTheory` integration in core.

## Appendix B: Known TODO/FIXME markers

- `packages/core/src/State.cpp`: `todo` on max-light overflow strategy and pixel retrieval note.
- `packages/core/src/LightList.cpp`: `todo` on behavior gate and split trail handling.
- `packages/core/src/objects/Heptagon919.cpp`: `todo` maxLength for all models.
- `packages/core/src/objects/Heptagon3024.cpp`: `todo` maxLength for all models.
- `packages/core/src/objects/HeptagonStar.h`: `todo` implement `getXYZ`.
