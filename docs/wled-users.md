# Migration Guide for WLED Users

MeshLED targets users who already run WLED-class LED systems and want topology-aware behaviors plus OSC-native control.

## Concept Mapping

| WLED mental model | MeshLED equivalent | Notes |
| --- | --- | --- |
| Segment + effect | Layer + emit behavior | Layers are runtime-controlled, with explicit behavior flags and palette controls. |
| Preset-driven control | API + OSC command surface | MeshLED favors external control surfaces (UI, OSC, custom tools). |
| Flat strip-first topology | Model/intersection/connection graph | Topology is first-class and informs routing and animation. |
| JSON-state workflows | HTTP endpoint and OSC contracts | Contracts are documented in `firmware-api.md` and `osc-contract.md`. |

## What Feels Familiar

- ESP32 firmware workflow (`PlatformIO` build/flash).
- Browser-based control panel for runtime changes.
- Layer and palette oriented operation.

## What Is New

- Shared C++ engine (`lightpath`) reused by firmware and simulator.
- Topology-aware emit routing via intersections/connections/models.
- OSC contract designed for live performance tooling.

## Suggested First Session

1. Build firmware from `/firmware/esp`.
2. Open the control panel and verify `/get_layers`, `/get_model`, and `/get_colors`.
3. Send OSC `/emit` and `/note_on` messages and inspect runtime behavior.
4. Tune layer behavior flags, easing, and palette settings.

## Integration Docs

- Build and verification: `build.md`
- Firmware HTTP API: `firmware-api.md`
- OSC contract: `osc-contract.md`
- UI/Firmware compatibility matrix: `ui-firmware-compat.md`
