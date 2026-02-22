# Lightgraph Monorepo Reorg Report

Date: 2026-02-22  
Author: Codex

## A) Current State Inventory

### Top-level tree summary

```text
.
├── .github/                  # CI workflow
├── apps/
│   └── control-panel/        # React control panel (Vite)
├── docs/                     # build/API/contract docs
├── firmware/
│   └── esp/                  # ESP firmware (PlatformIO + Arduino)
├── packages/
│   ├── core/
│   │   └── src/              # shared C++ core model/engine
│   └── simulator/            # openFrameworks desktop simulator
├── scripts/                  # bootstrap/build helpers
├── tools/
│   └── esp-stacktrace-decoder/ # optional debug helper
└── vendor/
    └── ofxColorTheory/       # git submodule dependency
```

### Major directories and responsibilities

| Path | Purpose | Notes |
|---|---|---|
| `packages/core/src/` | Shared light engine/model C++ | Used by firmware and simulator |
| `packages/simulator/` | openFrameworks desktop simulator app | Consumes core model and OSC input |
| `firmware/esp/` | ESP32/ESP32-S3 firmware | PlatformIO + Arduino, HTTP/OSC/WLED-like API |
| `apps/control-panel/` | Browser control panel | React + Vite |
| `vendor/ofxColorTheory/` | Palette/color theory dependency | Git submodule (not vendored copy) |
| `docs/` | Build/API/compat/version docs | Includes migration report |
| `scripts/` | Root helper scripts | bootstrap + build workflows |
| `tools/esp-stacktrace-decoder/` | Optional crash dump decode helper | Now configurable and path-portable |

### Build/run entrypoints detected

- Firmware:
  - `firmware/esp/platformio.ini`
  - `pio run -e esp32dev`
  - `pio run -e esp32-s3-devkitc-1`
- React app:
  - `apps/control-panel/package.json`
  - `npm run dev`, `npm run lint`, `npm run build`
- Simulator:
  - `packages/simulator/Makefile`
  - `packages/simulator/config.make`
  - `packages/simulator/simulator.xcodeproj`
- CI:
  - `.github/workflows/ci.yml`

### Tooling/language assessment

- Core native code: C++ (openFrameworks-style integration via simulator; no standalone CMake found).
- Firmware: Arduino framework via PlatformIO; targets:
  - `esp32dev` (ESP32)
  - `esp32-s3-devkitc-1` (ESP32-S3)
- React app: Vite + React, npm lockfile present.

### Communication architecture (current)

- Control panel -> firmware: HTTP/JSON endpoints (`/get_layers`, `/update_settings`, `/device_info`, etc.).
- Performance/control inputs -> firmware + simulator: OSC over UDP (default port `54321`).
- Firmware inter-device transport: optional ESP-NOW, mDNS, SSDP.

### Coupling and risk points

- `firmware/esp/src` is a symlink to `../../packages/core/src`.
- Core palette/rule includes are path-coupled to `vendor/ofxColorTheory`.
- Simulator project metadata remains path-sensitive to openFrameworks location.

### Suspicious clutter / debt

- `packages/simulator/simulator.qbs` still assumes default `../../../../openframeworks` layout.
- Firmware config emits a PlatformIO warning for `build_src_dir` (left unchanged to avoid risky behavior changes in this phase).

## B) Proposed Target Structure

Target structure implemented:

```text
.
├── apps/
│   └── control-panel/
├── firmware/
│   └── esp/
├── packages/
│   ├── core/
│   │   └── src/
│   └── simulator/
├── vendor/
│   └── ofxColorTheory/
├── docs/
├── scripts/
├── tools/
│   └── esp-stacktrace-decoder/
├── README.md
└── LICENSE
```

### Naming conventions

- App surfaces under `apps/`.
- Device firmware under `firmware/`.
- Shared native code under `packages/core/`.
- openFrameworks host tooling under `packages/simulator/`.
- Third-party git dependency under `vendor/` as submodule.
- Utilities under `tools/`.

## C) Build & Run Plan

### Common prerequisites

- Git with submodule support.
- Python 3.11+ with PlatformIO.
- Node.js 20 LTS + npm.
- Local openFrameworks checkout for simulator builds.

### Bootstrap

```bash
git submodule update --init --recursive
./scripts/bootstrap.sh
```

### Control panel

```bash
cd apps/control-panel
npm ci --legacy-peer-deps
npm run lint
npm run build
npm run dev
```

### Firmware

```bash
cd firmware/esp
pio run -e esp32dev
pio run -e esp32-s3-devkitc-1
```

Or via helper script:

```bash
./scripts/build-firmware.sh esp32dev compiledb
```

