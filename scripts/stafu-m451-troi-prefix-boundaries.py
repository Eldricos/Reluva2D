#!/usr/bin/env python3
"""M451: all-pairs Troi SPR prefix and shifted-match boundary investigation.

Usage:
  python scripts/stafu-m451-troi-prefix-boundaries.py m325-record-structure.txt games/STTNG games/STAFU-CD > m451-troi-prefix-boundaries.txt

Read-only: original game bytes are read locally and never printed or written.
A repeated-byte boundary is NOT evidence of a codec command boundary.
"""
from __future__ import annotations
import hashlib
import itertools
from pathlib import Path
import sys
import zipfile

EXPECTED_SHA = "664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"

def read_report(path):
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe", b"\xfe\xff")):
        return raw.decode("utf-16")
    if raw[:256].count(b"\0") > 32:
        return raw.decode("utf-16-le")
    return raw.decode("utf-8-sig")

def locate(name, roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name, root/name.lower(), root/"STTNG"/name):
                if p.is_file():
                    return p.read_bytes(), str(p)
            archives = (root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):
            archives = (root,)
        else:
            continue
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):
                continue
            with zipfile.ZipFile(archive) as zf:
                matches = [entry for entry in zf.namelist() if entry.replace("\\","/").split("/")[-1].upper() == name]
                if len(matches) == 1:
                    return zf.read(matches[0]),str(archive)+":"+matches[0]
                if len(matches) > 1:
                    raise ValueError("ambiguous archive entry")
    raise FileNotFoundError(name)

def prefix(a,b):
    n = min(len(a),len(b))
    for i in range(n):
        if a[i] != b[i]:
            return i
    return n

def matches(a,b):
    # Longest common substring, including shifted positions, with its offsets.
    prior = [0]*(len(b)+1)
    best = (0,0,0)
    for i,x in enumerate(a):
        curr = [0]*(len(b)+1)
        for j,y in enumerate(b):
            if x == y:
                size = prior[j]+1
                curr[j+1] = size
                if size > best[0]:
                    best = (size,i-size+1,j-size+1)
        prior = curr
    return best

def local_window(a,b,offset,width=16):
    end = min(len(a),len(b),offset+width)
    if offset >= end:
        return (0,0,0)
    eq = sum(a[i]==b[i] for i in range(offset,end))
    bits = sum(8-(a[i]^b[i]).bit_count() for i in range(offset,end))
    return (end-offset,eq,bits)

def main():
    if len(sys.argv)<3:
        print(__doc__.strip())
        return 2
    path = Path(sys.argv[1])
    if not path.is_file():
        print(f"M451_ERROR missing_report={path}")
        return 2
    entries = {}
    for line in read_report(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):
            continue
        row = dict(part.split("=",1) for part in line.split()[1:] if "=" in part)
        if row.get("resource","").upper()=="BRDGTROI.SPR" and row.get("kind")=="direct":
            ordinal = int(row["ordinal"],0)
            if ordinal in entries:
                print(f"M451_ERROR duplicate_ordinal={ordinal}")
                return 2
            entries[ordinal]=row
    if sorted(entries)!=list(range(1,10)):
        print(f"M451_ERROR expected_nine_direct_images found={sorted(entries)}")
        return 2
    try:
        raw,source=locate("BRDGTROI.SPR",[Path(x) for x in sys.argv[2:]])
    except (OSError,ValueError,zipfile.BadZipFile) as exc:
        print(f"M451_ERROR resource={exc}")
        return 2
    digest=hashlib.sha256(raw).hexdigest()
    if digest!=EXPECTED_SHA:
        print(f"M451_ERROR unexpected_resource_sha256={digest}")
        return 2
    payloads={}
    for ordinal,row in sorted(entries.items()):
        start=int(row["dataOffset"],0)
        size=int(row["dataBytes"],0)
        end=int(row["chunkEnd"],0)
        if start<0 or size<=0 or start+size!=end or end>len(raw):
            print(f"M451_ERROR invalid_bounds ordinal={ordinal}")
            return 2
        payloads[ordinal]=raw[start:end]
    print(f"M451_INPUT resource=BRDGTROI.SPR sha256={digest} source={source} directImages={len(payloads)} readOnly=1")
    results=[]
    for i,j in itertools.combinations(sorted(payloads),2):
        a,b=payloads[i],payloads[j]
        p=prefix(a,b)
        run,atA,atB=matches(a,b)
        # Distinguish aligned prefix from a shifted common substring.
        equal=sum(x==y for x,y in zip(a,b))
        markerA,markerB=entries[i]["marker"],entries[j]["marker"]
        print(f"M451_PAIR a={i} b={j} markerA={markerA} markerB={markerB} prefix={p} alignedEqual={equal} commonRun={run} runOffsetA={atA} runOffsetB={atB} runShift={atB-atA}")
        results.append((p,i,j,run,atA,atB))
    for p,i,j,run,atA,atB in sorted(results,reverse=True)[:10]:
        print(f"M451_RANK a={i} b={j} prefix={p} longestRun={run} offsetA={atA} offsetB={atB}")
    a,b=payloads[5],payloads[7]
    boundary=prefix(a,b)
    print(f"M451_BOUNDARY a=5 b=7 sharedPrefix={boundary} lengthA={len(a)} lengthB={len(b)}")
    for offset in (max(0,boundary-32),max(0,boundary-16),max(0,boundary-8),boundary,boundary+8,boundary+16):
        n,eq,bits=local_window(a,b,offset)
        print(f"M451_WINDOW a=5 b=7 start={offset} bytes={n} equalBytes={eq} equalBits={bits} comparedBits={n*8}")
    # A shifted-match search around the first mismatch, excluding the already
    # identical prefix, to look for local resynchronization.
    for shift in range(-16,17):
        startA=boundary
        startB=boundary+shift
        if startB<0 or startB>=len(b):
            continue
        n=min(32,len(a)-startA,len(b)-startB)
        if n<=0:
            continue
        eq=sum(a[startA+k]==b[startB+k] for k in range(n))
        print(f"M451_SHIFT a=5 b=7 shift={shift} compared={n} equalBytes={eq}")
    print(f"M451_SUMMARY pairCount={len(results)} topPrefix={max(x[0] for x in results)} boundary={boundary} codecBoundaryProven=0 decoderConfirmed=0")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
