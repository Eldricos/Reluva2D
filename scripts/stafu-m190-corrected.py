#!/usr/bin/env python3
"""Read-only LE page-map correction for STAFU Milestone 190."""

import hashlib
import pathlib
import struct
import sys

EXPECTED = (
    "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
)

if len(sys.argv) != 2:
    sys.exit("Usage: python stafu-m190-corrected.py games/STTNG/STTNG.OVL")

p = pathlib.Path(sys.argv[1])
b = p.read_bytes()
sha = hashlib.sha256(b).hexdigest()

print(
    f"RM_M190_FIX_INPUT bytes={len(b)} "
    f"sha256={sha} knownShaMatch={int(sha == EXPECTED)}"
)

if sha != EXPECTED:
    sys.exit("ERROR: unexpected executable; no output written")

h = 229780
page_size = struct.unpack_from("<I", b, h + 0x28)[0]
objrel = struct.unpack_from("<I", b, h + 0x40)[0]
objcount = struct.unpack_from("<I", b, h + 0x44)[0]
maprel = struct.unpack_from("<I", b, h + 0x48)[0]
dataoff = struct.unpack_from("<I", b, h + 0x80)[0]

print(
    f"RM_M190_FIX_LE header={h} pageSize={page_size} "
    f"objectTableRel={objrel} objectCount={objcount} "
    f"pageMapRel={maprel} dataPagesOffset={dataoff}"
)

if (page_size, objcount) != (4096, 2):
    sys.exit("ERROR: unexpected LE metadata")

for i in range(objcount):
    o = h + objrel + i * 24
    size, base, flags, first, count = struct.unpack_from("<IIIII", b, o)

    off = 0x56E4
    page_index = off // page_size
    within = off % page_size

    if off + 36 > size or page_index >= count:
        print(
            f"RM_M190_FIX_OBJECT index={i+1} "
            "candidate=0 reason=outOfObject"
        )
        continue

    mapoff = h + maprel + (first - 1 + page_index) * 4
    raw = b[mapoff:mapoff + 4]
    page = int.from_bytes(raw[:3], "big")
    pf = raw[3]

    phys = (
        dataoff + (page - 1) * page_size + within
        if page else -1
    )

    print(
        f"RM_M190_FIX_CANDIDATE object={i+1} "
        f"base=0x{base:x} flags=0x{flags:x} "
        f"mapOffset={mapoff} entry={raw.hex()} "
        f"page={page} pageFlags={pf} physical={phys}"
    )

    if page == 0 or pf != 0 or not 0 <= phys <= len(b) - 36:
        print(f"RM_M190_FIX_TABLE object={i+1} extractable=0")
        continue

    chunk = b[phys:phys + 36]
    vals = struct.unpack("<9I", chunk)

    out = pathlib.Path(
        f"m190-fixed-object{i+1}-table-{phys}.bin"
    )
    out.write_bytes(chunk)

    print(
        f"RM_M190_FIX_TABLE object={i+1} "
        f"file={out} bytes={chunk.hex()} entries="
        + ",".join(f"0x{x:08x}" for x in vals)
    )

    start = max(0, phys - 64)
    end = min(len(b), phys + 128)

    win = pathlib.Path(
        f"m190-fixed-object{i+1}-context-{start}.bin"
    )
    win.write_bytes(b[start:end])

    print(
        f"RM_M190_FIX_WINDOW object={i+1} "
        f"from={start} to={end} file={win} "
        f"sha256={hashlib.sha256(b[start:end]).hexdigest()}"
    )

print(
    "RM_M190_FIX_LIMITATION "
    "relocationsUnapplied=1 handlerTargetsUnverified=1"
)
