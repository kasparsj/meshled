# MeshLED Firmware HTTP API

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
- `wifi.ssid` is the active network SSID (`AP` SSID in AP mode, STA SSID in station mode).
- `wifi.mode` is `"ap"` or `"sta"`.

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

### `GET /get_colors[?maxColors=<int>][&layer=<id>]`

- Returns sampled LED stream for visualization.

### Intersection editing (`POST`)

- `/add_intersection` body JSON:
  - required: `numPorts` (`2` or `4`), `topPixel`, `group`
  - optional: `bottomPixel`
  - `group` must be a single valid group bit.
- `/remove_intersection` body JSON:
  - `{"id":<intersectionId>,"group":<groupIndex>}`

### Declarative topology schema (JSON)

#### `GET /export_topology`

- Exports normalized topology schema:
  - `schemaVersion`, `pixelCount`
  - `intersections[]`
  - `connections[]`
  - `models[]` (including routing strategy and conditional weights)
  - `ports[]` (port-id to intersection/slot mapping)
  - `gaps[]`

#### `POST /import_topology`

- Input JSON must match exported schema.
- Replaces current topology in-memory and rebuilds runtime `State`.
- Success response:

```json
{"success":true,"intersectionCount":12,"connectionCount":11}
```

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
