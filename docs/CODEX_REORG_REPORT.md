# Lightgraph Monorepo Reorg Report

Date: 2026-02-22  
Author: Codex

## A) Current State Inventory

### Top-level tree summary

```text
.
├── .github/                  # CI workflow
├── EspStackTraceDecoder/     # local ESP stacktrace decode helper
├── apps/
│   └── control-panel/        # React control panel (Vite)
├── docs/                     # build/API/contract docs
├── firmware/
│   └── esp/                  # ESP firmware (PlatformIO + Arduino)
├── packages/
│   ├── core/
│   │   └── src/              # shared C++ core model/engine
│   └── simulator/            # openFrameworks desktop simulator
├── scripts/                  # repo automation/build helpers
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
| `scripts/` | Bootstrap/build helper scripts | Root-level workflows |

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
- Simulator project metadata is path-sensitive to core/vendor locations.

### Suspicious clutter / debt

- `.idea/` committed IDE metadata.
- `EspStackTraceDecoder/esp32decoder.sh` has absolute local machine paths.
- `packages/simulator/simulator.qbs` includes machine-specific absolute openFrameworks paths.
- `firmware/esp/platformio.ini` contains stale warning comment (`; NOT WORKING!!`).

## B) Proposed Target Structure

Target structure (now implemented for core areas):

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
└── README.md
```

### Naming conventions

- App surfaces under `apps/`.
- Device firmware under `firmware/`.
- Shared native code under `packages/core/`.
- openFrameworks host tooling under `packages/simulator/`.
- Third-party git dependency under `vendor/` as submodule.

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

## D) Phased Migration Plan

## Phase 1: minimal reshuffle + docs (Complete)

Exact moves/renames:

1. `react-app` -> `apps/control-panel`
2. `homo_deus` -> `firmware/esp`
3. firmware symlink update:
   - `firmware/esp/src` -> `../../src` (Phase 1 state)
4. Added root `README.md`
5. Updated docs/CI paths for moved app/firmware directories

User-facing improvements:

- Clearer top-level ownership for app vs firmware.
- Root quickstart and consistent module entrypoints.

Risk notes:

- Firmware include coupling via `src` symlink.
- External scripts using old paths might break.

Rollback:

- Revert Phase 1 commit as a single unit.

## Phase 2: tooling + scripts + CI + deeper structure (Complete)

Exact moves/renames:

1. `src` -> `packages/core/src`
2. `simulator` -> `packages/simulator`
3. `ofxColorTheory` submodule path -> `vendor/ofxColorTheory`
4. firmware symlink update:
   - `firmware/esp/src` -> `../../packages/core/src`
5. Updated simulator project path references:
   - core group `../core/src`
   - vendor group `../../vendor/ofxColorTheory`
6. Updated core include references to vendor path.
7. Added root scripts:
   - `scripts/build-control-panel.sh`
   - `scripts/build-firmware.sh`
8. Expanded CI checks:
   - firmware symlink validation
   - simulator job path moved to `packages/simulator`

User-facing improvements:

- Package boundaries now explicit (`packages/core`, `packages/simulator`).
- Vendor dependency location standardized.
- Repeatable root build scripts for app/firmware checks.

Risk notes:

- Simulator still depends on local openFrameworks and path-sensitive project metadata.
- QBS file remains machine-path dependent and should be cleaned in Phase 3.

Rollback:

- Revert Phase 2 commit as a single unit.
- If needed, selectively revert path-coupled project file edits first.

## Phase 3: optional deeper cleanup (Pending)

Planned changes:

1. Normalize/remove machine-specific paths (`simulator.qbs`, stacktrace helper script).
2. Decide on `.idea/` handling (remove or document).
3. Add OSS policy files (`LICENSE`, `CONTRIBUTING`, `CODE_OF_CONDUCT`, `SECURITY`, `CHANGELOG`).
4. Optional API hygiene roadmap (legacy mutating `GET` endpoints with compatibility aliases).

## E) Open Source Readiness Checklist

Current status:

- `README.md`: Present.
- `LICENSE`: Missing.
- `CONTRIBUTING.md`: Missing.
- `CODE_OF_CONDUCT.md`: Missing.
- `SECURITY.md`: Missing.
- `CHANGELOG.md`: Missing.
- CI: Present and updated for new monorepo layout.

Minimal suggested next baseline:

1. Add `LICENSE`.
2. Add contributor/security governance docs.
3. Keep CI scope practical (web lint/build, firmware compile smoke, simulator scoped checks).

---

## Phase Progress Log

- Phase 0 (Audit + Plan): Complete.
- Phase 1 (Minimal reshuffle + docs): Complete.
- Phase 2 (Tooling + scripts + structure): Complete.
- Phase 3 (Optional deeper cleanup): Pending.

## Validation Log

- `npm run lint && npm run build` passes in `apps/control-panel` (warnings only).
- `pio run -e esp32dev -t compiledb` passes in `firmware/esp`.
- Simulator scoped checks run from `packages/simulator`; full build remains environment-dependent on local openFrameworks setup.
