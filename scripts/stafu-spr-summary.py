#!/usr/bin/env python3
"""Summarize the read-only SPR byte-pattern logs; no game assets are accessed."""
import argparse
import json
import pathlib
import re

PATTERN = re.compile(
    r"^  record=(\d+) at=0x([0-9a-f]+) family=(\S+) n=(\d+) "
    r"distinct=(\d+) adjacentEqual=(\d+) runs3plus=(\d+) longest=(\d+) "
    r"zeros=(\d+) ff=(\d+) highBit=(\d+) pairCountZero=(\d+)/(\d+)$"
)

def summarize(folder):
    totals = {}
    for path in sorted(pathlib.Path(folder).glob("*.SPR.txt")):
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            match = PATTERN.match(line)
            if not match:
                continue
            _, _, family, n, distinct, adjacent, runs, longest, zero, ff, high, pairzero, pairs = match.groups()
            entry = totals.setdefault(family, dict(chunks=0, bytes=0, adjacentEqual=0, runs3plus=0,
                                                  maxRun=0, zero=0, ff=0, highBit=0,
                                                  pairCountZero=0, pairTokens=0))
            entry["chunks"] += 1
            entry["bytes"] += int(n)
            entry["adjacentEqual"] += int(adjacent)
            entry["runs3plus"] += int(runs)
            entry["maxRun"] = max(entry["maxRun"], int(longest))
            entry["zero"] += int(zero)
            entry["ff"] += int(ff)
            entry["highBit"] += int(high)
            entry["pairCountZero"] += int(pairzero)
            entry["pairTokens"] += int(pairs)
    return totals

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("folder", nargs="?", default="out/stafu-spr-report")
    parser.add_argument("--baseline", help="Optional prior JSON report to compare")
    args = parser.parse_args()
    report = summarize(args.folder)
    if not report:
        parser.error("No payload pattern lines found; check tool output and directory")
    output = pathlib.Path(args.folder) / "patterns.json"
    output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2, sort_keys=True))
    if args.baseline:
        old = json.loads(pathlib.Path(args.baseline).read_text(encoding="utf-8-sig"))
        print("Baseline comparison:", "UNCHANGED" if old == report else "CHANGED")
        if old != report:
            raise SystemExit(2)
