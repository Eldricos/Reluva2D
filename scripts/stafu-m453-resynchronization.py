#!/usr/bin/env python3
"""M453: targeted SPR resynchronization analysis.
Usage: python scripts/stafu-m453-resynchronization.py m325-record-structure.txt games/STTNG games/STAFU-CD > m453-resynchronization.txt
Read-only: reports offsets and statistics, never resource payload bytes.
"""
from pathlib import Path
from collections import defaultdict
import hashlib
import sys
import zipfile

TARGETS=[("BRDGDATA.SPR",1,4),("BRDGDATA.SPR",2,8),("BRDGPICA.SPR",6,7),("BRDGTROI.SPR",2,4),("BRDGTROI.SPR",5,8)]
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}

def read_report(path):
    b=path.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")):
        return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")

def locate(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file():
                    return p.read_bytes(),str(p)
            archives=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):
            archives=(root,)
        else:
            continue
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):
                continue
            with zipfile.ZipFile(archive) as zf:
                matches=[n for n in zf.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(matches)==1:
                    return zf.read(matches[0]),str(archive)+":"+matches[0]
                if len(matches)>1:
                    raise ValueError("ambiguous archive entries")
    raise FileNotFoundError(name)

def prefix(a,b):
    for i,(x,y) in enumerate(zip(a,b)):
        if x!=y:
            return i
    return min(len(a),len(b))

def exact_runs(a,b):
    results=[]
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift)
        ib=ia+shift
        limit=min(len(a)-ia,len(b)-ib)
        k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:
                k+=1
                continue
            start=k
            while k<limit and a[ia+k]==b[ib+k]:
                k+=1
            results.append((k-start,ia+start,ib+start))
    return results

def window(a,b,ia,ib,width):
    if ia<0 or ib<0:
        return 0,0,0
    n=max(0,min(width,len(a)-ia,len(b)-ib))
    eq=sum(a[ia+k]==b[ib+k] for k in range(n))
    bits=sum(8-(a[ia+k]^b[ib+k]).bit_count() for k in range(n))
    return n,eq,bits

def main():
    if len(sys.argv)<3:
        print(__doc__.strip())
        return 2
    report=Path(sys.argv[1])
    if not report.is_file():
        print(f"M453_ERROR missing_report={report}")
        return 2
    rows=defaultdict(dict)
    for line in read_report(report).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):
            continue
        r=dict(token.split("=",1) for token in line.split()[1:] if "=" in token)
        if r.get("kind")!="direct":
            continue
        name=r["resource"].upper()
        ordinal=int(r["ordinal"],0)
        if ordinal in rows[name]:
            print(f"M453_ERROR duplicate resource={name} ordinal={ordinal}")
            return 2
        rows[name][ordinal]=r
    roots=[Path(p) for p in sys.argv[2:]]
    payloads={}
    for name in sorted({x[0] for x in TARGETS}):
        try:
            raw,source=locate(name,roots)
        except (OSError,ValueError,zipfile.BadZipFile) as exc:
            print(f"M453_ERROR resource={name} detail={exc}")
            return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:
            print(f"M453_ERROR resource={name} unexpected_sha256={digest}")
            return 2
        print(f"M453_RESOURCE resource={name} sha256={digest} source={source}")
        for ordinal,r in rows[name].items():
            start=int(r["dataOffset"],0)
            size=int(r["dataBytes"],0)
            end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):
                print(f"M453_ERROR invalid_bounds resource={name} ordinal={ordinal}")
                return 2
            payloads[name,ordinal]=raw[start:end]
    for name,i,j in TARGETS:
        a=payloads[name,i]
        b=payloads[name,j]
        p=prefix(a,b)
        candidates=sorted((r for r in exact_runs(a,b) if r[0]>=16 and r[1]>=p and r[2]>=p and (r[1]>p or r[2]>p)),key=lambda r:(-r[0],r[1],r[2]))
        print(f"M453_PAIR resource={name} a={i} b={j} lenA={len(a)} lenB={len(b)} prefix={p} postRuns16={len(candidates)}")
        for length,oa,ob in candidates[:8]:
            print(f"M453_RUN resource={name} a={i} b={j} length={length} startA={oa} startB={ob} endA={oa+length} endB={ob+length} shift={ob-oa} tailA={len(a)-oa-length} tailB={len(b)-ob-length}")
            for label,x,y,width in (("before16",oa-16,ob-16,16),("before8",oa-8,ob-8,8),("after8",oa+length,ob+length,8),("after16",oa+length+8,ob+length+8,16)):
                n,eq,bits=window(a,b,x,y,width)
                print(f"M453_EDGE resource={name} a={i} b={j} runA={oa} runB={ob} side={label} compared={n} equalBytes={eq} equalBits={bits}")
            for shift in range(ob-oa-4,ob-oa+5):
                start=max(p,oa-32)
                n,eq,bits=window(a,b,start,start+shift,32)
                print(f"M453_PRE_SHIFT resource={name} a={i} b={j} runA={oa} runB={ob} shift={shift} compared={n} equalBytes={eq} equalBits={bits}")
    print(f"M453_SUMMARY targets={len(TARGETS)} readOnly=1 commandBoundariesProven=0 decoderConfirmed=0")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
