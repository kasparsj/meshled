# Lightgraph Monorepo Reorg Report

Date: 2026-02-22  
Author: Codex (Phase 0 audit + plan)

## A) Current State Inventory

### Top-level tree summary

```text
.
├── .github/                  # CI workflow
├── EspStackTraceDecoder/     # local ESP stacktrace decode helper
├── apps/
│   └── control-panel/        # React control panel (Vite)
├── docs/                     # API/build/version docs
├── firmware/
│   └── esp/                  # ESP firmware (PlatformIO + Arduino)
├── ofxColorTheory/           # git submodule (currently not initialized)
├── scripts/                  # bootstrap helpers
├── simulator/                # openFrameworks host simulator app
└── src/                      # shared core native logic (used by firmware/simulator)
```

### Major directories and responsibilities

| Path | Purpose | Notes |
|---|---|---|
| `src/` | Core light model/engine (`State`, `LightList`, geometry objects, palettes) | C++ shared by simulator and firmware-adjacent code |
| `simulator/` | openFrameworks desktop app | Receives OSC (`/emit`, `/note_on`, etc.) and renders model |
| `firmware/esp/` | ESP firmware | PlatformIO project (`platformio.ini`), main sketch `homo_deus.ino` |
| `apps/control-panel/` | Browser control panel | Vite + React 19 + Tailwind |
| `ofxColorTheory/` | Palette/color theory dependency | Git submodule, currently uninitialized locally (`git submodule status` shows leading `-`) |
| `docs/` | Contracts and build docs | `build.md`, firmware HTTP API, OSC contract, compatibility matrix |
| `EspStackTraceDecoder/` | Troubleshooting helper | Contains machine-specific hardcoded paths; not integrated in CI |

### Build/run entrypoints detected

- Firmware:
  - `firmware/esp/platformio.ini`
  - Build: `pio run -e esp32dev` (also `esp32-s3-devkitc-1`)
  - Flash: `pio run -e <env> -t upload`
- React app:
  - `apps/control-panel/package.json`
  - `npm run dev`, `npm run build`, `npm run lint`
- Simulator/core:
  - `simulator/Makefile`, `simulator/config.make`, `simulator/simulator.xcodeproj`
  - openFrameworks-based; CI runs scoped checks + optional `make -n`
- CI:
  - `.github/workflows/ci.yml` with jobs for web, firmware smoke, simulator scoped checks

### Tooling/language assessment

- Core native code: C++ (openFrameworks-style project integration in `simulator/`; no standalone CMake found).
- Firmware: Arduino framework via PlatformIO; targets:
  - `esp32dev` (ESP32)
  - `esp32-s3-devkitc-1` (ESP32-S3)
- React app: Vite + React (npm + `package-lock.json`).

### Communication architecture (current)

- Control panel -> firmware: HTTP/JSON endpoints on port 80 (`/get_layers`, `/update_settings`, `/device_info`, etc.).
- Performance/control inputs -> firmware + simulator: OSC over UDP, default port `54321`.
- Firmware peer-to-peer: optional ESP-NOW transport (`ESPNowLib.h`) + mDNS/SSDP discovery.
- Shared protocol docs:
  - `docs/firmware-api.md`
  - `docs/osc-contract.md`
  - `docs/ui-firmware-compat.md`

### Coupling and risk points

- `firmware/esp/src` is a symlink to `../../src` (critical for firmware include paths like `src/objects/...`).
- `src/*` includes `../ofxColorTheory/src/...` headers, so submodule location matters.
- Simulator project files reference `../src` and `../ofxColorTheory` paths.

### Suspicious clutter / debt

- `.idea/` committed IDE metadata.
- `EspStackTraceDecoder/esp32decoder.sh` contains absolute local paths (`/Users/...`).
- `simulator/simulator.qbs` contains absolute `/home/...` path values.
- `firmware/esp/platformio.ini` has comment `; NOT WORKING!!` and mixed legacy notes.

## B) Proposed Target Structure

Target structure (final-state intent, phased):

```text
.
├── apps/
│   └── control-panel/             # formerly react-app
├── firmware/
│   └── esp/                       # formerly homo_deus
├── packages/
│   ├── core/                      # formerly src (plus core-specific build metadata)
│   └── simulator/                 # formerly simulator
├── vendor/
│   └── ofxColorTheory/            # git submodule (not vendored source copy)
├── docs/
├── scripts/
├── tools/
│   └── esp-stacktrace-decoder/    # optional, cleaned helper
├── README.md
└── LICENSE
```

### Naming conventions

- `react-app` -> `apps/control-panel` (clear app role).
- `homo_deus` -> `firmware/esp` (hardware-oriented naming).
- `src` -> `packages/core` (shared engine package boundary).
- `simulator` -> `packages/simulator` (desktop app tied to core).
- Keep `ofxColorTheory` as a git submodule; move path only with coordinated include/path updates.

## C) Build & Run Plan

### Common prerequisites

- Git with submodule support.
- Python 3.11+ and PlatformIO (`pip install platformio`).
- Node.js 20 LTS + npm.
- openFrameworks checkout available locally for simulator builds.

### Bootstrap baseline

```bash
git submodule update --init --recursive
```

### Core/simulator (openFrameworks)

- Current build surface is the simulator app, which compiles against shared core C++.
- From simulator directory:

```bash
make -n
```

- If OF path differs:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

### Firmware build/flash

```bash
cd firmware/esp
pio run -e esp32dev
pio run -e esp32-s3-devkitc-1
```

