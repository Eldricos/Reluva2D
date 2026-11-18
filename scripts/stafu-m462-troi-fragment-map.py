#!/usr/bin/env python3
"""M462: global exact-fragment map across nine BRDGTROI.SPR frames.
Usage: python scripts/stafu-m462-troi-fragment-map.py m325-record-structure.txt games/STTNG games/STAFU-CD > m462-troi-fragment-map.txt
Reports maximal pairwise exact runs >=6, global fragment occurrences, coverage and
short-match evidence for marker-distinct frame 6. Read-only; not a decoder.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,sys,zipfile
NAME="BRDGTROI.SPR"
SHA="664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"
MIN_RUN=6
def report_text(p):
    b=p.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")):return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")
def locate(roots):
    for root in roots:
        if root.is_dir():
            for p in (root/NAME,root/NAME.lower(),root/"STTNG"/NAME):
                if p.is_file():return p.read_bytes(),str(p)
            archives=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):archives=(root,)
        else:continue
        for p in archives:
            if not p.is_file() or not zipfile.is_zipfile(p):continue
            with zipfile.ZipFile(p) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==NAME]
                if len(found)==1:return z.read(found[0]),str(p)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entries")
    raise FileNotFoundError(NAME)
def maximal_runs(a,b,minimum):
    # Enumerate maximal diagonal matches; no arbitrary choice of a single alignment.
    result=[]
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift);ib=ia+shift;limit=min(len(a)-ia,len(b)-ib);k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:k+=1;continue
            start=k
            while k<limit and a[ia+k]==b[ib+k]:k+=1
            if k-start>=minimum:result.append((ia+start,ib+start,k-start))
    return sorted(result,key=lambda t:(t[0],t[1],-t[2]))
def occurrences(fragment,blobs):
    found=[]
    for frame,blob in sorted(blobs.items()):
        start=0
        while True:
            p=blob.find(fragment,start)
            if p<0:break
            found.append((frame,p))
            start=p+1
    return found
def covered_intervals(mask):
    spans=[];start=None
    for i,v in enumerate(mask):
        if v and start is None:start=i
        if not v and start is not None:spans.append((start,i));start=None
    if start is not None:spans.append((start,len(mask)))
    return spans
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    report=Path(sys.argv[1])
    if not report.is_file():print("M462_ERROR missing_report");return 2
    rows={}
    for line in report_text(report).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        fields=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if fields.get("resource","").upper()!=NAME or fields.get("kind")!="direct":continue
        i=int(fields["ordinal"],0)
        if i in rows:print(f"M462_ERROR duplicate_frame={i}");return 2
        rows[i]=fields
    if sorted(rows)!=list(range(1,10)):print(f"M462_ERROR expected_frames_1_to_9 actual={sorted(rows)}");return 2
    try:raw,source=locate([Path(p) for p in sys.argv[2:]])
    except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M462_ERROR resource={NAME} detail={e}");return 2
    digest=hashlib.sha256(raw).hexdigest()
    if digest!=SHA:print(f"M462_ERROR hash={digest} expected={SHA}");return 2
    blobs={}
    for i,r in sorted(rows.items()):
        start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
        if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M462_ERROR bounds frame={i}");return 2
        blobs[i]=raw[start:end]
    print(f"M462_RESOURCE resource={NAME} sha256={digest} source={source} frames=9 minRun={MIN_RUN}")
    for i in sorted(blobs):
        print(f"M462_FRAME frame={i} marker={rows[i].get('marker','NA')} bytes={len(blobs[i])}")
    pair_runs={};fragment_keys=set();mask={i:bytearray(len(b)) for i,b in blobs.items()}
    frame6_short=Counter()
    for i in range(1,10):
        for j in range(i+1,10):
            found=maximal_runs(blobs[i],blobs[j],MIN_RUN)
            pair_runs[i,j]=found
            for a,b,n in found:
                fragment_keys.add(blobs[i][a:a+n])
                mask[i][a:a+n]=b"\x01"*n
                mask[j][b:b+n]=b"\x01"*n
            print(f"M462_PAIR a={i} b={j} markerSame={int(rows[i].get('marker')==rows[j].get('marker'))} runs={len(found)} matchedBytes={sum(n for _,_,n in found)} maxRun={max((n for _,_,n in found),default=0)} shifts={','.join(f'{b-a}:{n}' for a,b,n in found) or 'none'}")
            for a,b,n in found:
                print(f"M462_RUN a={i} b={j} offsetA={a} offsetB={b} bytes={n} shift={b-a} hex={blobs[i][a:a+n].hex()}")
            if 6 in (i,j):
                for threshold in (2,3,4,5):
                    shorter=maximal_runs(blobs[i],blobs[j],threshold)
                    best=max((n for _,_,n in shorter),default=0)
                    count=sum(1 for _,_,n in shorter if n<threshold+1)
                    frame6_short[threshold]+=len(shorter)
                    print(f"M462_FRAME6_SHORT other={j if i==6 else i} minRun={threshold} runs={len(shorter)} longest={best} exactlyThreshold={count} examples={';'.join(f'{a}/{b}/{n}/{blobs[i][a:a+n].hex()}' for a,b,n in shorter[:12]) or 'none'}")
    for i in sorted(blobs):
        spans=covered_intervals(mask[i]);uncovered=covered_intervals(bytearray(1-v for v in mask[i]))
        print(f"M462_COVERAGE frame={i} covered={sum(mask[i])} total={len(mask[i])} fraction={sum(mask[i])/len(mask[i]):.4f} coveredSpans={';'.join(f'{a}-{b}' for a,b in spans) or 'none'} uncoveredSpans={';'.join(f'{a}-{b}' for a,b in uncovered) or 'none'}")
    # Each distinct maximal pairwise match is a fragment; occurrences may extend
    # to more than the two frames in which it was originally detected.
    catalog=[]
    for frag in fragment_keys:
        loc=occurrences(frag,blobs)
        frames=sorted({i for i,_ in loc})
        if len(frames)<2:raise AssertionError("pairwise fragment not globally shared")
        catalog.append((frag,loc,frames))
    catalog.sort(key=lambda t:(-len(t[2]),-len(t[0]),t[0]))
    for rank,(frag,loc,frames) in enumerate(catalog,1):
        print(f"M462_FRAGMENT rank={rank} bytes={len(frag)} frameCount={len(frames)} occurrenceCount={len(loc)} frames={','.join(map(str,frames))} positions={';'.join(f'{i}:{p}' for i,p in loc)} hex={frag.hex()}")
    print(f"M462_SUMMARY pairs={len(pair_runs)} pairRuns={sum(map(len,pair_runs.values()))} distinctFragments={len(catalog)} frame6Coverage={sum(mask[6])} frame6ShortCounts={','.join(f'{k}:{v}' for k,v in sorted(frame6_short.items()))} readOnly=1 decoderConfirmed=0")
    print("M462_NOTE fragments=pairwise_maximal_exact_runs not_codec_commands=1 shortMatchesMayBeCoincidental=1 shiftsAreByteOffsetsNotPixelCoordinates=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
