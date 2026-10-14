#!/usr/bin/env python3
"""Package and download release third-party installs without rebuilding them."""

from __future__ import annotations

import argparse
import json
import platform
import re
import zipfile
from pathlib import Path



ROOT = Path(__file__).resolve().parents[1]
PLATFORMS = ("win32", "linux", "macosx", "android", "ios")


def pin() -> str:
    import subprocess
    return subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, encoding="utf-8").strip()


def package_name(target: str, configuration: str, tag: str, architecture: str) -> str:
    if target not in PLATFORMS or configuration not in ("Debug", "Release"):
        raise ValueError("Unsupported platform/configuration")
    if configuration == "Debug" and target != "win32":
        raise ValueError("Non-Windows dependencies are built as Release; use the Release package")
    if not re.fullmatch(r"[A-Za-z0-9_.-]+", tag):
        raise ValueError("Invalid release tag")
    return f"eve-third-party-{target}-{architecture}-{configuration.lower()}-{tag}.zip"


def validate_stamp(text: str, target: str, configuration: str) -> None:
    fields = dict(line.split("=", 1) for line in text.splitlines() if "=" in line)
    for key, expected in (("aggregate", pin()), ("platform", target), ("build_type", configuration)):
        if fields.get(key) != expected:
            raise ValueError(f"Third-party stamp {key}: {fields.get(key)!r}, expected {expected!r}")


def pack(prefix: Path, target: str, configuration: str, tag: str, output: Path, architecture: str) -> Path:
    validate_stamp((prefix / "third-party-version.txt").read_text(encoding="utf-8"), target, configuration)
    if not (prefix / "include").is_dir() or not (prefix / "lib").is_dir():
        raise ValueError("Third-party install must contain include/ and lib/")
    manifest = {
        "schema": "evengine.third-party-package", "version": 1,
        "platform": target, "configuration": configuration, "aggregate": pin(),
        "release": tag, "architecture": architecture, "host": platform.platform(),
    }
    output.mkdir(parents=True, exist_ok=True)
    archive = output / package_name(target, configuration, tag, architecture)
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as bundle:
        for path in sorted(prefix.rglob("*")):
            if path.is_file():
                # Dereference installed library symlinks so Windows can extract too.
                bundle.write(path, path.relative_to(prefix).as_posix())
        for path in sorted(ROOT.rglob("*")):
            if path.is_file() and path.name.lower().startswith(("license", "copying", "copyright")):
                bundle.write(path, "licenses/" + path.relative_to(ROOT).as_posix())
        bundle.writestr("eve-third-party-package.json", json.dumps(manifest, indent=2) + "\n")
    print(f"Packaged {archive}")
    return archive


def main() -> None:
    parser = argparse.ArgumentParser(description="Package a third-party release install")
    parser.add_argument("platform", choices=PLATFORMS)
    parser.add_argument("tag")
    parser.add_argument("--configuration", default="Release", choices=("Debug", "Release"))
    parser.add_argument("--architecture", required=True, choices=("x86_64", "arm64"))
    parser.add_argument("--prefix", type=Path, default=Path("build/install"))
    parser.add_argument("--output", type=Path, default=Path("artifacts"))
    args = parser.parse_args()
    pack(args.prefix, args.platform, args.configuration, args.tag, args.output, args.architecture)


if __name__ == "__main__":
    import sys
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8")
    main()
