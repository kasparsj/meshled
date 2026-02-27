# MeshLED Installer Workflow

Date: 2026-02-27

## Overview

The installer web app lives at `apps/installer` and uses [ESP Web Tools](https://esphome.github.io/esp-web-tools/) for browser-based USB flashing over Web Serial.

The UX is intentionally similar to install.wled.me:

- channel selection (`stable` / `beta`)
- one-click install button
- erase-before-install option
- browser capability checks

## Local development

```bash
cd apps/installer
npm ci
npm run dev
```

## Verify in CI-equivalent mode

```bash
cd apps/installer
npm ci
npm run lint
npm run build
```

Validate manifest policy:

```bash
./scripts/validate-installer-manifests.sh
```

## Firmware manifest files

The installer reads two manifest files:

- `apps/installer/public/firmware/manifest-stable.json`
- `apps/installer/public/firmware/manifest-beta.json`

Each manifest should contain builds for:

- `ESP32`
- `ESP32-S3`

## Release asset naming convention

Expected merged binary names:

- `meshled-vX.Y.Z-esp32.bin`
- `meshled-vX.Y.Z-esp32s3.bin`

Host these under the matching GitHub release tag, for example:

- `https://github.com/kasparsj/meshled/releases/download/v0.1.0/meshled-v0.1.0-esp32.bin`
- `https://github.com/kasparsj/meshled/releases/download/v0.1.0/meshled-v0.1.0-esp32s3.bin`

## Generate merged binaries (manual/backfill)

Use this when you need a local or backfill build:

```bash
./scripts/export-installer-firmware.sh vX.Y.Z
```

The script:

1. Builds release environments for ESP32 and ESP32-S3.
2. Merges bootloader + partitions + boot_app0 + app firmware into a single flashable binary for each chip family.
3. Writes output files to `dist/installer-firmware/vX.Y.Z` by default.

## Automated release assets

Workflow: `.github/workflows/release-installer-firmware.yml`

Trigger modes:

- automatic on GitHub Release `published`
- manual via workflow dispatch with `tag` input

The workflow:

1. Builds release firmware for ESP32 and ESP32-S3.
2. Produces merged installer binaries via `scripts/export-installer-firmware.sh`.
3. Publishes release assets:
   - `meshled-vX.Y.Z-esp32.bin`
   - `meshled-vX.Y.Z-esp32s3.bin`
   - `SHA256SUMS.txt`

## Publishing steps

1. Ensure `manifest-stable.json` and/or `manifest-beta.json` points to expected release asset URLs for tag `vX.Y.Z`.
2. Create and publish the GitHub release for tag `vX.Y.Z` (the asset workflow runs automatically).
3. Verify release assets were attached by the workflow.
4. Update `manifest-stable.json` and/or `manifest-beta.json` when promoting channels:
   - set `version`
   - set `parts[].path` URLs to release assets
5. Rebuild and deploy the installer site.
