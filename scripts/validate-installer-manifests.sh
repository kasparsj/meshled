#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MANIFEST_DIR="$ROOT_DIR/apps/installer/public/firmware"

shopt -s nullglob
manifest_paths=("$MANIFEST_DIR"/manifest-*.json)
shopt -u nullglob

if [ "${#manifest_paths[@]}" -eq 0 ]; then
  echo "No installer manifest files found in: $MANIFEST_DIR"
  exit 1
fi

python3 - "${manifest_paths[@]}" <<'PY'
import json
import re
import sys
from pathlib import Path

expected_chip_suffix = {
    "ESP32": "esp32",
    "ESP32-S3": "esp32s3",
}

tag_pattern = r"v[0-9]+\.[0-9]+\.[0-9]+(?:[-+][0-9A-Za-z.-]+)?"
asset_url_re = re.compile(
    rf"^https://github\.com/kasparsj/meshled/releases/download/({tag_pattern})/meshled-({tag_pattern})-(esp32|esp32s3)\.bin$"
)

errors = []

for manifest_path in [Path(arg) for arg in sys.argv[1:]]:
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        errors.append(f"{manifest_path}: invalid JSON ({exc})")
        continue

    builds = manifest.get("builds")
    if not isinstance(builds, list) or not builds:
        errors.append(f"{manifest_path}: 'builds' must be a non-empty array")
        continue

    seen_chip_families = set()

    for build_index, build in enumerate(builds):
        chip_family = build.get("chipFamily")
        label = f"{manifest_path}: builds[{build_index}]"

        if chip_family not in expected_chip_suffix:
            errors.append(
                f"{label}: unsupported chipFamily '{chip_family}' (expected one of: {', '.join(expected_chip_suffix)})"
            )
            continue

        seen_chip_families.add(chip_family)
        expected_suffix = expected_chip_suffix[chip_family]
        parts = build.get("parts")

        if not isinstance(parts, list) or not parts:
            errors.append(f"{label}: 'parts' must be a non-empty array")
            continue

        for part_index, part in enumerate(parts):
            part_label = f"{label}.parts[{part_index}]"
            path = part.get("path")

            if not isinstance(path, str):
                errors.append(f"{part_label}: path must be a string")
                continue

            match = asset_url_re.fullmatch(path)
            if not match:
                errors.append(
                    f"{part_label}: path must match "
                    "https://github.com/kasparsj/meshled/releases/download/vX.Y.Z/meshled-vX.Y.Z-(esp32|esp32s3).bin"
                )
                continue

            release_tag, file_tag, chip_suffix = match.groups()
            if release_tag != file_tag:
                errors.append(
                    f"{part_label}: release tag '{release_tag}' must match filename tag '{file_tag}'"
                )

            if chip_suffix != expected_suffix:
                errors.append(
                    f"{part_label}: chipFamily '{chip_family}' must use '-{expected_suffix}.bin' (found '-{chip_suffix}.bin')"
                )

            if part.get("offset") != 0:
                errors.append(f"{part_label}: offset must be 0 for merged installer firmware")

    missing_chip_families = sorted(set(expected_chip_suffix) - seen_chip_families)
    if missing_chip_families:
        errors.append(
            f"{manifest_path}: missing required chipFamily entries: {', '.join(missing_chip_families)}"
        )

if errors:
    print("Installer manifest validation failed:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print(f"Validated {len(sys.argv) - 1} installer manifest file(s).")
PY
