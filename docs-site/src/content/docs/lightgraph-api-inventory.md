---
title: "Lightgraph API and Parent Usage Inventory (Phase 0)"
---

Date: 2026-02-23
Workspace: `/Users/kasparsj/Work2/meshled`
Submodule: `/Users/kasparsj/Work2/meshled/packages/lightgraph`

## Baseline Build/Test Snapshot

### Parent repo checks

- Core host tests (standalone submodule invocation):
  - `cmake -S packages/lightgraph -B packages/lightgraph/build-baseline -DLIGHTGRAPH_CORE_BUILD_TESTS=ON`
  - `cmake --build packages/lightgraph/build-baseline --parallel`
  - `ctest --test-dir packages/lightgraph/build-baseline --output-on-failure`
  - Result: pass (2/2 tests)

- Firmware wiring and smoke:
  - `readlink firmware/esp/src`
  - Result: `../../packages/lightgraph/src`
  - `pio run -e esp32dev -t compiledb`
  - Result: pass

- Simulator smoke:
  - `make -n` (in `apps/simulator`)
  - Result: failed due missing openFrameworks path (`../../../../openframeworks/.../compile.project.mk`)

- Control panel CI-style checks:
  - `npm ci --legacy-peer-deps`
  - `npm run lint` (warnings only)
  - `npm run build`
  - Result: pass

### Lightgraph profile matrix (clean baseline dirs)

- ASan:
  - `CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-asan-baseline -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_ASAN=ON`
  - `cmake --build packages/lightgraph/build-asan-baseline --parallel`
  - `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir packages/lightgraph/build-asan-baseline --output-on-failure`
  - Result: pass

- UBSan:
  - `CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-ubsan-baseline -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_UBSAN=ON`
  - `cmake --build packages/lightgraph/build-ubsan-baseline --parallel`
  - `ctest --test-dir packages/lightgraph/build-ubsan-baseline --output-on-failure`
  - Result: pass

- Strict warnings:
  - `CC=clang CXX=clang++ cmake -S packages/lightgraph -B packages/lightgraph/build-warnings-baseline -DLIGHTGRAPH_CORE_BUILD_TESTS=ON -DLIGHTGRAPH_CORE_ENABLE_STRICT_WARNINGS=ON`
  - `cmake --build packages/lightgraph/build-warnings-baseline --parallel`
  - `ctest --test-dir packages/lightgraph/build-warnings-baseline --output-on-failure`
  - Result: pass

## Current API Map (de-facto public surface)

Public symbols are exposed via `packages/lightgraph/include/lightgraph/*` headers.
Internal definitions live under module-scoped `packages/lightgraph/src/*` folders.

### Core config/types

- `packages/lightgraph/src/core/Types.h`
  - `ColorRGB`
  - `Groups`, `BehaviourFlags`, `ListOrder`, `ListHead`, `BlendMode`, `Ease`

- `packages/lightgraph/src/core/Limits.h`
  - global compile-time limits/macros (e.g. `MAX_GROUPS`, `MAX_LIGHT_LISTS`, `RANDOM_COLOR`, `INFINITE_DURATION`)

- `packages/lightgraph/src/core/Platform.h`
  - platform adapters/macros (`LP_LOG*`, `LP_RANDOM`, `LP_STRING`)

- `packages/lightgraph/src/runtime/EmitParams.h`
  - `EmitParam`
  - `EmitParams`

- `packages/lightgraph/src/Globals.h`
  - globals: `gPerlinNoise`, `gMillis`

- `packages/lightgraph/src/Random.h`
  - `Random` static random/config API

### Topology/routing

- `packages/lightgraph/src/topology/Owner.h`
  - `Owner`

- `packages/lightgraph/src/topology/Port.h`
  - `Port`, `InternalPort`, `ExternalPort`
  - `sendLightViaESPNow` hook

- `packages/lightgraph/src/topology/Intersection.h`
  - `Intersection`

- `packages/lightgraph/src/topology/Connection.h`
  - `Connection`

- `packages/lightgraph/src/topology/Weight.h`
  - `Weight`

- `packages/lightgraph/src/topology/Model.h`
  - `Model`

- `packages/lightgraph/src/topology/TopologyObject.h`
  - `TopologyObject`, `PixelGap`

### Runtime/state

- `packages/lightgraph/src/runtime/RuntimeLight.h`
  - `RuntimeLight`

- `packages/lightgraph/src/runtime/Light.h`
  - `Light`

- `packages/lightgraph/src/runtime/Behaviour.h`
  - `Behaviour`

- `packages/lightgraph/src/runtime/LightList.h`
  - `LightList`, `LightMessage`

- `packages/lightgraph/src/runtime/BgLight.h`
  - `BgLight`

- `packages/lightgraph/src/runtime/State.h`
  - `State` (+ static `State::autoParams`)

