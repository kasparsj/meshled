# Contributing to MeshLED

## Prerequisites

- Git with submodule support
- Node.js 20+
- Python 3.11+
- PlatformIO
- openFrameworks checkout (for simulator work)

## Setup

```bash
git submodule update --init --recursive
./scripts/bootstrap.sh
```

## Validate Changes

Run the checks for the area you touched:

```bash
./scripts/build-control-panel.sh
./scripts/build-firmware.sh esp32dev compiledb
cd packages/simulator && make -n
```

If `openframeworks` is not at the default path, override `OF_ROOT`:

```bash
cd packages/simulator
OF_ROOT=/path/to/openframeworks make -n
```

## Pull Requests

1. Keep changes scoped and history-friendly.
2. Update docs when behavior or paths change.
3. Keep CI green.
4. Include a short risk note for path/toolchain changes.

## Commit Guidelines

- Prefer clear, imperative commit messages.
- Keep refactors and behavior changes separate when possible.

## Code Style

- Match existing style in each package.
- Favor small, safe incremental changes over broad rewrites.
