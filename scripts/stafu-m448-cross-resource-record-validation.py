#!/usr/bin/env python3
"""M448: falsifiable cross-resource checks on the M325 SPR record inventory.

Input is the ORIGINAL m325-record-structure.txt diagnostic output (UTF-16LE
PowerShell output is accepted). No original game assets are read.
Checks are structural consistency tests, NOT evidence of codec semantics.

Usage: python scripts/stafu-m448-cross-resource-record-validation.py m325-record-structure.txt
"""
from __future__ import annotations
import collections
from pathlib import Path
import re
import sys

def read_text(path: Path) -> str:
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe", b"\xfe\xff")):
        return raw.decode("utf-16")
    if raw[:256].count(b"\x00") > 32:
        return raw.decode("utf-16-le")
    return raw.decode("utf-8-sig")

def parse(text: str):
    result = collections.defaultdict(lambda: collections.defaultdict(list))
    for line in text.splitlines():
        if not line.startswith("SPR_M325_"):
            continue
        parts = line.split()
        kind = parts[0][len("SPR_M325_"):]
        data = dict(p.split("=", 1) for p in parts[1:] if "=" in p)
        if "resource" in data:
            result[data["resource"]][kind].append(data)
    return result

def integer(row, key):
    return int(row[key], 0)

def validate(resources):
    failed = 0
    tested = 0
    def check(resource, hypothesis, condition, detail):
        nonlocal failed, tested
        tested += 1
        if not condition:
            failed += 1
        print(f"M448_CHECK resource={resource} hypothesis={hypothesis} status={'PASS' if condition else 'REJECT'} {detail}")

    if len(resources) < 2:
        print("M448_ERROR need at least two independently inventoried SPR resources")
        return 2
    print(f"M448_INPUT resources={len(resources)} source=M325 readOnly=1")
    for name in sorted(resources):
        group = resources[name]
        records = sorted(group["RECORD"], key=lambda x: integer(x, "record"))
        chunks = sorted((x for x in group["CHUNK"] if x.get("record") == "2"), key=lambda x: integer(x, "event"))
        images = sorted(group["IMAGE"], key=lambda x: integer(x, "ordinal"))
        summaries = group["RECORD2_SUMMARY"]
        metadata = group["RESOURCE"]
        if len(metadata) != 1 or len(summaries) != 1 or not records:
            check(name, "complete_inventory", False, "missing_or_duplicate_metadata=1")
            continue
        size = integer(metadata[0], "fileBytes")
        check(name, "record_partition", all(integer(x, "start") <= integer(x, "end") <= size and integer(x, "bytes") == integer(x, "end") - integer(x, "start") for x in records) and all(integer(a, "end") == integer(b, "start") for a,b in zip(records,records[1:])) and integer(records[-1], "end") == size, f"records={len(records)} fileBytes={size}")
        r2 = next((x for x in records if x.get("record") == "2"), None)
        if r2 is None or not chunks:
            check(name, "record2_chunk_partition", False, "missing_record2=1")
            continue
        start, end = integer(r2, "start"), integer(r2, "end")
        contiguous = integer(chunks[0], "chunkOffset") == start and integer(chunks[-1], "chunkOffset") + integer(chunks[-1], "chunkBytes") == end
        contiguous &= all(integer(a, "chunkOffset") + integer(a, "chunkBytes") == integer(b, "chunkOffset") for a,b in zip(chunks,chunks[1:]))
        contiguous &= all(integer(x, "chunkBytes") == integer(x, "payloadBytes") + 8 and integer(x, "payloadOffset") == integer(x, "chunkOffset") + 8 for x in chunks)
        check(name, "record2_chunk_partition", contiguous, f"chunks={len(chunks)} record2Bytes={end-start}")
        check(name, "image_event_bijection", len(images) == sum(x.get("imageHeader") == "1" for x in chunks) and all(any(x.get("event") == y.get("event") and x.get("marker") == y.get("marker") and x.get("imageDataOffset") == y.get("dataOffset") and x.get("imageDataBytes") == y.get("dataBytes") for x in chunks) for y in images), f"images={len(images)}")
        check(name, "image_payload_containment", all(integer(x, "dataOffset") + integer(x, "dataBytes") == integer(x, "chunkEnd") and start <= integer(x, "dataOffset") <= integer(x, "chunkEnd") <= end for x in images), f"images={len(images)}")
        direct = [x for x in images if x.get("kind") == "direct"]
        reference = [x for x in images if x.get("kind") == "reference"]
        check(name, "summary_counts", len(images) == integer(summaries[0], "images") and len(direct) == integer(summaries[0], "direct") and len(reference) == integer(summaries[0], "references"), f"direct={len(direct)} references={len(reference)}")
        markers = sorted(set(x["marker"] for x in direct))
        check(name, "uniform_direct_marker", len(markers) == 1, f"markerVariants={len(markers)}")
        print(f"M448_RESOURCE resource={name} direct={len(direct)} references={len(reference)} markers={','.join(markers)} dimensions={','.join(sorted(set(x['headerA']+'x'+x['headerB'] for x in direct)))}")
    print(f"M448_SUMMARY checks={tested} rejected={failed} passed={tested-failed} structuralOnly=1 decoderConfirmed=0")
    return 1 if failed else 0

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    path = Path(sys.argv[1])
    if not path.is_file():
        print("M448_ERROR input not found:", path)
        return 2
    return validate(parse(read_text(path)))

if __name__ == "__main__":
    raise SystemExit(main())
