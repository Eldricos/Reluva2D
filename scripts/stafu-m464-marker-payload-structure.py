#!/usr/bin/env python3
"""M464: marker-associated SPR payload statistics, with size-controlled comparisons.
Usage: python scripts/stafu-m464-marker-payload-structure.py m325-record-structure.txt games/STTNG games/STAFU-CD > m464-marker-payload-structure.txt
Read-only. Marker associations do not identify a codec.
"""
from collections import Counter, defaultdict
from pathlib import Path
import hashlib, math, sys, zipfile

def read_report(path):
    b=path.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")): return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")

def locate(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file(): return p.read_bytes(),str(p)
            archives=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root): archives=(root,)
        else: continue
        for p in archives:
            if not p.is_file() or not zipfile.is_zipfile(p): continue
            with zipfile.ZipFile(p) as z:
                names=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(names)>1: raise ValueError("ambiguous archive entries "+name)
                if names: return z.read(names[0]),str(p)+":"+names[0]
    raise FileNotFoundError(name)

def entropy(counts):
    total=sum(counts)
    return -sum((n/total)*math.log2(n/total) for n in counts if n) if total else 0.

def stats(b):
    c=Counter(b); ones=sum(x.bit_count() for x in b)
    adjacent=sum(b[i]==b[i-1] for i in range(1,len(b)))
    runs=[]; start=0
    for i in range(1,len(b)+1):
        if i==len(b) or b[i]!=b[start]:
            runs.append(i-start);start=i
    return dict(length=len(b),unique=len(c),entropy=entropy(c.values()),ones=ones/(8*len(b)),
                zero=c[0]/len(b),ff=c[255]/len(b),adjacent=adjacent/max(1,len(b)-1),
                maxrun=max(runs),bitTransitions=sum(((b[i]^b[i-1]).bit_count()) for i in range(1,len(b)))/max(1,8*(len(b)-1)))

def prefix(a,b):
    n=0
    while n<min(len(a),len(b)) and a[n]==b[n]: n+=1
    return n

def main():
    if len(sys.argv)<3: print(__doc__.strip());return 2
    report=Path(sys.argv[1])
    if not report.is_file():print("M464_ERROR missing_report");return 2
    rows=defaultdict(list)
    for line in read_report(report).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        d=dict(x.split("=",1) for x in line.split()[1:] if "=" in x)
        if d.get("kind")!="direct" or not d.get("resource","").upper().endswith(".SPR"):continue
        try:
            d["_ordinal"]=int(d["ordinal"],0)
            d["_start"]=int(d["dataOffset"],0)
            d["_size"]=int(d["dataBytes"],0)
            d["_end"]=int(d["chunkEnd"],0)
        except (KeyError,ValueError): print("M464_ERROR malformed_image_row");return 2
        rows[d["resource"].upper()].append(d)
    if not rows: print("M464_ERROR no_direct_spr_rows");return 2
    roots=[Path(s) for s in sys.argv[2:]]
    frames=[]; skipped=[]
    for name in sorted(rows):
        try: raw,source=locate(name,roots)
        except (OSError,ValueError,zipfile.BadZipFile) as e:
            skipped.append(name);print(f"M464_SKIP resource={name} reason={type(e).__name__}");continue
        print(f"M464_RESOURCE resource={name} frames={len(rows[name])} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
        seen=set()
        for r in sorted(rows[name],key=lambda x:x["_ordinal"]):
            idx=r["_ordinal"]; start=r["_start"];size=r["_size"];end=r["_end"]
            if idx in seen or start<0 or size<=0 or start+size!=end or end>len(raw):
                print(f"M464_ERROR invalid_frame resource={name} ordinal={idx}");return 2
            seen.add(idx)
            b=raw[start:end];s=stats(b)
            marker=r.get("marker","unknown")
            # Dimensions are taken only from report fields; never inferred from payload length.
            width=r.get("headerA","unknown");height=r.get("headerB","unknown")
            f=dict(name=name,idx=idx,marker=marker,width=width,height=height,b=b,**s)
            frames.append(f)
            print(f"M464_FRAME resource={name} ordinal={idx} marker={marker} widthField={width} heightField={height} bytes={len(b)} unique={s['unique']} entropy={s['entropy']:.5f} bitOne={s['ones']:.5f} bitTransition={s['bitTransitions']:.5f} zero={s['zero']:.5f} ff={s['ff']:.5f} adjacentEqual={s['adjacent']:.5f} maxRun={s['maxrun']} first8={b[:8].hex()} last8={b[-8:].hex()}")
    if not frames:print("M464_ERROR no_frames_loaded");return 2
    groups=defaultdict(list)
    for f in frames:groups[f["marker"]].append(f)
    for marker,items in sorted(groups.items()):
        def avg(k):return sum(x[k] for x in items)/len(items)
        print(f"M464_MARKER marker={marker} frames={len(items)} resources={len(set(x['name'] for x in items))} meanBytes={avg('length'):.3f} meanEntropy={avg('entropy'):.5f} meanBitOne={avg('ones'):.5f} meanBitTransition={avg('bitTransitions'):.5f} meanAdjacentEqual={avg('adjacent'):.5f} meanZero={avg('zero'):.5f} meanMaxRun={avg('maxrun'):.3f}")
    # Pairwise controls: dimensions and lengths are explicit, never assumed equivalent.
    comparisons=Counter();examples=defaultdict(list)
    for i,a in enumerate(frames):
        for b in frames[i+1:]:
            if a["name"]==b["name"]:continue
            marker_same=a["marker"]==b["marker"]
            dimensions_known=all(x not in ("unknown","") for x in (a["width"],a["height"],b["width"],b["height"]))
            dimension_same=dimensions_known and (a["width"],a["height"])==(b["width"],b["height"])
            length_near=abs(a["length"]-b["length"])<=max(8,round(.10*max(a["length"],b["length"])))
            category=("same" if marker_same else "different", "same" if dimension_same else "different_or_unknown","near" if length_near else "far")
            comparisons[category]+=1
            if len(examples[category])<8:
                examples[category].append(f"{a['name']}:{a['idx']}/{b['name']}:{b['idx']}/prefix{prefix(a['b'],b['b'])}/entropyDelta{abs(a['entropy']-b['entropy']):.3f}")
    for category,count in sorted(comparisons.items()):
        print(f"M464_CONTROL marker={category[0]} dimensions={category[1]} length={category[2]} pairs={count} examples={';'.join(examples[category])}")
    # Prefix repetition, separated by within-resource vs cross-resource and marker agreement.
    buckets=defaultdict(list)
    for i,a in enumerate(frames):
        for b in frames[i+1:]:
            key=("within" if a["name"]==b["name"] else "cross","same" if a["marker"]==b["marker"] else "different")
            buckets[key].append(prefix(a["b"],b["b"]))
    for (scope,agreement),values in sorted(buckets.items()):
        print(f"M464_PREFIX scope={scope} marker={agreement} pairs={len(values)} mean={sum(values)/len(values):.3f} max={max(values)} ge8={sum(v>=8 for v in values)}")
    print(f"M464_SUMMARY resourcesLoaded={len(set(f['name'] for f in frames))} frames={len(frames)} markers={len(groups)} skippedResources={len(skipped)} readOnly=1 codecConfirmed=0")
    print("M464_NOTE observationalOnly=1 smallMarkerGroupsAndResourceContentConfound=1 headerFieldsAreNotValidatedDimensions=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
