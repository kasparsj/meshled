---
title: "UI/Firmware Compatibility Matrix"
---

Date: 2026-02-28  
UI target: `apps/control-panel` (package version `0.0.0`)  
Firmware target: `firmware/esp` on `meshled/main`

## Purpose

Define which firmware endpoints are required by the React control panel and what contract details must stay stable.

## Compatibility Matrix

| Feature | UI location | Endpoint(s) | Required contract | Current status |
|---|---|---|---|---|
| Device info | `apps/control-panel/src/hooks/useDeviceInfo.js` | `GET /device_info` | JSON object with at least `wifi.ssid` (active SSID), `wifi.mode`, `ip`, `leds.pwr` | Compatible |
| Load layers | `apps/control-panel/src/hooks/useLayers.js` | `GET /get_layers` | JSON array of layer objects (`id`, `visible`, `brightness`, `speed`, `palette`, etc.) | Compatible |
| Toggle layer visible | `apps/control-panel/src/hooks/useLayers.js` | `POST /toggle_visible` | Accept form args `layer`, `visible`; any 2xx/3xx success | Compatible (302 redirect behavior) |
| Brightness/speed/offset | `apps/control-panel/src/hooks/useLayers.js` | `POST /update_layer_brightness`, `POST /update_speed`, `POST /update_layer_offset` | Accept form args and persist change | Compatible |
| Layer easing/behavior/reset | `apps/control-panel/src/hooks/useLayers.js` | `POST /update_ease`, `POST /update_behaviour_flags`, `POST /reset_layer` | Accept form args; return success status | Compatible |
| Add/remove layer | `apps/control-panel/src/hooks/useLayers.js` | `POST /add_layer`, `POST /remove_layer` | Mutate layer set; return success status | Compatible |
| Layer palette editing | `apps/control-panel/src/hooks/useLayers.js` | `POST /update_palette`, `GET /get_palette_colors` | `update_palette` accepts form args + JSON strings for colors/positions | Compatible |
| User palette library | `apps/control-panel/src/hooks/usePalettes.js` | `GET /get_palettes`, `POST /save_palette`, `POST /delete_palette`, `POST /sync_palettes` | JSON contract for palette objects | Compatible |
| Settings read/write | `apps/control-panel/src/hooks/useSettings.js` | `GET /get_settings`, `POST /update_settings` | `get_settings` JSON keys expected by settings UI (including `activeSSID`, `apMode`); `update_settings` accepts form args | Compatible |
| WiFi + reboot actions | `apps/control-panel/src/hooks/useSettings.js` | `POST /update_wifi`, `POST /restart` | Commands trigger restart; response may be interrupted by reboot | Partially compatible (client should tolerate disconnect) |
| LED stream for model view | `apps/control-panel/src/hooks/useColors.js` | `GET /get_colors` | JSON with `colors[]`, `step`, `totalPixels` | Compatible |
| Model topology | `apps/control-panel/src/hooks/useModelData.js` | `GET /get_model` | JSON with `schemaVersion` (`>=2` for external editing), intersections/connections/models/gaps arrays, and `capabilities.crossDevice` runtime fields (`enabled`, `transport`, `ready`, `runtimeState`, `peerCount`, `discoveryInProgress`, `droppedPackets`, `consecutiveFailures`, `lastError`) | Compatible |
| Intersection editing | `apps/control-panel/src/contexts/IntersectionContext.jsx` | `POST /add_intersection`, `POST /remove_intersection` | JSON request/response, clear error payloads (`group` is group bitmask) | Compatible |
| External port editing | `apps/control-panel/src/contexts/IntersectionContext.jsx`, `apps/control-panel/src/components/ModelInfo.jsx` | `POST /add_external_port`, `POST /update_external_port`, `POST /remove_external_port` | JSON body contract for external port CRUD, strict validation errors on bad slot/MAC/IDs | Compatible |
| Cross-device peer discovery | `apps/control-panel/src/contexts/IntersectionContext.jsx`, `apps/control-panel/src/components/ModelInfo.jsx` | `POST /cross_device/discover_peers`, `GET /cross_device/status`, `GET /cross_device/peers` | Discovery must be async/non-blocking; status and peers endpoints must remain available while WLED controls are active | Compatible |

## Known Gaps and Risks

1. `useDeviceInfo` now normalizes sparse payloads (including WLED-style `freeheap`, `fs.u`, `fs.t`) for most consumers, but firmware still does not expose dedicated keys for:
   - `activeLights`, `freeMemory`, `fps`, `storageUsed`, `storageTotal`
   - A normalized firmware payload would simplify UI logic.
2. `POST /update_wifi` reboots immediately; UI can see fetch/network errors even when operation succeeded.
3. `POST /toggle_visible` returns `302` redirect rather than JSON or plain `200`.
4. `GET /get_palettes` returns names only unless `v=true`; clients relying on full objects must always set `v=true`.

## Stability Rules (Recommended)

For endpoints used by React UI:

1. Do not rename paths without adding alias routes for at least one release cycle.
2. Preserve required response keys and basic types.
3. If new required fields are introduced, make them optional first.
4. Keep CORS behavior unchanged for browser compatibility.
5. Keep `update_palette` accepting form args until UI migration is complete.

## Change Checklist Before Merging Firmware API Changes

1. Verify all endpoints in this matrix still return expected status and shape.
2. Smoke-test these UI flows:
   - Layer list load and edits
   - Palette save/load/delete
   - Settings load/save
   - Model fetch and intersection add/remove
   - External port add/update/remove with two configured devices
3. If any contract change is unavoidable, update:
   - `/firmware-api/`
   - this file
   - React UI call sites in same change set
