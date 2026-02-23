# Lightpath API and Parent Usage Inventory (Phase 0)

Date: 2026-02-23
Workspace: `/Users/kasparsj/Work2/meshled`
Submodule: `/Users/kasparsj/Work2/meshled/packages/lightpath`

## Baseline Build/Test Snapshot

### Parent repo checks

- Core host tests (standalone submodule invocation):
  - `cmake -S packages/lightpath -B packages/lightpath/build-baseline -DLIGHTPATH_CORE_BUILD_TESTS=ON`
  - `cmake --build packages/lightpath/build-baseline --parallel`
  - `ctest --test-dir packages/lightpath/build-baseline --output-on-failure`
  - Result: pass (2/2 tests)

- Firmware wiring and smoke:
  - `readlink firmware/esp/src`
  - Result: `../../packages/lightpath/src`
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

### Lightpath profile matrix (clean baseline dirs)

- ASan:
  - `CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-asan-baseline -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_ASAN=ON`
  - `cmake --build packages/lightpath/build-asan-baseline --parallel`
  - `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir packages/lightpath/build-asan-baseline --output-on-failure`
  - Result: pass

- UBSan:
  - `CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-ubsan-baseline -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_UBSAN=ON`
  - `cmake --build packages/lightpath/build-ubsan-baseline --parallel`
  - `ctest --test-dir packages/lightpath/build-ubsan-baseline --output-on-failure`
  - Result: pass

- Strict warnings:
  - `CC=clang CXX=clang++ cmake -S packages/lightpath -B packages/lightpath/build-warnings-baseline -DLIGHTPATH_CORE_BUILD_TESTS=ON -DLIGHTPATH_CORE_ENABLE_STRICT_WARNINGS=ON`
  - `cmake --build packages/lightpath/build-warnings-baseline --parallel`
  - `ctest --test-dir packages/lightpath/build-warnings-baseline --output-on-failure`
  - Result: pass

## Current API Map (de-facto public surface)

Public symbols are exposed via `packages/lightpath/include/lightpath/*` headers.
Internal definitions live under module-scoped `packages/lightpath/src/*` folders.

### Core config/types

- `packages/lightpath/src/core/Types.h`
  - `ColorRGB`
  - `Groups`, `BehaviourFlags`, `ListOrder`, `ListHead`, `BlendMode`, `Ease`

- `packages/lightpath/src/core/Limits.h`
  - global compile-time limits/macros (e.g. `MAX_GROUPS`, `MAX_LIGHT_LISTS`, `RANDOM_COLOR`, `INFINITE_DURATION`)

- `packages/lightpath/src/core/Platform.h`
  - platform adapters/macros (`LP_LOG*`, `LP_RANDOM`, `LP_STRING`)

- `packages/lightpath/src/runtime/EmitParams.h`
  - `EmitParam`
  - `EmitParams`

- `packages/lightpath/src/Globals.h`
  - globals: `gPerlinNoise`, `gMillis`

- `packages/lightpath/src/LPRandom.h`
  - `LPRandom` static random/config API

### Topology/routing

- `packages/lightpath/src/topology/LPOwner.h`
  - `LPOwner`

- `packages/lightpath/src/topology/Port.h`
  - `Port`, `InternalPort`, `ExternalPort`
  - `sendLightViaESPNow` hook

- `packages/lightpath/src/topology/Intersection.h`
  - `Intersection`

- `packages/lightpath/src/topology/Connection.h`
  - `Connection`

- `packages/lightpath/src/topology/Weight.h`
  - `Weight`

- `packages/lightpath/src/topology/Model.h`
  - `Model`

- `packages/lightpath/src/topology/LPObject.h`
  - `LPObject`, `PixelGap`

### Runtime/state

- `packages/lightpath/src/runtime/LPLight.h`
  - `LPLight`

- `packages/lightpath/src/runtime/Light.h`
  - `Light`

- `packages/lightpath/src/runtime/Behaviour.h`
  - `Behaviour`

- `packages/lightpath/src/runtime/LightList.h`
  - `LightList`, `LightMessage`

- `packages/lightpath/src/runtime/BgLight.h`
  - `BgLight`

- `packages/lightpath/src/runtime/State.h`
  - `State` (+ static `State::autoParams`)

### Palette/rendering

- `packages/lightpath/src/rendering/Palette.h`
  - `Palette`
  - wrap mode macros/constants (`WRAP_*`)

- `packages/lightpath/src/rendering/Palettes.h`
  - predefined palette factories
  - `getPaletteCount()`, `getPalette()`

