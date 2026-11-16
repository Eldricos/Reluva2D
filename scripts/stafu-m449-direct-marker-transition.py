#!/usr/bin/env python3
"""M449: compare direct-image marker transitions in the M325 SPR inventory.

Usage:
  python scripts/stafu-m449-direct-marker-transition.py m325-record-structure.txt > m449-direct-marker-transition.txt

Reads only the original M325 text report. Tests marker/dimension and
marker/payload associations; neither test proves a decoder or codec mode.
"""
from __future__ import annotations
from collections import Counter, defaultdict
from pathlib import Path
import sys

def read(path):
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe", b"\xfe\xff")):
        return raw.decode("utf-16")
    if raw[:256].count(b"\0") > 32:
        return raw.decode("utf-16-le")
    return raw.decode("utf-8-sig")

def parse(text):
    result = defaultdict(lambda: defaultdict(list))
    for line in text.splitlines():
        if not line.startswith("SPR_M325_"):
            continue
        head, *tokens = line.split()
        fields = dict(token.split("=", 1) for token in tokens if "=" in token)
        if "resource" in fields:
            result[fields["resource"]][head.removeprefix("SPR_M325_")].append(fields)
    return result

def num(row, key):
    return int(row[key], 0)

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    path = Path(sys.argv[1])
    if not path.is_file():
        print(f"M449_ERROR missing_input={path}")
        return 2
    groups = parse(read(path))
    if "BRDGTROI.SPR" not in groups or len(groups) < 5:
        print(f"M449_ERROR expected_BRDGTROI_and_five_resources resources={len(groups)}")
        return 2
    rows = []
    for resource in sorted(groups):
        images = sorted(groups[resource]["IMAGE"], key=lambda r: num(r, "ordinal"))
        chunks = {num(r, "event"): r for r in groups[resource]["CHUNK"] if r.get("record") == "2"}
        direct = [r for r in images if r.get("kind") == "direct"]
        for r in direct:
            event = num(r, "event")
            chunk = chunks.get(event)
            if not chunk or chunk.get("marker") != r.get("marker"):
                print(f"M449_ERROR image_chunk_mismatch resource={resource} event={event}")
                return 2
            row = dict(resource=resource, ordinal=num(r, "ordinal"), event=event,
                       marker=r["marker"], headerA=num(r, "headerA"),
                       headerB=num(r, "headerB"), dataBytes=num(r, "dataBytes"),
                       chunkOffset=num(chunk, "chunkOffset"),
                       chunkBytes=num(chunk, "chunkBytes"), tag=chunk["tag"])
            rows.append(row)
            print("M449_IMAGE " + " ".join(f"{k}={v}" for k,v in row.items()))
    markers = sorted(set(r["marker"] for r in rows))
    print(f"M449_INPUT resources={len(groups)} directImages={len(rows)} markers={','.join(markers)} source=M325 readOnly=1")
    for resource in sorted(groups):
        rr = [r for r in rows if r["resource"] == resource]
        for marker in markers:
            subset = [r for r in rr if r["marker"] == marker]
            if not subset:
                continue
            sizes = sorted(r["dataBytes"] for r in subset)
            dims = sorted(set(f'{r["headerA"]}x{r["headerB"]}' for r in subset))
            print(f"M449_GROUP resource={resource} marker={marker} count={len(subset)} minBytes={sizes[0]} maxBytes={sizes[-1]} dimensions={','.join(dims)}")
    troi = sorted((r for r in rows if r["resource"] == "BRDGTROI.SPR"), key=lambda r:r["ordinal"])
    transitions = [(a,b) for a,b in zip(troi,troi[1:]) if a["marker"] != b["marker"]]
    for a,b in transitions:
        print(f'M449_TRANSITION resource=BRDGTROI.SPR fromOrdinal={a["ordinal"]} toOrdinal={b["ordinal"]} fromEvent={a["event"]} toEvent={b["event"]} fromMarker={a["marker"]} toMarker={b["marker"]} fromBytes={a["dataBytes"]} toBytes={b["dataBytes"]}')
    dimensions = defaultdict(set)
    for r in rows:
        dimensions[(r["headerA"],r["headerB"])].add(r["marker"])
    same_dimension_mixed = sorted((d,m) for d,m in dimensions.items() if len(m)>1)
    for (a,b), ms in same_dimension_mixed:
        print(f"M449_COUNTEREXAMPLE hypothesis=marker_determined_by_dimensions dimensions={a}x{b} markers={','.join(sorted(ms))}")
    # A global, non-overlapping payload-size range would allow size to
    # distinguish marker values; overlapping observed ranges reject it.
    ranges = {m:(min(r["dataBytes"] for r in rows if r["marker"]==m),max(r["dataBytes"] for r in rows if r["marker"]==m)) for m in markers}
    for m,(lo,hi) in sorted(ranges.items()):
        print(f"M449_SIZE_RANGE marker={m} minBytes={lo} maxBytes={hi}")
    overlap = len(markers)==2 and max(ranges[markers[0]][0],ranges[markers[1]][0]) <= min(ranges[markers[0]][1],ranges[markers[1]][1])
    print(f"M449_HYPOTHESIS marker_determined_by_dimensions={'REJECT' if same_dimension_mixed else 'NOT_REJECTED'} counterexampleDimensions={len(same_dimension_mixed)}")
    print(f"M449_HYPOTHESIS disjoint_payload_size_ranges={'REJECT' if overlap else 'NOT_REJECTED'} overlap={int(overlap)}")
    print(f"M449_SUMMARY transitions={len(transitions)} sameDimensionMixed={len(same_dimension_mixed)} decoderConfirmed=0")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