### Simulator

```bash
cd packages/simulator
make -n
```

If needed:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

### Optional crash decode helper

```bash
./tools/esp-stacktrace-decoder/esp32decoder.sh <addr2line> <firmware.elf> <dump.txt>
```

## D) Phased Migration Plan

## Phase 1: minimal reshuffle + docs (Complete)

Exact moves/renames:

1. `react-app` -> `apps/control-panel`
2. `homo_deus` -> `firmware/esp`
3. Added root `README.md`
4. Updated docs/CI paths for moved app/firmware directories

Improvements:

- Clear top-level ownership for app and firmware.
- Root quickstart onboarding.

## Phase 2: tooling + scripts + structure (Complete)

Exact moves/renames:

1. `src` -> `packages/core/src`
2. `simulator` -> `packages/simulator`
3. `ofxColorTheory` submodule path -> `vendor/ofxColorTheory`
4. `firmware/esp/src` symlink -> `../../packages/core/src`
5. Updated core include paths + simulator path refs
6. Added root build scripts
7. Updated CI for new layout

Improvements:

- Explicit package boundaries.
- Cleaner, scriptable workflows.

## Phase 3: deeper cleanup (Complete)

Exact changes:

1. Removed IDE clutter from repo and added root `.gitignore` for local/editor/build artifacts.
2. Moved stacktrace decoder utility to tools namespace:
   - `EspStackTraceDecoder` -> `tools/esp-stacktrace-decoder`
3. Replaced hardcoded machine paths in decoder script with argument-driven, portable usage.
4. Normalized simulator path defaults to `openframeworks` naming in:
   - `packages/simulator/Makefile`
   - `packages/simulator/config.make`
   - `packages/simulator/simulator.qbs`
5. Added OSS readiness files:
   - `LICENSE`
   - `CONTRIBUTING.md`
   - `CODE_OF_CONDUCT.md`
   - `SECURITY.md`
   - `CHANGELOG.md`
6. Updated project metadata/docs (`README.md`, `docs/build.md`).

Improvements:

- Fewer machine-specific assumptions.
- Clearer contributor and governance baseline.
- Cleaner repository defaults for new contributors.

Risk notes:

- QBS/openFrameworks setups are still environment-sensitive for users with non-default OF layout.
- Full firmware build compatibility is still sensitive to ESP32 framework API differences (see follow-up log below).

Rollback strategy:

- Revert Phase 3 commit as one unit; no runtime protocol changes were made.

## E) Open Source Readiness Checklist

Status:

- `LICENSE`: Added.
- `README.md`: Added and updated.
- `CONTRIBUTING.md`: Added.
- `CODE_OF_CONDUCT.md`: Added.
- `SECURITY.md`: Added.
- `CHANGELOG.md`: Added.
- CI: Updated and layout-aligned.

Recommended next baseline items:

1. Add CI coverage for docs linting and basic markdown link checks (optional).
2. Resolve remaining full firmware compile blockers around mDNS/ESP-NOW API compatibility.

---

## Phase Progress Log

- Phase 0 (Audit + Plan): Complete.
- Phase 1 (Minimal reshuffle + docs): Complete.
- Phase 2 (Tooling + scripts + structure): Complete.
- Phase 3 (Deeper cleanup): Complete.

## Validation Log

- `./scripts/build-control-panel.sh` passes (existing lint warnings only).
- `./scripts/build-firmware.sh esp32dev compiledb` passes.
- Simulator scoped checks pass from `packages/simulator`.
- Full simulator build remains dependent on local openFrameworks checkout path.

## Follow-up Log (Post-Phase)

Date: 2026-02-22

Completed follow-ups:

1. Firmware config hardening:
   - Replaced ignored PlatformIO option with valid root source configuration:
     - `[platformio] src_dir = .`
2. Firmware dependency declarations:
   - Added `bblanchon/ArduinoJson @ ^6.21.5`
   - Added `dvarrel/ESPping @ ^1.0.5`

Current full-build status (`pio run -e esp32dev`):

- `ArduinoJson.h` missing-header issue: fixed.
- `ESPping.h` missing-header issue: fixed.
- Remaining blockers (pre-existing behavior now surfaced after dependency fixes):
  - `MDNSLib.h`: uses `MDNS.address(i)` which is not available in current framework API.
  - `ESPNowLib.h`: receive callback signature uses `esp_now_recv_info_t`, but framework expects legacy callback type (`esp_now_recv_cb_t` with MAC pointer signature).

Implication:

- Firmware smoke (`compiledb`) remains green.
- Full firmware compile is still not fully green and needs a dedicated compatibility patch across mDNS/ESP-NOW code paths.
