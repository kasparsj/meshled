# MeshLED Versioning and Release Policy

Date: 2026-02-22

## Version scheme

`meshled` uses Semantic Versioning with `v` prefix:

- `vMAJOR.MINOR.PATCH`
- Examples: `v0.1.0`, `v0.2.3`, `v1.0.0`

## Stability rules

Before `v1.0.0`:

- `MINOR` may include breaking changes.
- `PATCH` should be non-breaking fixes/docs/chore updates.

At and after `v1.0.0`:

- `MAJOR`: breaking API/behavior changes.
- `MINOR`: backward-compatible feature additions.
- `PATCH`: backward-compatible fixes only.

## Compatibility scope

Compatibility is evaluated for:

1. Firmware HTTP contract used by React app:
   - see `docs/firmware-api.md`
   - see `docs/ui-firmware-compat.md`
2. OSC input contract consumed by SuperCollider/Tidal workflows:
   - canonical firmware contract: `docs/osc-contract.md`
   - artwork integration mapping: artwork repo `docs/osc-contract.md`
3. Build/dependency expectations:
   - see `docs/build.md`

## Release process

For each release:

1. Ensure CI is green (web + firmware smoke + simulator scoped checks).
2. Update contract docs if behavior changed.
3. Create an annotated tag:

```bash
git tag -a vX.Y.Z -m "meshled vX.Y.Z"
git push origin vX.Y.Z
```

4. Publish a GitHub release for tag `vX.Y.Z`.
5. Firmware release assets are auto-generated and uploaded by `.github/workflows/release-installer-firmware.yml`.
6. In artwork repo, bump `runtime/meshled` submodule in a separate commit.

## First stable runtime tag

The first installation-oriented runtime tag is:

- `v0.1.0`
