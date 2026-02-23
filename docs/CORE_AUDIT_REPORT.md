# MeshLED Lightpath Integration Notes

Date: 2026-02-23

The full engine-level audit is now maintained in the Lightpath repository:
- Canonical report: [Lightpath Core Audit Report](https://github.com/kasparsj/lightpath/blob/main/docs/CORE_AUDIT_REPORT.md)
- Local submodule copy: `/Users/kasparsj/Work2/meshled/packages/lightpath/docs/CORE_AUDIT_REPORT.md`

## Scope (MeshLED only)

This document tracks only MeshLED integration contracts for Lightpath.

## Integration Contracts

- Core source is consumed from the git submodule at `/Users/kasparsj/Work2/meshled/packages/lightpath`.
- Firmware compiles core through symlink: `/Users/kasparsj/Work2/meshled/firmware/esp/src -> ../../packages/lightpath/src`.
- Desktop simulator lives at `/Users/kasparsj/Work2/meshled/apps/simulator` and includes core source from `../../packages/lightpath/src`.
- Simulator depends on an external openFrameworks checkout (`OF_ROOT`).

## CI Coverage in MeshLED

- Core host build and tests run against `packages/lightpath` via CMake.
- Firmware CI validates the `firmware/esp/src` symlink target and runs PlatformIO `compiledb` smoke.
- Simulator CI validates app layout under `apps/simulator` and runs optional `make -n` when `OF_ROOT` is provided.

## Local Verification Commands

```bash
cmake -S packages/lightpath -B packages/lightpath/build -DLIGHTPATH_CORE_BUILD_TESTS=ON
cmake --build packages/lightpath/build
ctest --test-dir packages/lightpath/build --output-on-failure

cd firmware/esp
pio run -e esp32dev -t compiledb

cd ../../apps/simulator
OF_ROOT=/path/to/openframeworks make -n
```

## Ownership

- Lightpath engine behavior, architecture, and deep code audit findings belong in the Lightpath repo.
- MeshLED-specific wiring, paths, CI contracts, and integration risks belong in this repo.
