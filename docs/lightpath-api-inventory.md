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

All symbols are currently exposed from `packages/lightpath/src/*` headers (no dedicated `include/` boundary yet).

### Core config/types

- `packages/lightpath/src/Config.h`
  - `ColorRGB`
  - `Groups`, `BehaviourFlags`, `ListOrder`, `ListHead`, `BlendMode`, `Ease`
  - global compile-time constants/macros (e.g. `MAX_GROUPS`, `MAX_LIGHT_LISTS`, `RANDOM_COLOR`, `INFINITE_DURATION`)

- `packages/lightpath/src/EmitParams.h`
  - `EmitParam`
  - `EmitParams`

- `packages/lightpath/src/Globals.h`
  - globals: `gPerlinNoise`, `gMillis`

- `packages/lightpath/src/LPRandom.h`
  - `LPRandom` static random/config API

### Topology/routing

- `packages/lightpath/src/LPOwner.h`
  - `LPOwner`

- `packages/lightpath/src/Port.h`
  - `Port`, `InternalPort`, `ExternalPort`
  - `sendLightViaESPNow` hook

- `packages/lightpath/src/Intersection.h`
  - `Intersection`

- `packages/lightpath/src/Connection.h`
  - `Connection`

- `packages/lightpath/src/Weight.h`
  - `Weight`

- `packages/lightpath/src/Model.h`
  - `Model`

- `packages/lightpath/src/LPObject.h`
  - `LPObject`, `PixelGap`

### Runtime/state

- `packages/lightpath/src/LPLight.h`
  - `LPLight`

- `packages/lightpath/src/Light.h`
  - `Light`

- `packages/lightpath/src/Behaviour.h`
  - `Behaviour`

- `packages/lightpath/src/LightList.h`
  - `LightList`, `LightMessage`

- `packages/lightpath/src/BgLight.h`
  - `BgLight`

- `packages/lightpath/src/State.h`
  - `State` (+ static `State::autoParams`)

### Palette/rendering

- `packages/lightpath/src/Palette.h`
  - `Palette`
  - wrap mode macros/constants (`WRAP_*`)

- `packages/lightpath/src/Palettes.h`
  - predefined palette factories
  - `getPaletteCount()`, `getPalette()`

### Debug/utilities

- `packages/lightpath/src/LPDebugger.h`
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
  - direct includes: `Heptagon919.h`, `Heptagon3024.h`, `Line.h`, `Cross.h`, `Triangle.h`, `LPDebugger.h`

- `apps/simulator/src/ofApp.cpp`
  - direct includes: `Globals.h`, `LightList.h`, `LPRandom.h`

- `firmware/esp/LightPath.h`
  - direct includes: `src/objects/Heptagon919.h`, `src/objects/Heptagon3024.h`, `src/objects/Line.h`, `src/objects/Triangle.h`, `src/Globals.h`, `src/LightList.h`, `src/LPRandom.h`

- `firmware/esp/homo_deus.ino`
  - optional direct include: `src/LPDebugger.h` (under `DEBUGGER_ENABLED`)

- `firmware/esp/WebServerLayers.h`
  - direct include: `src/Palettes.h`

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
