---
title: "MeshLED Firmware HTTP API"
---

Date: 2026-02-23  
Applies to: `firmware/esp` in this repository

## Base URL

- `http://<device-ip>`
- Default port: `80`

## Transport + auth

- CORS is enabled for API routes (`Access-Control-Allow-Origin: *`).
- Read routes are `GET`.
- Mutation routes are `POST`.
- Optional token auth for mutating routes:
  - Enable in settings: `api_auth_enabled=1`.
  - Set token: `api_auth_token=<token>`.
  - Send token as `Authorization: Bearer <token>` (preferred), `X-API-Token: <token>`, or `token` query arg.
- If auth is enabled and token is missing/invalid, route returns `401 {"error":"Unauthorized"}`.

## Common response shapes

### Layer (`GET /get_layers`)

```json
{
  "id": 0,
  "visible": true,
  "brightness": 255,
  "speed": 1.0,
  "fadeSpeed": 0,
  "easing": 0,
  "blendMode": 0,
  "behaviourFlags": 0,
  "offset": 0.0,
  "palette": {
    "colors": ["#FF0000", "#00FF00"],
    "positions": [0.0, 1.0],
    "colorRule": -1,
    "interMode": 1,
    "wrapMode": 0,
    "segmentation": 0.0
  }
}
```

### Colors (`GET /get_colors`)

```json
{
  "colors": [{"r":255,"g":0,"b":0,"w":0}],
  "step": 1,
  "totalPixels": 919
}
```

## Device + settings

### `GET /device_info`

- WLED-style info payload.
- Common keys: `wifi.ssid`, `wifi.mode`, `ip`, `leds.pwr`.
- MeshLED build metadata keys:
  - `meshledVersion`: release tag or git describe string from firmware build.
  - `meshledCommitSha`: git commit SHA when available from build metadata (or parsed from git-describe version string).
  - `meshledBuildSha`: short hash of the running firmware image.
    - Usually matches commit SHA in CI/PlatformIO builds.
    - In Arduino IDE builds without injected build macros, this falls back to the app image ELF SHA.
  - `meshledReleaseSha`: backward-compatible alias for `meshledBuildSha`.
- `wifi.ssid` is the active network SSID (`AP` SSID in AP mode, STA SSID in station mode).
- `wifi.mode` is `"ap"` or `"sta"`.
- Cross-device capability keys:
  - `crossDevice.enabled`: whether external transport is compiled/registered.
  - `crossDevice.transport`: active transport label (currently `esp-now` or `none`).
  - `crossDevice.ready`: transport runtime readiness.
  - `crossDevice.runtimeState`: `disabled` | `initializing` | `ready` | `degraded`.
  - `crossDevice.peerCount`: currently known peer count.
  - `crossDevice.discoveryInProgress`: async peer discovery state.
  - `crossDevice.droppedPackets`: RX queue drops.
  - `crossDevice.consecutiveFailures`: transport failure counter used for auto-degrade.
  - `crossDevice.lastError`: last transport error string.

### `GET /ota_status` (when OTA feature is compiled in)

- Returns OTA diagnostics JSON for confirming OTA apply/revert behavior.
- Includes:
  - current runtime identifiers (`meshledVersion`, `meshledCommitSha`, `meshledBuildSha`, `meshledReleaseSha`, `sketchMD5`)
  - reset reason (`resetReason`, `resetReasonCode`)
  - running partition metadata (`runningPartition`, `runningPartitionAddress`, `runningOtaState`) when available from ESP-IDF APIs
  - `lastOta` object persisted in SPIFFS (`/ota_status.json`) with stage transitions:
    - `start`
    - `end`
    - `error`
    - `interrupted`
    - `booted_new_firmware`
    - `reverted_or_not_applied`

### `GET /get_settings`

- Returns runtime settings JSON.
- Includes:
  - LED/object config (`pixelCount*`, `pixelPin*`, `ledType`, `colorOrder`, `ledLibrary`, `objectType`)
  - LED library capability metadata:
    - `availableLedLibraries`: array of available backend IDs (for example `[0,1]`)
    - `unavailableLedLibraryReasons`: object keyed by library ID string with short reason text (for example `{"0":"Disabled on ESP32-C3 to avoid RMT driver conflict"}`)
  - LED type capability metadata:
    - `availableLedTypes`: array of LED type IDs supported by at least one available backend
    - `ledTypeAvailableLibraries`: object keyed by LED type ID string with array of available backend IDs that support that type (for example `{"2":[1],"0":[0,1]}`)
    - `unavailableLedTypeReasons`: object keyed by LED type ID string with short reason text
  - network/runtime (`maxBrightness`, `deviceHostname`, WiFi saved credentials, `activeSSID`, `apMode`)
  - optional runtime toggles (OSC/OTA)
  - API auth config (`apiAuthEnabled`, `apiAuthToken`)

### `POST /update_settings`

- Input: form args (`multipart/form-data`).
- Notable args:
  - `max_brightness`, `hostname`
  - `pixel_count1`, `pixel_count2`, `pixel_pin1`, `pixel_pin2`, `pixel_density`
  - `led_type`, `color_order`, `led_library`, `object_type`
  - `osc_enabled`, `osc_port`
  - `ota_enabled`, `ota_port`, `ota_password`
  - `api_auth_enabled`, `api_auth_token`
- Success: `200 text/plain` with `OK`.

### `POST /update_wifi`

