#!/usr/bin/env python3
"""M458: cross-frame SPR transition consistency with shared-byte controls.
Usage: python scripts/stafu-m458-transition-consistency.py m325-record-structure.txt games/STTNG games/STAFU-CD > m458-transition-consistency.txt
Read-only; outputs aggregate counts, never game data bytes.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,math,sys,zipfile
NAMES=("BRDGDATA.SPR","BRDGPICA.SPR","BRDGTROI.SPR")
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}
MIN_RUN=8
def read_report(path):
    b=path.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")):return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")
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
def histogram(blob,allowed=None):
    h=Counter()
    for i in range(len(blob)-1):
        if allowed is None or (allowed[i] and allowed[i+1]):h[(blob[i+1]-blob[i])&255]+=1
    return h
def jsd(a,b):
    na=sum(a.values());nb=sum(b.values())
    if not na or not nb:return None
    result=0.
    for k in set(a)|set(b):
        p=a[k]/na;q=b[k]/nb;m=(p+q)/2
        if p:result+=p*math.log2(p/m)/2
        if q:result+=q*math.log2(q/m)/2
    return result
def fmt(v):return "NA" if v is None else f"{v:.4f}"
def top(h,n=8):return ",".join(f"{k}:{v}" for k,v in sorted(h.items(),key=lambda kv:(-kv[1],kv[0]))[:n]) or "none"
def matching_masks(a,b,minimum):
    # Every byte belonging to any exact match of >= minimum bytes,
    # regardless of alignment; not an inferred codec boundary.
    ma=bytearray(len(a));mb=bytearray(len(b));runs=0
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift);ib=ia+shift;limit=min(len(a)-ia,len(b)-ib);k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:k+=1;continue
            start=k
            while k<limit and a[ia+k]==b[ib+k]:k+=1
            if k-start>=minimum:
                runs+=1
                ma[ia+start:ia+k]=b"\x01"*(k-start)
                mb[ib+start:ib+k]=b"\x01"*(k-start)
    return ma,mb,runs
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    path=Path(sys.argv[1])
    if not path.is_file():print("M458_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in read_report(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print(f"M458_ERROR duplicate resource={name} ordinal={idx}");return 2
        rows[name][idx]=r
    data={}
    for name in NAMES:
        try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M458_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M458_ERROR hash resource={name} sha256={digest}");return 2
        if len(rows[name])!=9:print(f"M458_ERROR expected_9_direct_frames resource={name} actual={len(rows[name])}");return 2
        print(f"M458_RESOURCE resource={name} sha256={digest} source={source}")
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M458_ERROR bounds resource={name} ordinal={idx}");return 2
            data[name,idx]=raw[start:end]
    for name in NAMES:
        ids=sorted(rows[name]);blobs={i:data[name,i] for i in ids}
        masks={i:bytearray(len(blobs[i])) for i in ids}
        total_runs=0
        for pos,i in enumerate(ids):
            for j in ids[pos+1:]:
                ma,mb,r=matching_masks(blobs[i],blobs[j],MIN_RUN)
                total_runs+=r
                for k,v in enumerate(ma):
                    if v:masks[i][k]=1
                for k,v in enumerate(mb):
                    if v:masks[j][k]=1
        full={i:histogram(blobs[i]) for i in ids}
        shared={i:histogram(blobs[i],masks[i]) for i in ids}
        unique={i:histogram(blobs[i],bytearray(1-v for v in masks[i])) for i in ids}
        aggregate=Counter();aggregate_unique=Counter();aggregate_shared=Counter()
        for i in ids:
            aggregate.update(full[i]);aggregate_unique.update(unique[i]);aggregate_shared.update(shared[i])
            print(f"M458_FRAME resource={name} ordinal={i} bytes={len(blobs[i])} sharedBytes={sum(masks[i])} fullTransitions={sum(full[i].values())} sharedTransitions={sum(shared[i].values())} uniqueTransitions={sum(unique[i].values())} boundaryTransitions={sum(full[i].values())-sum(shared[i].values())-sum(unique[i].values())}")
        comparisons=0;full_js=[];unique_js=[];shared_js=[]
        for pos,i in enumerate(ids):
            for j in ids[pos+1:]:
                comparisons+=1
                f=jsd(full[i],full[j]);u=jsd(unique[i],unique[j]);s=jsd(shared[i],shared[j])
                full_js.append(f)
                if u is not None:unique_js.append(u)
                if s is not None:shared_js.append(s)
                print(f"M458_PAIR resource={name} a={i} b={j} fullJSD={fmt(f)} uniqueJSD={fmt(u)} sharedJSD={fmt(s)} uniqueTransitionsA={sum(unique[i].values())} uniqueTransitionsB={sum(unique[j].values())}")
        def mean(vals):return sum(vals)/len(vals) if vals else None
        print(f"M458_AGGREGATE resource={name} frames={len(ids)} pairs={comparisons} minRun={MIN_RUN} matchedRunsPairwise={total_runs} fullTransitions={sum(aggregate.values())} sharedTransitions={sum(aggregate_shared.values())} uniqueTransitions={sum(aggregate_unique.values())} fullMeanJSD={fmt(mean(full_js))} uniqueMeanJSD={fmt(mean(unique_js))} sharedMeanJSD={fmt(mean(shared_js))}")
        print(f"M458_TOP resource={name} group=full values={top(aggregate)}")
        print(f"M458_TOP resource={name} group=shared values={top(aggregate_shared)}")
        print(f"M458_TOP resource={name} group=unique values={top(aggregate_unique)}")
        print(f"M458_NOTE resource={name} sharedMeansAnyPairwiseExactRun=1 uniqueMeansOutsideAllRuns=1 boundaryTransitionsExcludedFromBoth=1 JSDIsDescriptiveNotSignificanceTest=1")
    print("M458_SUMMARY resources=3 frames=27 minRun=8 readOnly=1 decoderConfirmed=0")
    return 0
if __name__=="__main__":raise SystemExit(main())
