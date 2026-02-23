# Third-Party Dependencies and License Notes

Date: 2026-02-23

This document tracks third-party code used by MeshLED and where license text/provenance is confirmed or still pending.

## Confirmed Dependencies

| Dependency | Location | Usage | License status |
|---|---|---|---|
| ofxColorTheory | `packages/lightpath/vendor/ofxColorTheory` (git submodule) | color-rule generation in core palette/light code | Confirmed MIT (`packages/lightpath/vendor/ofxColorTheory/LICENSE`) |
| FastNoise | `packages/lightpath/src/FastNoise.h`, `packages/lightpath/src/FastNoise.cpp` | procedural noise behavior | Confirmed MIT header in source file |

## Dependencies Requiring Follow-up

| Dependency/material | Location | Current status | Follow-up |
|---|---|---|---|
| ofxEasing | `packages/lightpath/src/ofxEasing.h` | No explicit license header in vendored file | Confirm upstream source + license and add attribution/copy to repo docs |
| FastLED/cpt-city derived palette data | `packages/lightpath/src/Palettes.h`, `packages/lightpath/src/Palettes.cpp` | Source comment mentions derivation, but attribution text is incomplete | Add explicit attribution + license compatibility note for reused palette data |

## Operational Notes

- Keep `packages/lightpath/vendor/ofxColorTheory` as a submodule; do not vendor inline copies elsewhere.
- When adding a new third-party dependency, include:
- source URL/repository
- version or commit reference
- license type and where full text is stored in this repo
- If a dependency has unclear licensing, treat it as blocked for redistribution until clarified.
