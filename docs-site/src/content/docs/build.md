---
title: "MeshLED Build and Verification"
---

Date: 2026-02-22

## Repository bootstrap

If this repo is used as a submodule, ensure nested submodules are initialized:

```bash
git submodule update --init --recursive
```

`ofxColorTheory` is required at `./packages/lightgraph/vendor/ofxColorTheory`.
`packages/lightgraph` itself is a submodule sourced from `git@github.com:kasparsj/lightgraph.git`.

Core-specific reproducibility notes live in `/core-build/`.
Core architecture notes live in `/core-architecture/`.
Third-party dependency licensing/provenance notes live in `/third-party-licenses/`.

Optional helper:

```bash
./scripts/bootstrap.sh
```

Optional crash-decoder helper:

```bash
./tools/esp-stacktrace-decoder/esp32decoder.sh <addr2line> <firmware.elf> <dump.txt>
```

## Firmware (`firmware/esp`)

Prerequisites:

- Python 3
- PlatformIO (`pip install platformio`)

Build:

```bash
cd firmware/esp
pio run -e esp32dev
```

Alternative target:

```bash
pio run -e esp32-s3-devkitc-1
```

## React control app (`apps/control-panel`)

Prerequisites:

- Node.js 20+ (18+ works, CI uses 20)

Install and verify:

```bash
cd apps/control-panel
npm ci --legacy-peer-deps
npm run lint
npm run build
```

Local dev server:

```bash
npm run dev
```

Cloudflare Pages deploy (control panel):

```bash
CLOUDFLARE_API_TOKEN=<token> \
CLOUDFLARE_ACCOUNT_ID=<account-id> \
./scripts/deploy-control-panel-cloudflare.sh --project-name meshled-ui --branch production
```

GitHub Pages test bundle (auto-published by `Docs (Pages)` workflow):

- JS: `https://<owner>.github.io/<repo>/control-panel-test/assets/app.js`
- CSS: `https://<owner>.github.io/<repo>/control-panel-test/assets/app.css`

For this repository, current URLs are:

- JS: `https://kasparsj.github.io/meshled/control-panel-test/assets/app.js`
- CSS: `https://kasparsj.github.io/meshled/control-panel-test/assets/app.css`
- Installer app: `https://kasparsj.github.io/meshled/installer/`

## Installer web app (`apps/installer`)

Prerequisites:

- Node.js 20+ (18+ works, CI uses 20)

Install and verify:

```bash
cd apps/installer
npm ci
npm run lint
npm run build
```

Local dev server:

```bash
npm run dev
```

GitHub Pages publish path:

- `https://<owner>.github.io/<repo>/installer/`

Cloudflare Pages deploy (installer):

```bash
CLOUDFLARE_API_TOKEN=<token> \
CLOUDFLARE_ACCOUNT_ID=<account-id> \
./scripts/deploy-installer-cloudflare.sh --project-name meshled-installer --branch production
```

## Simulator (`apps/simulator`)

Prerequisites:

- openFrameworks checkout

Default expectation from simulator directory:

- `OF_ROOT=../../../../openframeworks`

Dry-run make:

```bash
cd apps/simulator
make -n
```

If openFrameworks is elsewhere:

```bash
OF_ROOT=/path/to/openframeworks make -n
```

## Core host build (`packages/lightgraph`)

Prerequisites:

- CMake 3.20+
- C++17 compiler

Build and test:

```bash
cmake -S packages/lightgraph -B packages/lightgraph/build -DLIGHTGRAPH_CORE_BUILD_TESTS=ON
cmake --build packages/lightgraph/build
ctest --test-dir packages/lightgraph/build --output-on-failure
```

Script helper:

```bash
./scripts/build-core.sh default
./scripts/build-core.sh all
```

## CI coverage

GitHub Actions workflow: `.github/workflows/ci.yml`

Jobs:

1. `Core (host build + tests)`: CMake configure/build + `ctest` for the shared native engine.
2. `Core (ASan)`: host build/test with AddressSanitizer enabled.
3. `Core (UBSan)`: host build/test with UndefinedBehaviorSanitizer enabled.
4. `Core (Warnings)`: host build/test with strict warnings enabled (`LIGHTGRAPH_CORE_ENABLE_STRICT_WARNINGS=ON`) and warnings treated as errors.
5. `Web (Control Panel)`: `npm ci --legacy-peer-deps`, lint, build.
6. `Web (Installer)`: `npm ci`, lint, build.
7. `Installer (Manifest policy)`: validates installer manifests against release URL and naming policy.
8. `Firmware (PlatformIO smoke)`: generates `compile_commands.json` for `esp32dev` (`pio run -e esp32dev -t compiledb`) to validate dependency resolution and toolchain setup.
9. `Simulator (Scoped smoke)`: project/config integrity checks, plus optional `make -n` when `OF_ROOT` is provided in CI environment.

Note:

- Simulator CI is intentionally scoped because runner images do not include openFrameworks by default.

## Root helper scripts

```bash
./scripts/build-core.sh all
./scripts/build-control-panel.sh
./scripts/deploy-control-panel-cloudflare.sh --project-name meshled-ui --branch production
./scripts/build-installer.sh
./scripts/deploy-installer-cloudflare.sh --project-name meshled-installer --branch production
./scripts/validate-installer-manifests.sh
./scripts/build-firmware.sh esp32dev compiledb
```
