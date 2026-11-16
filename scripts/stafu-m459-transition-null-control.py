#!/usr/bin/env python3
"""M459: sample-size-controlled JSD for SPR frame transition distributions.
Usage: python scripts/stafu-m459-transition-null-control.py m325-record-structure.txt games/STTNG games/STAFU-CD > m459-transition-null-control.txt
Deterministic, read-only, 200 without-replacement samples per comparison.
Null: subset transitions drawn independently from each corresponding whole frame.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,math,random,sys,zipfile
NAMES=("BRDGDATA.SPR","BRDGPICA.SPR","BRDGTROI.SPR")
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26ac512f0f4f0d2674bb3a7"}
# Keep reference hashes independent of analysis outputs.
HASHES["BRDGDATA.SPR"]="0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9"
HASHES["BRDGPICA.SPR"]="59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360"
HASHES["BRDGTROI.SPR"]="664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"
REPS=200;MIN_RUN=8;SEED=459
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
def matching_masks(a,b,minimum):
    ma=bytearray(len(a));mb=bytearray(len(b))
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift);ib=ia+shift;limit=min(len(a)-ia,len(b)-ib);k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:k+=1;continue
            start=k
            while k<limit and a[ia+k]==b[ib+k]:k+=1
            if k-start>=minimum:
                ma[ia+start:ia+k]=b"\x01"*(k-start)
                mb[ib+start:ib+k]=b"\x01"*(k-start)
    return ma,mb
def jsd(a,b):
    na=sum(a.values());nb=sum(b.values())
    if not na or not nb:return None
    result=0.
    for k in set(a)|set(b):
        p=a[k]/na;q=b[k]/nb;m=(p+q)/2
        if p:result+=p*math.log2(p/m)/2
        if q:result+=q*math.log2(q/m)/2
    return result
def percentile(vals,p):
    s=sorted(vals);return s[int((len(s)-1)*p)]
def fmt(v):return "NA" if v is None else f"{v:.4f}"
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    path=Path(sys.argv[1])
    if not path.is_file():print("M459_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in read_report(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print(f"M459_ERROR duplicate resource={name} ordinal={idx}");return 2
        rows[name][idx]=r
    data={}
    for name in NAMES:
        try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M459_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M459_ERROR hash resource={name} sha256={digest}");return 2
        if len(rows[name])!=9:print(f"M459_ERROR expected_9_direct_frames resource={name} actual={len(rows[name])}");return 2
        print(f"M459_RESOURCE resource={name} sha256={digest} source={source}")
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M459_ERROR bounds resource={name} ordinal={idx}");return 2
            data[name,idx]=raw[start:end]
    rng=random.Random(SEED)
    for name in NAMES:
        ids=sorted(rows[name]);blobs={i:data[name,i] for i in ids}
        masks={i:bytearray(len(blobs[i])) for i in ids}
        for pos,i in enumerate(ids):
            for j in ids[pos+1:]:
                ma,mb=matching_masks(blobs[i],blobs[j],MIN_RUN)
                for k,v in enumerate(ma):
                    if v:masks[i][k]=1
                for k,v in enumerate(mb):
                    if v:masks[j][k]=1
        arrays={};groups={}
        for i in ids:
            b=blobs[i];arr=[(b[k+1]-b[k])&255 for k in range(len(b)-1)]
            arrays[i]=arr
            shared=[arr[k] for k in range(len(arr)) if masks[i][k] and masks[i][k+1]]
            unique=[arr[k] for k in range(len(arr)) if not masks[i][k] and not masks[i][k+1]]
            groups[i]={"shared":shared,"unique":unique}
            print(f"M459_FRAME resource={name} ordinal={i} fullTransitions={len(arr)} sharedTransitions={len(shared)} uniqueTransitions={len(unique)}")
        totals=defaultdict(list);counts=Counter()
        for pos,i in enumerate(ids):
            for j in ids[pos+1:]:
                cohort="marker2_comparison" if name=="BRDGTROI.SPR" and 6 in (i,j) else "same_marker"
                for group in ("shared","unique"):
                    a=groups[i][group];b=groups[j][group]
                    if not a or not b:
                        print(f"M459_PAIR resource={name} a={i} b={j} cohort={cohort} group={group} observed=NA reason=empty_group")
                        continue
                    obs=jsd(Counter(a),Counter(b));null=[]
                    for _ in range(REPS):
                        sa=rng.sample(arrays[i],len(a));sb=rng.sample(arrays[j],len(b))
                        null.append(jsd(Counter(sa),Counter(sb)))
                    low=percentile(null,.025);median=percentile(null,.5);high=percentile(null,.975)
                    # +1 correction: conservative Monte Carlo tail proportion.
                    upper=(1+sum(v>=obs for v in null))/(REPS+1)
                    lower=(1+sum(v<=obs for v in null))/(REPS+1)
                    delta=obs-median
                    totals[cohort,group].append(delta);counts[cohort,group]+=1
                    print(f"M459_PAIR resource={name} a={i} b={j} cohort={cohort} group={group} nA={len(a)} nB={len(b)} observed={fmt(obs)} nullP025={fmt(low)} nullMedian={fmt(median)} nullP975={fmt(high)} deltaMedian={fmt(delta)} upperTail={fmt(upper)} lowerTail={fmt(lower)}")
        for (cohort,group),vals in sorted(totals.items()):
            print(f"M459_AGGREGATE resource={name} cohort={cohort} group={group} pairs={counts[cohort,group]} meanDeltaMedian={fmt(sum(vals)/len(vals))} positiveDeltas={sum(x>0 for x in vals)} negativeDeltas={sum(x<0 for x in vals)}")
        print(f"M459_NOTE resource={name} null=uniform_without_replacement_from_corresponding_full_frame transitions=adjacent_modulo256 seed={SEED} repetitions={REPS} sampleSizeControlled=1 independenceNotAssumed=0 multipleTestingAdjusted=0")
    print(f"M459_SUMMARY resources=3 frames=27 repetitions={REPS} readOnly=1 decoderConfirmed=0")
    return 0
if __name__=="__main__":raise SystemExit(main())