### Debug/utilities

- `packages/lightpath/src/debug/LPDebugger.h`
  - `LPDebugger`

- `packages/lightpath/src/HashMap.h`
  - `HashMap`

- `packages/lightpath/src/FastNoise.h`
  - `FastNoise`

### Built-in topology objects

- `packages/lightpath/src/objects/HeptagonStar.h`
  - `HeptagonStar`, `HeptagonStarModel`

- `packages/lightpath/src/objects/Heptagon919.h`
  - `Heptagon919`, `HEPTAGON919_*`

- `packages/lightpath/src/objects/Heptagon3024.h`
  - `Heptagon3024`, `HEPTAGON3024_*`

- `packages/lightpath/src/objects/Line.h`
  - `Line`, `LineModel`, `LINE_PIXEL_COUNT`

- `packages/lightpath/src/objects/Cross.h`
  - `Cross`, `CrossModel`, `CROSS_PIXEL_COUNT`

- `packages/lightpath/src/objects/Triangle.h`
  - `Triangle`, `TriangleModel`, `TRIANGLE_PIXEL_COUNT`

## Parent Usage Map (outside `packages/lightpath`)

### Include-level dependencies

- `apps/simulator/src/ofApp.h`
  - direct include: `lightpath/lightpath.hpp`

- `apps/simulator/src/ofApp.cpp`
  - uses `lightpath` symbols via `ofApp.h`

- `firmware/esp/LightPath.h`
  - direct include: `lightpath/lightpath.hpp`

- `firmware/esp/homo_deus.ino`
  - includes `LightPath.h` (which includes `lightpath/lightpath.hpp`)

- `firmware/esp/WebServerLayers.h`
  - direct include: `lightpath/rendering.hpp`

- `firmware/esp/ofxColorTheoryCompat.cpp`
  - direct include of vendor `.cpp` files from `packages/lightpath/vendor/ofxColorTheory/src/*`

### Symbol-level usage hotspots

- `apps/simulator/src/ofApp.cpp`
  - object/runtime construction: `Heptagon919`, `Heptagon3024`, `Line`, `Cross`, `Triangle`, `State`, `LPDebugger`
  - emit/runtime API: `EmitParams`, `EmitParam`, `state->emit`, `state->update`, `state->stopNote`, `state->stopAll`, `state->paletteColor`
  - topology traversal: `LPObject::inter`, `LPObject::conn`, `Intersection`, `Connection`
  - render constants/types: `MAX_GROUPS`, `MAX_NOTES_SET`, `INFINITE_DURATION`, `RANDOM_COLOR`, `ListOrder`, `ListHead`, `ColorRGB`, `gMillis`

- `firmware/esp/LEDLib.h`
  - object/runtime creation: `Heptagon919`, `Heptagon3024`, `Line`, `Triangle`, `State`
  - control surface: `State::autoParams`, `state->autoEmit`, `state->update`, `state->stopAll`, `state->colorAll`, `state->splitAll`
  - command emit: `EmitParams`, `LPObject::getParams`

- `firmware/esp/OSCLib.h`
  - OSC mapping to API: `EmitParams`, `EmitParam`, `ListOrder`, `RANDOM_COLOR`, `INFINITE_DURATION`, `MAX_NOTES_SET`
  - palette updates: `ColorRGB`, `state->paletteColor`, direct `lightList->palette` mutation

- `firmware/esp/WebServerLayers.h`
  - palette APIs: `Palette`, `getPaletteCount`, `getPalette`
  - layer APIs: `LightList`, `BgLight` assumptions, blend/ease/list fields

- `firmware/esp/WebServerSetup.h`
  - direct graph mutation: `Intersection`, `Connection`, `Model`, `LPObject::inter/conn/models`
  - topology limits/constants: `MAX_GROUPS`

- `firmware/esp/WLEDApiLib.h`, `firmware/esp/FSLib.h`, `firmware/esp/ESPNowLib.h`, `firmware/esp/NeoPixelBusLib.h`, `firmware/esp/FastLEDLib.h`
  - direct use of `State`, `LightList`, `ColorRGB`, `Palette`, and pixel translation methods on `LPObject`

## Notes

- The parent repo currently depends on a broad, internal-facing surface of `lightpath/src` headers and mutable runtime internals.
- Most parent integration points are direct-struct/class field access rather than a narrow facade API.
- Next phase will introduce a documented namespaced public header layer under `packages/lightpath/include/lightpath/`, with compatibility retained so parent migration can be incremental.
