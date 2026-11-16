#!/usr/bin/env python3
"""M454: ordered, non-overlapping exact-match alignment of SPR image payloads.
Usage: python scripts/stafu-m454-ordered-matches.py m325-record-structure.txt games/STTNG games/STAFU-CD > m454-ordered-matches.txt
Read-only; outputs offsets and statistics, not copyrighted resource bytes.
"""
from pathlib import Path
from collections import defaultdict
import hashlib
import sys
import zipfile

TARGETS=[("BRDGDATA.SPR",1,4),("BRDGDATA.SPR",2,8),("BRDGPICA.SPR",6,7),("BRDGTROI.SPR",2,4),("BRDGTROI.SPR",5,8)]
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}

def read_report(p):
    raw=p.read_bytes()
    if raw.startswith((b"\xff\xfe",b"\xfe\xff")):return raw.decode("utf-16")
    return raw.decode("utf-16-le" if raw[:256].count(b"\0")>32 else "utf-8-sig")

def locate(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file():return p.read_bytes(),str(p)
            archives=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):archives=(root,)
        else:continue
        for p in archives:
            if not p.is_file() or not zipfile.is_zipfile(p):continue
            with zipfile.ZipFile(p) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(p)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entries")
    raise FileNotFoundError(name)

def maximal_runs(a,b,min_length):
    out=[]
    for shift in range(1-len(a),len(b)):
        x=max(0,-shift);y=x+shift;limit=min(len(a)-x,len(b)-y);k=0
        while k<limit:
            if a[x+k]!=b[y+k]:k+=1;continue
            start=k
            while k<limit and a[x+k]==b[y+k]:k+=1
            if k-start>=min_length:out.append((x+start,y+start,k-start))
    return sorted(out)

def ordered_chain(runs):
    # Maximum total identical bytes subject to strict forward ordering.
    # Deterministic ties: fewer segments, then lexicographic offsets.
    best=[]
    for idx,(a,b,n) in enumerate(runs):
        candidates=[(n,((a,b,n),))]
        for prev in best:
            score,chain=prev
            pa,pb,pn=chain[-1]
            if pa+pn<=a and pb+pn<=b:
                candidates.append((score+n,chain+((a,b,n),)))
        chosen=max(candidates,key=lambda item:(item[0],-len(item[1]),tuple((-x,-y,-z) for x,y,z in item[1])))
        best.append(chosen)
    return max(best,key=lambda item:(item[0],-len(item[1]),tuple((-x,-y,-z) for x,y,z in item[1]))) if best else (0,())

def analyze(name,i,j,a,b,min_length):
    runs=maximal_runs(a,b,min_length)
    score,chain=ordered_chain(runs)
    print(f"M454_PAIR resource={name} a={i} b={j} lenA={len(a)} lenB={len(b)} minRun={min_length} candidateRuns={len(runs)} chainRuns={len(chain)} matchedBytes={score} unmatchedA={len(a)-score} unmatchedB={len(b)-score}")
    previousA=previousB=0
    for index,(x,y,n) in enumerate(chain,1):
        gapA=x-previousA;gapB=y-previousB
        print(f"M454_GAP resource={name} a={i} b={j} index={index} startA={previousA} startB={previousB} bytesA={gapA} bytesB={gapB} delta={gapB-gapA}")
        print(f"M454_MATCH resource={name} a={i} b={j} index={index} startA={x} startB={y} bytes={n} endA={x+n} endB={y+n} shift={y-x}")
        previousA=x+n;previousB=y+n
    print(f"M454_TAIL resource={name} a={i} b={j} startA={previousA} startB={previousB} bytesA={len(a)-previousA} bytesB={len(b)-previousB} delta={(len(b)-previousB)-(len(a)-previousA)}")
    print(f"M454_CHECK resource={name} a={i} b={j} lengthDelta={len(b)-len(a)} unmatchedDelta={(len(b)-score)-(len(a)-score)} conserved={int(len(b)-len(a)==(len(b)-score)-(len(a)-score))}")

def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    path=Path(sys.argv[1])
    if not path.is_file():print("M454_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in read_report(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();ordinal=int(r["ordinal"],0)
        if ordinal in rows[name]:print(f"M454_ERROR duplicate resource={name} ordinal={ordinal}");return 2
        rows[name][ordinal]=r
    data={}
    for name in sorted({t[0] for t in TARGETS}):
        try:raw,source=locate(name,[Path(p) for p in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M454_ERROR resource={name} detail={e}");return 2
        sha=hashlib.sha256(raw).hexdigest()
        if sha!=HASHES[name]:print(f"M454_ERROR unexpected_hash resource={name} sha256={sha}");return 2
        print(f"M454_RESOURCE resource={name} sha256={sha} source={source}")
        for ordinal,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M454_ERROR bounds resource={name} ordinal={ordinal}");return 2
            data[name,ordinal]=raw[start:end]
    for name,i,j in TARGETS:
        for threshold in (8,16):
            analyze(name,i,j,data[name,i],data[name,j],threshold)
    print(f"M454_SUMMARY pairs={len(TARGETS)} thresholds=8,16 readOnly=1 decoderConfirmed=0")
    return 0

if __name__=="__main__":raise SystemExit(main())
