#!/usr/bin/env python3
"""M461: controlled analysis of Troi SPR differing intervals.
Usage: python scripts/stafu-m461-troi-intervals.py m325-record-structure.txt games/STTNG games/STAFU-CD > m461-troi-intervals.txt
Read-only; comparisons are byte-level and do not imply decoded pixels or opcodes.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,sys,zipfile
NAME="BRDGTROI.SPR"
SHA="664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"
PAIRS=((2,4),(5,8))
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
def common_prefix(a,b):
    i=0
    while i<min(len(a),len(b)) and a[i]==b[i]:i+=1
    return i
def common_suffix(a,b,limit):
    i=0
    while i<min(len(a),len(b))-limit and a[-1-i]==b[-1-i]:i+=1
    return i
def matches(a,b,n):
    out=[]
    for i in range(len(a)-n+1):
        for j in range(len(b)-n+1):
            if a[i:i+n]==b[j:j+n]:out.append((i,j,a[i:i+n].hex()))
    return out
def edit_distance(a,b):
    # Unit-cost insertion/deletion/substitution; canonical traceback is deterministic.
    m=len(a);n=len(b)
    dp=[[0]*(n+1) for _ in range(m+1)]
    for i in range(m+1):dp[i][0]=i
    for j in range(n+1):dp[0][j]=j
    for i in range(1,m+1):
        for j in range(1,n+1):
            dp[i][j]=min(dp[i-1][j]+1,dp[i][j-1]+1,dp[i-1][j-1]+(a[i-1]!=b[j-1]))
    i=m;j=n;ops=[]
    while i or j:
        if i and j and a[i-1]==b[j-1] and dp[i][j]==dp[i-1][j-1]:
            ops.append(("equal",i-1,j-1));i-=1;j-=1
        elif i and j and dp[i][j]==dp[i-1][j-1]+1:
            ops.append(("replace",i-1,j-1));i-=1;j-=1
        elif i and dp[i][j]==dp[i-1][j]+1:
            ops.append(("delete",i-1,j));i-=1
        else:
            ops.append(("insert",i,j-1));j-=1
    ops.reverse()
    return dp[m][n],ops
def repeat_kmers(b,k):
    c=Counter(b[i:i+k] for i in range(len(b)-k+1))
    return [(x.hex(),n) for x,n in c.most_common() if n>=2]
def show(label,items,limit=16):
    return ",".join(f"{x}:{n}" for x,n in items[:limit]) or "none"
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    p=Path(sys.argv[1])
    if not p.is_file():print("M461_ERROR missing_report");return 2
    rows={}
    for line in report_text(p).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("resource","").upper()!=NAME or r.get("kind")!="direct":continue
        idx=int(r["ordinal"],0)
        if idx in rows:print(f"M461_ERROR duplicate_ordinal={idx}");return 2
        rows[idx]=r
    if len(rows)!=9:print(f"M461_ERROR expected_9_frames actual={len(rows)}");return 2
    try:raw,source=locate([Path(s) for s in sys.argv[2:]])
    except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M461_ERROR resource={NAME} detail={e}");return 2
    digest=hashlib.sha256(raw).hexdigest()
    if digest!=SHA:print(f"M461_ERROR sha256={digest} expected={SHA}");return 2
    print(f"M461_RESOURCE resource={NAME} sha256={digest} source={source}")
    blobs={}
    for idx,r in rows.items():
        start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
        if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M461_ERROR bounds ordinal={idx}");return 2
        blobs[idx]=raw[start:end]
    intervals={}
    for a_id,b_id in PAIRS:
        a=blobs[a_id];b=blobs[b_id]
        prefix=common_prefix(a,b);suffix=common_suffix(a,b,prefix)
        aa=a[prefix:len(a)-suffix if suffix else len(a)]
        bb=b[prefix:len(b)-suffix if suffix else len(b)]
        intervals[a_id,b_id]=(aa,bb)
        print(f"M461_PAIR a={a_id} b={b_id} markerA={rows[a_id].get('marker','NA')} markerB={rows[b_id].get('marker','NA')} payloadA={len(a)} payloadB={len(b)} prefix={prefix} suffix={suffix} intervalA={len(aa)} intervalB={len(bb)}")
        print(f"M461_INTERVAL a={a_id} b={b_id} side=A hex={aa.hex(' ')}")
        print(f"M461_INTERVAL a={a_id} b={b_id} side=B hex={bb.hex(' ')}")
        for k in (2,3,4):
            found=matches(aa,bb,k)
            shifts=Counter(j-i for i,j,_ in found)
            print(f"M461_MATCH a={a_id} b={b_id} k={k} count={len(found)} shifts={show('shifts',sorted(shifts.items(),key=lambda x:(-x[1],x[0])))} examples={';'.join(f'{i}/{j}/{h}' for i,j,h in found[:20]) or 'none'}")
            for side,blob in (("A",aa),("B",bb)):
                print(f"M461_REPEAT a={a_id} b={b_id} side={side} k={k} repeated={show('repeats',repeat_kmers(blob,k))}")
        distance,ops=edit_distance(aa,bb)
        counts=Counter(op for op,_,_ in ops)
        print(f"M461_EDIT a={a_id} b={b_id} distance={distance} equal={counts['equal']} replace={counts['replace']} delete={counts['delete']} insert={counts['insert']}")
        for op,i,j in ops:
            if op in ("insert","delete"):
                start_a=max(0,i-4);start_b=max(0,j-4)
                print(f"M461_EDIT_SHIFT a={a_id} b={b_id} op={op} indexA={i} indexB={j} neighborhoodA={aa[start_a:i+5].hex(' ')} neighborhoodB={bb[start_b:j+5].hex(' ')}")
    # Compare the four variable intervals to each other, not just paired counterparts.
    all_intervals=[(f"{i}-{j}-A",a) for (i,j),(a,b) in intervals.items()]+[(f"{i}-{j}-B",b) for (i,j),(a,b) in intervals.items()]
    for pos,(name_a,a) in enumerate(all_intervals):
        for name_b,b in all_intervals[pos+1:]:
            for k in (2,3,4):
                found=matches(a,b,k)
                print(f"M461_CROSS intervalA={name_a} intervalB={name_b} k={k} count={len(found)} examples={';'.join(f'{i}/{j}/{h}' for i,j,h in found[:12]) or 'none'}")
    print("M461_NOTE editTraceback=one_of_multiple_optimal_alignments insertionDeletionNotProofOfCodecField=1 shortMatchesCanOccurByChance=1")
    print("M461_SUMMARY pairs=2 intervals=4 readOnly=1 decoderConfirmed=0")
    return 0
if __name__=="__main__":raise SystemExit(main())
