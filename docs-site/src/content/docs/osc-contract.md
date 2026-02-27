---
title: "MeshLED OSC Input Contract"
---

Date: 2026-02-23  
Status: Canonical contract for firmware OSC input behavior

## Scope

This document defines the OSC interface exposed by meshled firmware at `firmware/esp/OSCLib.h`.

Artwork/performance sender details (SuperCollider, TidalCycles, workshop scripts) should be documented in the artwork repository integration notes.

## Transport and endpoint

- Protocol: UDP OSC
- Firmware port: `54321` by default (`oscPort` setting)
- Port can be changed through firmware settings endpoints (`/get_settings`, `/update_settings`)

## Inbound OSC addresses

Firmware subscribes to:

- `/emit`
- `/note_on`
- `/note_off`
- `/notes_set`
- `/palette`
- `/color`
- `/split`
- `/auto`
- `/command`

Source: `firmware/esp/OSCLib.h`.

## Message contracts

## `/emit`

Format:

- Repeating pairs: `[paramId, value, paramId, value, ...]`

Behavior:

- Triggers a new emit using provided parameters.

## `/note_on`

Format:

- Same pair format as `/emit`.
- Commonly includes `NOTE_ID` for later stop.

Behavior:

- Emits with effectively infinite duration until stopped by `/note_off`.

## `/note_off`

Format:

- Optional single argument: `noteId`

Behavior:

- With `noteId`: stop that note.
- Without args: stop all notes.

## `/notes_set`

Format:

- Repeating triplets:
  - `[noteId, paramId, value, noteId, paramId, value, ...]`

Behavior:

- Batch updates/emits for multiple notes.

## `/palette`

Format:

- `[paletteIndex]`

Behavior:

- Sets current palette index.

## `/color`

Format:

- `[layerIndex]`
- or `[layerIndex, color]`
- or `[layerIndex, color1, color2, ...]`

Color format:

- 24-bit integer `0xRRGGBB`

Behavior:

- No color args: random color for the layer.
- One color: solid color.
- Multiple colors: gradient.

## `/split`

Format:

- Optional `[layerIndex]`

Behavior:

- With index: split one layer.
- Without args: split all layers.

## `/auto`

Format:

- No arguments required.

Behavior:

- Toggles auto emitter mode.

## `/command`

Format:

- Single string argument containing one or more command chars.

Behavior:

- Executes each char through firmware `doCommand`.
- Treat this as debug/legacy control.

## Emit parameter IDs

Pair-based endpoints (`/emit`, `/note_on`, `/notes_set`) use numeric parameter IDs:

| Name | ID |
|---|---|
| `MODEL` | `0` |
| `SPEED` | `1` |
| `EASE` | `2` |
| `FADE` | `3` |
| `FADE_THRESH` | `4` |
| `FADE_EASE` | `5` |
| `LENGTH` | `6` |
| `TRAIL` | `7` |
| `ORDER` | `8` |
| `HEAD` | `9` |
| `LINKED` | `10` |
| `FROM` | `11` |
| `DURATION_MS` | `12` |
| `DURATION_FRAMES` | `13` |
| `COLOR` | `14` |
| `COLOR_INDEX` | `15` |
| `NOTE_ID` | `16` |
| `MIN_BRI` | `17` |
| `MAX_BRI` | `18` |
| `BEHAVIOUR` | `19` |
| `EMIT_GROUPS` | `20` |
| `EMIT_OFFSET` | `21` |
| `COLOR_CHANGE_GROUPS` | `22` |

Source of truth: `packages/lightgraph/src/runtime/EmitParams.h`.

## Shared enums and flags

## Easing

- Valid `EASE` range: `0..33`
- Matches `packages/lightgraph/src/core/Types.h` (`EASE_NONE` to `EASE_ELASTIC_INOUT`)

## Behaviour flags (bitmask)

- `1` `POS_CHANGE_FADE`
- `2` `BRI_CONST_NOISE`
- `4` `RENDER_SEGMENT`
- `8` `ALLOW_BOUNCE`
- `16` `FORCE_BOUNCE`
- `32` `EXPIRE_IMMEDIATE`
- `64` `EMIT_FROM_CONN`
- `128` `FILL_EASE`
- `256` `RANDOM_COLOR`
- `512` `MIRROR_FLIP`
- `1024` `MIRROR_ROTATE`
- `2048` `SMOOTH_CHANGES`

Source: `packages/lightgraph/src/core/Types.h`.

## Compatibility and caveats

1. Command-char behavior can be compile-flag dependent (`DEBUGGER_ENABLED` paths).
2. Use `/auto` for reliable auto-emitter toggling instead of `/command`.
3. OSC traffic is unauthenticated and unencrypted by default.
