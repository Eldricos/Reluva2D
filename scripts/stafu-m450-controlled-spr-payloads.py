#!/usr/bin/env python3
"""M450: controlled byte-level SPR image-payload comparisons.

Usage:
  python scripts/stafu-m450-controlled-spr-payloads.py m325-record-structure.txt games/STTNG games/STAFU-CD > m450-controlled-spr-payloads.txt

Reads original SPR files from supplied directories or STTNG.ZIP archives.
Does not extract, write, or modify game resources. The report contains only
statistics, offsets, and short SHA-256 fingerprints, never raw payload bytes.
"""
from __future__ import annotations
import collections
import hashlib
import itertools
from pathlib import Path
import sys
import zipfile

def read_report(path):
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe", b"\xfe\xff")):
        return raw.decode("utf-16")
    if raw[:256].count(b"\0") > 32:
        return raw.decode("utf-16-le")
    return raw.decode("utf-8-sig")

def parse_report(text):
    groups = collections.defaultdict(list)
    for line in text.splitlines():
        if not line.startswith("SPR_M325_IMAGE "):
            continue
        row = dict(part.split("=", 1) for part in line.split()[1:] if "=" in part)
        groups[row["resource"].upper()].append(row)
    return groups

def locate(name, roots):
    for root in roots:
        if root.is_file() and zipfile.is_zipfile(root):
            archives = [root]
        elif root.is_dir():
            # Prefer installed files; archive fallback.
            direct = [root / name, root / name.lower(), root / "STTNG" / name]
            for path in direct:
                if path.is_file():
                    return path.read_bytes(), str(path)
            archives = [root / "STTNG.ZIP", root / "cd" / "STTNG.ZIP"]
        else:
            continue
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):
                continue
            with zipfile.ZipFile(archive) as zf:
                candidates = [x for x in zf.namelist() if x.replace("\\", "/").split("/")[-1].upper() == name]
                if len(candidates) == 1:
                    return zf.read(candidates[0]), str(archive) + ":" + candidates[0]
                if len(candidates) > 1:
                    raise ValueError(f"ambiguous archive entries for {name}: {len(candidates)}")
    raise FileNotFoundError(name)

def prefix(a,b):
    return next((i for i,(x,y) in enumerate(zip(a,b)) if x != y), min(len(a),len(b)))

def suffix(a,b):
    return prefix(a[::-1],b[::-1])

def longest_run(a,b):
    # Longest common contiguous byte sequence, including shifted alignment.
    previous = [0]*(len(b)+1)
    best = 0
    for x in a:
        current = [0]*(len(b)+1)
        for j,y in enumerate(b,1):
            if x == y:
                current[j] = previous[j-1]+1
                best = max(best,current[j])
        previous = current
    return best

def describe(name,row,raw):
    start = int(row["dataOffset"],0)
    length = int(row["dataBytes"],0)
    end = int(row["chunkEnd"],0)
    if start < 0 or length < 0 or start+length != end or end > len(raw):
        raise ValueError(f"invalid payload bounds {name} ordinal={row['ordinal']}")
    payload = raw[start:end]
    counts = collections.Counter(payload)
    return payload, dict(resource=name,ordinal=int(row["ordinal"],0),event=int(row["event"],0),
        marker=row["marker"],dimensions=row["headerA"]+"x"+row["headerB"],
        offset=start,bytes=len(payload),unique=len(counts),zero=counts[0],
        ff=counts[255],sha256=hashlib.sha256(payload).hexdigest()[:16])

def pair(a,b,kind):
    x,y=a[0],b[0]
    aligned = min(len(x),len(y))
    equal = sum(p==q for p,q in zip(x,y))
    bit_equal = sum(8-(p^q).bit_count() for p,q in zip(x,y))
    xor_nonzero = sum(p!=q for p,q in zip(x,y))
    print("M450_PAIR "+f"type={kind} a={a[1]['resource']}:{a[1]['ordinal']} b={b[1]['resource']}:{b[1]['ordinal']} "+
        f"markerA={a[1]['marker']} markerB={b[1]['marker']} lenA={len(x)} lenB={len(y)} "+
        f"prefix={prefix(x,y)} suffix={suffix(x,y)} alignedBytes={aligned} equalBytes={equal} "+
        f"differentBytes={xor_nonzero} equalBits={bit_equal} comparedBits={8*aligned} longestCommonRun={longest_run(x,y)}")

def main():
    if len(sys.argv) < 3:
        print(__doc__.strip())
        return 2
    report = Path(sys.argv[1])
    roots = [Path(x) for x in sys.argv[2:]]
    if not report.is_file():
        print(f"M450_ERROR report_missing={report}")
        return 2
    groups = parse_report(read_report(report))
    names = ["BRDGTROI.SPR","BRDGWORF.SPR"]
    samples = {}
    for name in names:
        try:
            raw, source = locate(name,roots)
        except (FileNotFoundError, ValueError, zipfile.BadZipFile) as exc:
            print(f"M450_ERROR resource={name} reason={exc}")
            return 2
        rows = sorted((r for r in groups[name] if r["kind"] == "direct"),key=lambda r:int(r["ordinal"],0))
        if not rows:
            print(f"M450_ERROR no_direct_images={name}")
            return 2
        print(f"M450_RESOURCE resource={name} bytes={len(raw)} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
        for row in rows:
            try:
                item = describe(name,row,raw)
            except ValueError as exc:
                print(f"M450_ERROR {exc}")
                return 2
            samples[(name,item[1]["ordinal"])] = item
            print("M450_IMAGE "+" ".join(f"{k}={v}" for k,v in item[1].items()))
    troi = [samples[("BRDGTROI.SPR",i)] for i in (5,6,7)]
    if not (troi[0][1]["marker"] == troi[2][1]["marker"] != troi[1][1]["marker"]):
        print("M450_ERROR unexpected_Troi_marker_sequence")
        return 2
    if len({x[1]["dimensions"] for x in troi}) != 1:
        print("M450_ERROR Troi_dimensions_mismatch")
        return 2
    for a,b in itertools.combinations(troi,2):
        pair(a,b,"troi_control")
    worf = [item for (name,_),item in sorted(samples.items()) if name=="BRDGWORF.SPR" and item[1]["dimensions"] == troi[1][1]["dimensions"]]
    for item in worf:
        pair(troi[1],item,"cross_resource_same_marker")
    # Compare the isolated marker to its immediate neighboring payloads
    # against a same-marker control in another resource.
    print(f"M450_SUMMARY troiControls=3 crossResourceControls={len(worf)} structuralOnly=1 decoderConfirmed=0")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