### Palette/rendering

- `packages/lightgraph/src/rendering/Palette.h`
  - `Palette`
  - wrap mode macros/constants (`WRAP_*`)

- `packages/lightgraph/src/rendering/Palettes.h`
  - predefined palette factories
  - `getPaletteCount()`, `getPalette()`

### Debug/utilities

- `packages/lightgraph/src/debug/Debugger.h`
  - `Debugger`

- `packages/lightgraph/src/HashMap.h`
  - `HashMap`

- `packages/lightgraph/src/FastNoise.h`
  - `FastNoise`

### Built-in topology objects

- `packages/lightgraph/src/objects/HeptagonStar.h`
  - `HeptagonStar`, `HeptagonStarModel`

- `packages/lightgraph/src/objects/Heptagon919.h`
  - `Heptagon919`, `HEPTAGON919_*`

- `packages/lightgraph/src/objects/Heptagon3024.h`
  - `Heptagon3024`, `HEPTAGON3024_*`

- `packages/lightgraph/src/objects/Line.h`
  - `Line`, `LineModel`, `LINE_PIXEL_COUNT`

- `packages/lightgraph/src/objects/Cross.h`
  - `Cross`, `CrossModel`, `CROSS_PIXEL_COUNT`

- `packages/lightgraph/src/objects/Triangle.h`
  - `Triangle`, `TriangleModel`, `TRIANGLE_PIXEL_COUNT`

## Parent Usage Map (outside `packages/lightgraph`)

### Include-level dependencies

- `apps/simulator/src/ofApp.h`
  - direct include: `lightgraph/lightgraph.hpp`

- `apps/simulator/src/ofApp.cpp`
  - uses `lightgraph` symbols via `ofApp.h`

- `firmware/esp/LightGraph.h`
  - direct include: `lightgraph/lightgraph.hpp`

- `firmware/esp/esp.ino`
  - includes `LightGraph.h` (which includes `lightgraph/lightgraph.hpp`)

- `firmware/esp/WebServerLayers.h`
  - direct include: `lightgraph/rendering.hpp`

- `firmware/esp/ofxColorTheoryCompat.cpp`
  - direct include of vendor `.cpp` files from `packages/lightgraph/vendor/ofxColorTheory/src/*`

### Symbol-level usage hotspots

- `apps/simulator/src/ofApp.cpp`
  - object/runtime construction: `Heptagon919`, `Heptagon3024`, `Line`, `Cross`, `Triangle`, `State`, `Debugger`
  - emit/runtime API: `EmitParams`, `EmitParam`, `state->emit`, `state->update`, `state->stopNote`, `state->stopAll`, `state->paletteColor`
  - topology traversal: `TopologyObject::inter`, `TopologyObject::conn`, `Intersection`, `Connection`
  - render constants/types: `MAX_GROUPS`, `MAX_NOTES_SET`, `INFINITE_DURATION`, `RANDOM_COLOR`, `ListOrder`, `ListHead`, `ColorRGB`, `gMillis`

- `firmware/esp/LEDLib.h`
  - object/runtime creation: `Heptagon919`, `Heptagon3024`, `Line`, `Triangle`, `State`
  - control surface: `State::autoParams`, `state->autoEmit`, `state->update`, `state->stopAll`, `state->colorAll`, `state->splitAll`
  - command emit: `EmitParams`, `TopologyObject::getParams`

- `firmware/esp/OSCLib.h`
  - OSC mapping to API: `EmitParams`, `EmitParam`, `ListOrder`, `RANDOM_COLOR`, `INFINITE_DURATION`, `MAX_NOTES_SET`
  - palette updates: `ColorRGB`, `state->paletteColor`, direct `lightList->palette` mutation

- `firmware/esp/WebServerLayers.h`
  - palette APIs: `Palette`, `getPaletteCount`, `getPalette`
  - layer APIs: `LightList`, `BgLight` assumptions, blend/ease/list fields

- `firmware/esp/WebServerSetup.h`
  - direct graph mutation: `Intersection`, `Connection`, `Model`, `TopologyObject::inter/conn/models`
  - topology limits/constants: `MAX_GROUPS`

- `firmware/esp/WLEDApiLib.h`, `firmware/esp/FSLib.h`, `firmware/esp/ESPNowLib.h`, `firmware/esp/NeoPixelBusLib.h`, `firmware/esp/FastLEDLib.h`
  - direct use of `State`, `LightList`, `ColorRGB`, `Palette`, and pixel translation methods on `TopologyObject`

## Notes

- The parent repo currently depends on a broad, internal-facing surface of `lightgraph/src` headers and mutable runtime internals.
- Most parent integration points are direct-struct/class field access rather than a narrow facade API.
- Next phase will introduce a documented namespaced public header layer under `packages/lightgraph/include/lightgraph/`, with compatibility retained so parent migration can be incremental.
