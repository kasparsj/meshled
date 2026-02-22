# Lightgraph Firmware HTTP API (Draft v0)

Date: 2026-02-22  
Applies to: `homo_deus` firmware in `lightgraph` (`main`)

## Scope

This document defines the HTTP contract currently used by the `react-app` control panel and model editor.

Base URL:

- `http://<device-ip>`
- Default port: `80`

Transport and behavior:

- No authentication.
- CORS headers are enabled (`Access-Control-Allow-Origin: *`) on API routes.
- Many mutation endpoints are implemented as `GET` for historical reasons.
- Response bodies are a mix of JSON and plain text.

## Data Shapes

### Layer object (`GET /get_layers`)

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

### Palette object (user palette)

```json
{
  "name": "My Palette",
  "colors": ["#FF0000", "#00FF00"],
  "positions": [0.0, 1.0],
  "colorRule": -1,
  "interMode": 1,
  "wrapMode": 0,
  "segmentation": 0.0
}
```

### Colors response (`GET /get_colors`)

```json
{
  "colors": [{"r":255,"g":0,"b":0,"w":0}],
  "step": 1,
  "totalPixels": 919
}
```

## Endpoint Contract

## Device and settings

### `GET /device_info`

- Alias of WLED info endpoint.
- Returns WLED-like JSON object.
- Common keys used by UI:
  - `wifi.ssid`
  - `ip`
  - `leds.pwr`

### `GET /get_settings`

- Returns current runtime settings as JSON.
- Keys include:
  - `maxBrightness`, `deviceHostname`
  - `pixelCount1`, `pixelCount2`, `pixelPin1`, `pixelPin2`
  - `pixelDensity`, `ledType`, `colorOrder`, `ledLibrary`, `objectType`
  - `savedSSID`, `savedPassword`
  - optional (if compiled): `oscEnabled`, `oscPort`, `otaEnabled`, `otaPort`, `otaPassword`

### `POST /update_settings`

- Input: form arguments (`multipart/form-data` from UI).
- Supported args:
  - `hostname`
  - `osc_enabled`, `osc_port`
  - `led_type`, `color_order`, `pixel_count1`, `pixel_count2`, `pixel_pin1`, `pixel_pin2`, `pixel_density`, `led_library`, `object_type`
  - `ota_enabled`, `ota_port`, `ota_password`
- Success: `200 text/plain` with `OK`.

### `POST /update_wifi`

- Input: form args `ssid`, `password`.
- On success, credentials are stored and device restarts.
- Important: restart may close connection before a normal response is observed by client.

### `GET /restart`

- Returns `200 text/plain` with `OK`, then restarts device.

## Layers

### `GET /get_layers`

- Returns array of editable layers.
- Note: non-editable/internal layers are omitted.

### `GET /toggle_visible?layer=<id>&visible=<true|false>`

- Toggles layer visibility.
- Success behavior: HTTP redirect (`302`) to `/` (historical web UI behavior).

### `GET /update_layer_brightness?layer=<id>&value=<1..255>`

- Success: `200 text/plain`.
- Errors: `400` for missing/invalid params.

### `GET /update_speed?layer=<id>&value=<-10.0..10.0>`

- Success: `200 text/plain`.

### `GET /update_ease?layer=<id>&ease=<0..33>`

- Easing enum range from `EASE_NONE` to `EASE_ELASTIC_INOUT`.

### `GET /update_behaviour_flags?layer=<id>&flags=<uint16>`

- Sets `Behaviour.flags` bitmask for a layer.

### `GET /update_layer_offset?layer=<id>&offset=<float>`

- Sets layer offset.

### `GET /reset_layer?layer=<id>`

- Resets layer to defaults.

### `POST /add_layer`

- Adds first available editable layer slot.
- Success: `200 text/plain`.

### `POST /remove_layer?layer=<id>`

- Removes editable layer.
- Layer `0` cannot be removed.

### `HTTP_ANY /update_palette`

- Required arg: `layer`.
- Optional args:
  - `colorRule` (`-1..7`)
  - `interMode` (`-1..2`)
  - `wrapMode` (`-1..3`)
  - `segmentation` (`>=0`)
  - `colors` (JSON array of hex strings, example: `["#FF0000","#00FF00"]`)
  - `positions` (JSON array of floats, `0..1`)
- UI uses `POST` with `FormData`.
- Success: `200 text/plain` with `OK`.

### `GET /get_palette_colors?index=<N|uN>&layer=<id>`

- Returns palette details as JSON:
  - `colors`, `positions`, `colorRule`, `interMode`, `wrapMode`, `segmentation`
- `index=N`: built-in palette index.
- `index=uN`: user palette index.

## Palette library

### `GET /get_palettes[?v=true][&name=<paletteName>]`

- Default response (no `v`): names only.
- With `v=true`: full palette objects.
- With `name=<...>`: returns specific palette (full details).

### `POST /save_palette`

- Input: JSON palette object (must include `name` and at least one color).
- Success: `{"success":true,"message":"Palette saved successfully"}`.

### `GET /delete_palette?index=<userPaletteIndex>`

- Deletes user palette by index.
- Success: JSON with `success` and `message`.

### `POST /sync_palettes?push=true|false&pull=true|false`

- Input: JSON array of palettes.
- `push=true`: add unknown client palettes on device.
- `pull=true`: return device palettes missing in client payload.

## Model and visualization

### `GET /get_colors[?maxColors=<int>][&layer=<id>]`

- Returns sampled LED color stream:
  - `colors` array of `{r,g,b,w}`
  - `step`
  - `totalPixels`
- Optional `layer` isolates one layer during sampling.

### `GET /get_model`

- Returns model topology as JSON:
  - `pixelCount`, `realPixelCount`, `modelCount`, `gapCount`
  - `intersections[]`, `connections[]`, `models[]`, `gaps[]`

### `POST /add_intersection`

- Input JSON:
  - required: `numPorts` (`2` or `4`), `topPixel`, `group`
  - optional: `bottomPixel`
- Success: `{"success":true,"id":<newId>}`.

### `POST /remove_intersection`

- Input JSON: `{"id":<intersectionId>,"group":<group>}`.
- Success: `{"success":true}`.

## WLED compatibility endpoints

- `GET|POST /json`
- `GET /json/info`
- `GET /device_info` (same handler as `/json/info`)
- `GET|POST /json/state`
- `GET /json/si`
- `GET /on`
- `GET /off`
- `GET /version`
- `GET /win`

## Errors and status codes

General patterns:

- `200`: success.
- `302`: redirect (notably `/toggle_visible`).
- `400`: invalid or missing params.
- `404`: missing resource/model/layer.
- `500`: internal failure (for example failed palette persistence).

Error body format is not uniform:

- Some routes return plain text.
- Some routes return JSON with `{"error":"..."}`.

## Known contract quirks

- Mutation-by-GET exists on several endpoints (`/update_*`, `/toggle_visible`, `/reset_layer`).
- `/update_wifi` may reboot before the client reads a response body.
- `/get_layers` includes only editable layers.
- `/update_palette` is registered as `HTTP_ANY`, but the React UI uses `POST` form args.

