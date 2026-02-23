# Core Architecture Primer

Date: 2026-02-23
Scope: `packages/lightpath/src`

## Purpose

The core is a shared C++ light engine that runs in both:

- firmware (`firmware/esp`)
- simulator (`apps/simulator`)

It is responsible for:

- topology-aware light routing (`Intersection`, `Connection`, `Port`)
- per-layer light lifecycle (`LightList`, `LPLight`, `Light`)
- frame updates and pixel compositing (`State`)
- parameter-driven behavior (`EmitParams`, `Behaviour`, `Palette`)

## Module Map

- `State.*`
- owns active light layers and per-pixel accumulators
- runs frame update and blending

- `LightList.*`
- per-layer config/state: speed, fade, easing, palette, blend mode, note binding
- owns the `LPLight*` array for that layer

- `LPLight.*`, `Light.*`
- per-light runtime state: owner, position, brightness, expiration, color

- `LPObject.*`, `Intersection.*`, `Connection.*`, `Port.*`, `Model.*`, `Weight.*`
- topology graph and weighted routing rules

- `EmitParams.*`, `Behaviour.*`, `Config.h`
- command schema and behavior flags/enums

- `Palette.*`, `Palettes.*`
- color interpolation, wrapping, and preset palette materialization

## Runtime Flow

```text
External input (OSC/HTTP/UI command)
  -> adapter parses input into EmitParams / layer mutations
  -> State::emit(...) or direct layer/state updates

Per-frame update
  -> State::autoEmit(...)
  -> State::update()
     -> clear pixel accumulators
     -> each LightList::update()
        -> each light owner update (Intersection/Connection)
        -> expire/remove finished lights
     -> State::updateLight() + blend via State::setPixel()

Output
  -> firmware reads State::getPixel(i) for LED drivers
  -> simulator reads State::getPixel(i) for visualization
```

## Ownership and Lifecycle

- `State` owns `lightLists[]` pointers and deletes non-background lists when they expire.
- slot `0` is reserved for background (`BgLight`) and remains allocated for state lifetime.
- `LightList` owns its `lights[]` array and deletes contained lights in destructor/reset paths.
- `Model` owns `Weight*` entries and deletes them in destructor.
- topology objects (`LPObject` graph) are created by concrete object types (`Line`, `Cross`, `Triangle`, etc.) and live for object lifetime.

## Adapter Contract (Core Boundary)

Core does not open network sockets. Adapters are responsible for transport:

- Simulator ingress: OSC/UDP via `ofxOsc`.
- Firmware ingress: OSC + HTTP endpoints.

Adapters map incoming controls to:

- `EmitParams` for new emissions
- direct `LightList` mutations for runtime controls (palette, behavior flags, layer visibility, etc.)

## Timing Model

- Frame-based with global `gMillis`.
- No fixed-timestep scheduler inside core.
- Expiration and easing behavior depend on caller update cadence.

## Known Constraints

- Single-threaded model; core is not thread-safe.
- Platform abstraction is macro-driven in `Config.h` (Arduino vs openFrameworks shims).
- Raw-pointer ownership is still used throughout major paths.
- Host tests provide coverage for baseline lifecycle/blend behavior, but complex-topology long-run coverage is still limited.

## Quick Verification

```bash
cmake -S packages/lightpath -B packages/lightpath/build -DLIGHTPATH_CORE_BUILD_TESTS=ON
cmake --build packages/lightpath/build
ctest --test-dir packages/lightpath/build --output-on-failure
```
