# MeshLED Installer Web App

Browser-based firmware installer for MeshLED, inspired by install.wled.me.

## Local Development

```bash
cd apps/installer
npm ci
npm run dev
```

## Production Build

```bash
npm run lint
npm run build
```

## Firmware Manifest Files

The installer currently reads:

- `public/firmware/manifest-stable.json`
- `public/firmware/manifest-beta.json`

Each manifest should point to release-hosted merged firmware binaries for:

- ESP32 (`chipFamily: ESP32`)
- ESP32-S3 (`chipFamily: ESP32-S3`)

Expected release asset naming:

- `meshled-vX.Y.Z-esp32.bin`
- `meshled-vX.Y.Z-esp32s3.bin`

Update both manifest files when creating a new release channel.