- Input: `ssid`, `password`.
- Saves credentials and restarts device.

### `POST /restart`

- Returns `200 text/plain` with `OK`, then restarts.

### `POST /update_brightness`

- Input: `value` (`1..255`).

## Layers

### `GET /get_layers`

- Returns editable layers.

### Mutation endpoints (all `POST`)

- `/toggle_visible` args: `layer`, `visible`
- `/update_layer_brightness` args: `layer`, `value`
- `/update_speed` args: `layer`, `value`
- `/update_ease` args: `layer`, `ease`
- `/update_fade_speed` args: `layer`, `value`
- `/update_behaviour_flags` args: `layer`, `flags`
- `/update_layer_offset` args: `layer`, `offset`
- `/update_blend_mode` args: `layer`, `mode`
- `/reset_layer` args: `layer`
- `/add_layer`
- `/remove_layer` args: `layer`

### Palette update for layer

#### `POST /update_palette`

- Required arg: `layer`
- Optional args:
  - `colorRule`, `interMode`, `wrapMode`, `segmentation`
  - `colors` (JSON array string of hex values)
  - `positions` (JSON array string of float positions)

### `GET /get_palette_colors`

- Args: `index=<N|uN>`, `layer=<id>`
- Returns layer-compatible palette payload.

## Palette library

### `GET /get_palettes[?v=true][&name=<paletteName>]`

- Names-only by default.
- `v=true` returns full palette objects.

### `POST /save_palette`

- Input: JSON palette object.

### `POST /delete_palette`

- Arg: `index=<userPaletteIndex>`.

### `POST /sync_palettes?push=true|false&pull=true|false`

- Input: JSON array of palette objects.

## Model + topology

### `GET /get_model`

- Returns model/editor payload used by control-panel.
- `intersections[].group` and `connections[].group` are emitted as group bitmasks.
- Includes `schemaVersion` (current value: `2`).
- Includes `capabilities.crossDevice` runtime diagnostics:
  - `enabled`, `transport`, `ready`, `runtimeState`
  - `peerCount`, `discoveryInProgress`
  - `droppedPackets`, `consecutiveFailures`, `lastError`
- `intersections[].ports[]` supports both:
  - internal ports: `{id,type:\"internal\",direction,group}`
  - external ports: `{id,type:\"external\",direction,group,device,targetId}`

### `GET /get_colors[?maxColors=<int>][&layer=<id>]`

- Returns sampled LED stream for visualization.

### Intersection editing (`POST`)

- `/add_intersection` body JSON:
  - required: `numPorts` (`2` or `4`), `topPixel`, `group`
  - optional: `bottomPixel`
  - `group` must be a single valid group bit.
- `/remove_intersection` body JSON:
  - `{"id":<intersectionId>,"group":<groupBitmask>}`
  - `group` expects a single valid group bit (legacy group index is accepted for backward compatibility).

### Declarative topology schema (JSON)

#### `GET /export_topology`

- Exports normalized topology schema (`schemaVersion: 2`):
  - `schemaVersion`, `pixelCount`
  - `intersections[]`
  - `connections[]`
  - `models[]` (including routing strategy and conditional weights)
  - `ports[]` with typed metadata:
    - `id`, `intersectionId`, `slotIndex`, `type`, `direction`, `group`
    - external-only: `deviceMac`, `targetPortId`
  - `gaps[]`

#### `POST /import_topology`

- Input JSON must match exported schema and include `schemaVersion: 2`.
- Schema versions other than `2` are rejected with `400`.
- Replaces current topology in-memory and rebuilds runtime `State`.
- Success response:

```json
{"success":true,"intersectionCount":12,"connectionCount":11}
```

### External ports (`POST`)

- `/add_external_port` body JSON:
  - required: `intersectionId`, `slotIndex`, `group`, `deviceMac`, `targetPortId`
  - optional: `direction` (default `false`)
- `/update_external_port` body JSON:
  - required: `portId`
  - optional mutable fields: `group`, `direction`, `deviceMac`, `targetPortId`
- `/remove_external_port` body JSON:
  - required: `portId`

### Cross-device runtime (`POST/GET`)

- `POST /cross_device/discover_peers`
  - Starts async peer discovery.
  - Does not block boot/runtime loops.
  - Success: `202` with current transport state snapshot.
- `GET /cross_device/peers`
  - Returns known peers (`mac`, `channel`, `encrypted`) plus `peerCount`.
- `GET /cross_device/status`
  - Returns runtime diagnostics:
    - `enabled`, `transport`, `runtimeState`, `ready`
    - `peerCount`, `discoveryInProgress`
    - `droppedPackets`, `consecutiveFailures`, `lastError`, `lastErrorAtMs`
- Build note:
  - Arduino IDE builds with the default ~1.2MB app partition can exceed flash size when full feature set is enabled.
  - Use a larger app partition scheme (for example "No OTA (Large APP)" / huge app) for coexistence builds.

## WLED compatibility routes

- `/json`, `/json/info`, `/json/state`, `/json/si`
- `/device_info`, `/on`, `/off`, `/version`, `/win`
- Note: when API auth is enabled, mutating WLED routes (`POST /json`, `/on`, `/off`) require token auth.

## Errors/status

- `200`: success
- `400`: invalid/missing params or invalid topology schema
- `401`: unauthorized mutation while API auth enabled
- `404`: missing model/resource
- `500`: internal failure
- `503`: transport unavailable / discovery start failed
