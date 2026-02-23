# MeshLED Documentation

MeshLED is a networked light platform built around a shared C++ engine (`lightpath`), ESP32 firmware, and a browser control panel.

<div class="meshled-hero">
  <div class="meshled-hero__title">From node graph to photons in one stack</div>
  <div class="meshled-hero__copy">
    Use this site to build firmware, control layers, integrate OSC workflows, and migrate from WLED-style setups.
  </div>
</div>

## Start Here

- **Build + run basics**: `build.md`
- **Firmware HTTP contract**: `firmware-api.md`
- **OSC message contract**: `osc-contract.md`
- **WLED migration guide**: `wled-users.md`

## Platform Surfaces

| Surface | Path | Role |
| --- | --- | --- |
| Core engine | `packages/lightpath` | topology-aware light runtime and rendering |
| Firmware | `firmware/esp` | device runtime + OSC/HTTP endpoints |
| Control panel | `apps/control-panel` | browser UI for model and layer control |

## For WLED Users

If your baseline is WLED, MeshLED should feel familiar in deployment flow while adding topology-aware runtime behavior and deeper OSC integration.

See the full migration map: `wled-users.md`.