Flash:

```bash
pio run -e esp32dev -t upload
```

### React control panel

```bash
cd apps/control-panel
npm ci --legacy-peer-deps
npm run lint
npm run build
npm run dev
```

## D) Phased Migration Plan

## Phase 1 (low risk): minimal reshuffle + docs

Goal: improve top-level clarity without touching path-sensitive core/simulator internals.

Planned file moves/renames:

1. `react-app` -> `apps/control-panel`
2. `homo_deus` -> `firmware/esp`
3. Update firmware internal symlink:
   - `firmware/esp/src` should point to repo root `src` (currently fragile after move if not corrected)
4. Add root `README.md` with quickstart and structure overview.
5. Update docs/CI references to new paths.

Expected user-facing improvements:

- Clearer monorepo domains (`apps`, `firmware`).
- New contributors can find entrypoints quickly from root README.
- Build instructions aligned with actual structure.

Risk notes:

- Firmware move can break includes if `src` symlink target is wrong.
- CI paths must be updated together with directory moves.
- Existing external scripts may still call old `homo_deus`/`react-app` paths.

Rollback strategy:

- `git revert <phase1-commit>` restores previous layout.
- For partial local rollback before commit: move directories back with `git mv` and restore symlink.

## Phase 2 (medium): tooling, scripts, CI hardening

Planned moves/changes:

1. `src` -> `packages/core/src` (or `packages/core` root with same contents).
2. `simulator` -> `packages/simulator`.
3. Move submodule path:
   - `ofxColorTheory` -> `vendor/ofxColorTheory` (remain submodule, update `.gitmodules` and includes).
4. Update include paths:
   - Core includes to `vendor/ofxColorTheory`.
   - Firmware symlink target to `packages/core/src`.
   - Simulator project refs (`../src`, `../ofxColorTheory`) to new paths.
5. Add root helper scripts:
   - `scripts/bootstrap.sh`
   - `scripts/build-firmware.sh`
   - `scripts/build-control-panel.sh`
6. Expand CI to include path updates and basic bootstrap validation.

Expected improvements:

- Strong package boundaries for core/simulator/vendor.
- Lower onboarding friction with consistent scripts.
- Better CI confidence for refactors.

Risk notes:

- Path rewrites in Xcode/OpenFrameworks metadata are error-prone.
- Submodule path migration can break clones if docs/scripts lag.

Rollback:

- Keep moves in isolated commits (core move, simulator move, submodule path move) for selective revert.

## Phase 3 (optional): deeper cleanup

Planned changes:

1. Quarantine obsolete utilities to `legacy/` or `tools/` with README notes.
2. Normalize simulator QBS/Xcode generated artifacts and remove machine-specific paths.
3. Introduce stricter lint/style checks for firmware/core where practical.
4. API cleanup roadmap (convert mutating GET endpoints to POST with compatibility shims).

Expected improvements:

- Cleaner OSS presentation.
- Reduced environment-specific surprises.

Risk notes:

- Behavior regressions if API cleanup is done without alias routes/tests.

Rollback:

- Keep compatibility aliases for one release cycle; revert optional cleanup commits independently.

## E) Open Source Readiness Checklist

Current status:

- `README.md`: Present at repo root (added in Phase 1).
- `LICENSE`: Missing.
- `CONTRIBUTING.md`: Missing.
- `CODE_OF_CONDUCT.md`: Missing.
- `SECURITY.md`: Missing.
- `CHANGELOG.md`: Missing.
- CI: Present and useful; currently smoke-level for firmware/simulator.

Suggested minimal OSS baseline (without overengineering):

1. Add root `README.md` and `LICENSE` first.
2. Add contributor policy docs (`CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, `SECURITY.md`).
3. Keep CI focused:
   - control panel lint/build
   - firmware PlatformIO compile smoke
   - simulator project integrity checks
4. Add release notes process (`CHANGELOG.md` + tag policy already documented in `docs/versioning.md`).

---

## Phase Progress Log

- Phase 0 (Audit + Plan): Complete.
- Phase 1 (Minimal reshuffle + docs): Complete.
- Phase 2 (Tooling + CI hardening): Pending.
- Phase 3 (Optional deeper cleanup): Pending.

## Phase 1 Execution Update

### What changed

- Moved `react-app` -> `apps/control-panel` via `git mv`.
- Moved `homo_deus` -> `firmware/esp` via `git mv`.
- Fixed firmware shared-core symlink:
  - `firmware/esp/src` now points to `../../src`.
- Updated CI and docs to new Phase 1 paths.
- Added root onboarding files:
  - `README.md`
  - `scripts/bootstrap.sh`
- Validation:
  - `npm run lint && npm run build` succeeds in `apps/control-panel` (warnings only).
  - `pio run -e esp32dev -t compiledb` succeeds in `firmware/esp`.
  - `simulator/make -n` is still environment-dependent on local openFrameworks checkout.

### Moved paths

- `react-app` -> `apps/control-panel`
- `homo_deus` -> `firmware/esp`

### What remains for Phase 2

1. Move core and simulator into `packages/`:
   - `src` -> `packages/core`
   - `simulator` -> `packages/simulator`
2. Move submodule path while keeping submodule semantics:
   - `ofxColorTheory` -> `vendor/ofxColorTheory`
3. Update include/project paths across firmware and simulator after those moves.
4. Add stronger root scripts for build/test workflows and expand CI checks accordingly.
