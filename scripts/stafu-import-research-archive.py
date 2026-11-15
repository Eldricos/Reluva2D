#!/usr/bin/env python3
"""Import the curated M326-M447 research ZIP into this repository.

Usage:
  python scripts/stafu-import-research-archive.py path/to/stafu-research-results-M326-M447.zip

Only files below docs/stafu/research/results/ are accepted. This script does not
commit or push; review the imported files before staging them.
"""
from pathlib import Path
from zipfile import ZipFile
import sys

PREFIX = "docs/stafu/research/results/"
EXPECTED = {f"{PREFIX}m{i}-evidence.md" for i in range(326, 448)}
EXPECTED |= {PREFIX + "INDEX.md", PREFIX + "archive-manifest.json"}

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    root = Path(__file__).resolve().parent.parent
    archive = Path(sys.argv[1]).resolve()
    if not archive.is_file():
        print(f"Archive not found: {archive}")
        return 2
    with ZipFile(archive) as z:
        names = set(z.namelist())
        if names != EXPECTED or len(z.namelist()) != len(EXPECTED):
            print(f"Unexpected archive contents: missing={sorted(EXPECTED - names)} extra={sorted(names - EXPECTED)}")
            return 2
        for info in z.infolist():
            if info.is_dir() or info.file_size > 2_000_000:
                print(f"Unexpected directory or oversized file: {info.filename}")
                return 2
        for name in sorted(names):
            dest = root / name
            if dest.exists() and name != PREFIX + "m447-evidence.md":
                print(f"Refusing to overwrite existing file: {dest}")
                return 2
        for name in sorted(names):
            dest = root / name
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(z.read(name))
    print(f"Imported {len(names)} research files under {PREFIX}")
    print("Review with: git status --short && git diff --no-index /dev/null docs/stafu/research/results/INDEX.md")
    print("Then: git add docs/stafu/research/results")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
