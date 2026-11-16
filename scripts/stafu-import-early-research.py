#!/usr/bin/env python3
"""Import the early STAFU curated evidence ZIP into the repository.

Usage:
  python scripts/stafu-import-early-research.py path/to/stafu-research-results-early-through-M325.zip

Review files before committing. This tool never pushes, overwrites existing
files, or extracts outside docs/stafu/research/results/.
"""
from pathlib import Path
from zipfile import ZipFile
import json
import sys

PREFIX = "docs/stafu/research/results/"
MANIFEST = PREFIX + "early-archive-manifest.json"
INDEX = PREFIX + "EARLY-INDEX.md"

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    root = Path(__file__).resolve().parent.parent
    archive = Path(sys.argv[1])
    if not archive.is_file():
        print("Archive not found:", archive)
        return 2
    with ZipFile(archive) as z:
        names = z.namelist()
        if len(names) != len(set(names)):
            print("Duplicate archive paths")
            return 2
        if MANIFEST not in names or INDEX not in names:
            print("Missing index or manifest")
            return 2
        manifest = json.loads(z.read(MANIFEST))
        entries = manifest.get("entries", [])
        expected = {INDEX, MANIFEST}
        for e in entries:
            p = e.get("evidence_path", "")
            if not p.startswith(PREFIX + "early/") or not p.endswith(".md"):
                print("Unexpected evidence path:", p)
                return 2
            expected.add(p)
        if len(entries) != 207 or len(expected) != 209 or set(names) != expected:
            print("Archive file count or paths mismatch")
            return 2
        for info in z.infolist():
            if info.is_dir() or info.file_size > 2_000_000:
                print("Unexpected directory or oversized file:", info.filename)
                return 2
            target = root / info.filename
            if target.exists():
                print("Refusing to overwrite:", target)
                return 2
        for name in sorted(names):
            target = root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(z.read(name))
    print(f"Imported {len(names)} curated research files into {PREFIX}")
    print("Review: git status --short")
    print("Then stage: git add docs/stafu/research/results")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
