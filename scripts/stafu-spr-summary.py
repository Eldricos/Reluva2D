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
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
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
    candidates = {}
    candidate_pattern = re.compile(r"^  candidate=(\S+) total=(\d+) valid=(\d+) exactArea=(\d+) exactDoubleArea=(\d+)$")
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            match = candidate_pattern.match(line)
            if not match:
                continue
            name, total, valid, area, double = match.groups()
            entry = candidates.setdefault(name, dict(total=0, valid=0, exactArea=0, exactDoubleArea=0))
            for key, value in zip(("total", "valid", "exactArea", "exactDoubleArea"),
                                  (total, valid, area, double)):
                entry[key] += int(value)
    if candidates:
        report = pathlib.Path(folder) / "candidates.json"
        report.write_text(json.dumps(candidates, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("Compression candidate report:", report)
    # Preserve per-resource groups: the same header tuple may occur in multiple files.
    header_groups = []
    header_pattern = re.compile(
        r"^  headerGroup=(\S+) count=(\d+) minBytes=(\d+) maxBytes=(\d+) "
        r"totalBytes=(\d+) samePrefix2=(\d+) samePrefix4=(\d+)$")
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            match = header_pattern.match(line)
            if match:
                key, count, minimum, maximum, total, prefix2, prefix4 = match.groups()
                header_groups.append({
                    "resource": path.name[:-4], "header": key,
                    "count": int(count), "minBytes": int(minimum),
                    "maxBytes": int(maximum), "totalBytes": int(total),
                    "samePrefix2": int(prefix2), "samePrefix4": int(prefix4)
                })
    if header_groups:
        report = pathlib.Path(folder) / "headers.json"
        report.write_text(json.dumps(header_groups, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("Repeated-header correlation report:", report)
    prefix_pattern = re.compile(
        r"^  prefixGroup=(\S+) count=(\d+) minCommon=(\d+) maxCommon=(\d+) "
        r"same8=(\d+) same16=(\d+) same32=(\d+) sameFirst2=(\d+) sameFirst4=(\d+) "
        r"lengthsDiv4=(\d+) lengthsDiv8=(\d+) lengthsDiv16=(\d+)$")
    prefix_groups = []
    fields = ("count", "minCommon", "maxCommon", "same8", "same16", "same32",
              "sameFirst2", "sameFirst4", "lengthsDiv4", "lengthsDiv8", "lengthsDiv16")
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            match = prefix_pattern.match(line)
            if match:
                key, *values = match.groups()
                entry = {"resource": path.name[:-4], "header": key}
                entry.update({name: int(value) for name, value in zip(fields, values)})
                prefix_groups.append(entry)
    if prefix_groups:
        report = pathlib.Path(folder) / "prefixes.json"
        report.write_text(json.dumps(prefix_groups, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("Prefix and block survey report:", report)
    pair_pattern = re.compile(
        r"^  framePair record=(\d+) from=(\d+) to=(\d+) fromTag=(\S+) toTag=(\S+) "
        r"sameHeader=(\d+) lenA=(\d+) lenB=(\d+) common=(\d+) prefix=(\d+) "
        r"suffix=(\d+) equalAtOffset=(\d+) emitBetween=(\d+) kramBetween=(\d+) otherBetween=(\d+)$")
    pairs = []
    names = ("record", "from", "to", "fromTag", "toTag", "sameHeader",
             "lenA", "lenB", "common", "prefix", "suffix", "equalAtOffset",
             "emitBetween", "kramBetween", "otherBetween")
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            match = pair_pattern.match(line)
            if match:
                entry = dict(zip(names, match.groups()))
                for name in names:
                    if name not in ("fromTag", "toTag"):
                        entry[name] = int(entry[name])
                entry["resource"] = path.name[:-4]
                pairs.append(entry)
    if pairs:
        report = pathlib.Path(folder) / "frame-pairs.json"
        report.write_text(json.dumps(pairs, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("Consecutive payload comparison report:", report)
    sequence_patterns = {
        "control": re.compile(r"^  control record=(\d+) chunk=(\d+) tag=(\S+) bytes=(\d+) value0=(\d+) value1=(\d+)$"),
        "image": re.compile(r"^  sequenceImage record=(\d+) chunk=(\d+) ordinal=(\d+) tag=(\S+) dataBytes=(\d+) headerA=(\d+) headerB=(\d+) marker=(\d+) precedingEmit=(-?\d+) precedingMark=(-?\d+) emitsSoFar=(\d+) marksSoFar=(\d+)$"),
        "summary": re.compile(r"^  sequenceSummary record=(\d+) images=(\d+) emits=(\d+) marks=(\d+) chunks=(\d+)$"),
    }
    sequence_fields = {
        "control": ("record", "chunk", "tag", "bytes", "value0", "value1"),
        "image": ("record", "chunk", "ordinal", "tag", "dataBytes", "headerA", "headerB", "marker", "precedingEmit", "precedingMark", "emitsSoFar", "marksSoFar"),
        "summary": ("record", "images", "emits", "marks", "chunks"),
    }
    sequence = []
    for path in sorted(pathlib.Path(folder).iterdir()):
        if not path.is_file() or not path.name.lower().endswith(".spr.txt"):
            continue
        for line in path.read_text(encoding="utf-8-sig").splitlines():
            for kind, pattern in sequence_patterns.items():
                match = pattern.match(line)
                if match:
                    fields = sequence_fields[kind]
                    item = {field: value if field == "tag" else int(value)
                            for field, value in zip(fields, match.groups())}
                    item.update(resource=path.name[:-4], kind=kind)
                    sequence.append(item)
                    break
    if sequence:
        report = pathlib.Path(folder) / "sequences.json"
        report.write_text(json.dumps(sequence, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("SPR sequence/control correlation report:", report)
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
