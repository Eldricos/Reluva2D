#!/usr/bin/env python3
"""M455: characterize unmatched intervals in ordered SPR frame alignments.
Usage: python scripts/stafu-m455-unmatched-intervals.py m325-record-structure.txt games/STTNG games/STAFU-CD > m455-unmatched-intervals.txt
Read-only; prints summary statistics, not resource bytes.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,math,sys,zipfile
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
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):continue
            with zipfile.ZipFile(archive) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(archive)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous ZIP entries")
    raise FileNotFoundError(name)
def runs(a,b,minimum):
    out=[]
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift);ib=ia+shift;limit=min(len(a)-ia,len(b)-ib);k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:k+=1;continue
            start=k
            while k<limit and a[ia+k]==b[ib+k]:k+=1
            if k-start>=minimum:out.append((ia+start,ib+start,k-start))
    return sorted(out)
def chain(r):
    dp=[]
    for a,b,n in r:
        choices=[(n,((a,b,n),))]
        for score,items in dp:
            x,y,z=items[-1]
            if x+z<=a and y+z<=b:choices.append((score+n,items+((a,b,n),)))
        dp.append(max(choices,key=lambda t:(t[0],-len(t[1]),tuple((-x,-y,-z) for x,y,z in t[1]))))
    return max(dp,key=lambda t:(t[0],-len(t[1]),tuple((-x,-y,-z) for x,y,z in t[1])))[1] if dp else ()
def stats(b):
    if not b:return "empty=1"
    c=Counter(b);entropy=-sum((n/len(b))*math.log2(n/len(b)) for n in c.values())
    adjacent=sum(b[i]==b[i-1] for i in range(1,len(b)))
    longest=1;run=1
    for i in range(1,len(b)):
        run=run+1 if b[i]==b[i-1] else 1
        longest=max(longest,run)
    return f"distinct={len(c)} entropy={entropy:.3f} adjacentEqual={adjacent} longestRepeat={longest} zeroCount={c[0]} ffCount={c[255]}"
def compare(a,b):
    best=None
    for shift in range(-8,9):
        start=max(0,-shift);end=min(len(a),len(b)-shift)
        n=max(0,end-start)
        if n<8:continue
        eq=sum(a[k]==b[k+shift] for k in range(start,end))
        bits=sum(8-(a[k]^b[k+shift]).bit_count() for k in range(start,end))
        result=(eq/n,bits/(8*n),n,shift,eq,bits)
        if best is None or (result[0],result[1],result[2],-abs(shift))>(best[0],best[1],best[2],-abs(best[3])):best=result
    if best is None:return "aligned=insufficient"
    fraction,bitfrac,n,shift,eq,bits=best
    return f"bestShift={shift} compared={n} equalBytes={eq} equalPercent={100*fraction:.2f} equalBits={bits} bitPercent={100*bitfrac:.2f}"
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    p=Path(sys.argv[1])
    if not p.is_file():print("M455_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in read_report(p).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print("M455_ERROR duplicate_image",name,idx);return 2
        rows[name][idx]=r
    data={}
    for name in sorted({x[0] for x in TARGETS}):
        try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M455_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M455_ERROR unexpected_hash resource={name} sha256={digest}");return 2
        print(f"M455_RESOURCE resource={name} sha256={digest} source={source}")
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print("M455_ERROR bounds",name,idx);return 2
            data[name,idx]=raw[start:end]
    for name,i,j in TARGETS:
        a=data[name,i];b=data[name,j]
        for minimum in (8,16):
            matches=chain(runs(a,b,minimum))
            print(f"M455_PAIR resource={name} a={i} b={j} minRun={minimum} chainRuns={len(matches)}")
            pa=pb=0;totalA=totalB=0
            for k,(x,y,n) in enumerate(tuple(matches)+((len(a),len(b),0),),1):
                aa=a[pa:x];bb=b[pb:y]
                totalA+=len(aa);totalB+=len(bb)
                if aa or bb:
                    print(f"M455_GAP resource={name} a={i} b={j} minRun={minimum} index={k} startA={pa} startB={pb} bytesA={len(aa)} bytesB={len(bb)} delta={len(bb)-len(aa)} {compare(aa,bb)}")
                    print(f"M455_STATS resource={name} a={i} b={j} minRun={minimum} index={k} side=A {stats(aa)}")
                    print(f"M455_STATS resource={name} a={i} b={j} minRun={minimum} index={k} side=B {stats(bb)}")
                pa=x+n;pb=y+n
            print(f"M455_CHECK resource={name} a={i} b={j} minRun={minimum} unmatchedA={totalA} unmatchedB={totalB} lengthDelta={len(b)-len(a)} conserved={int(totalB-totalA==len(b)-len(a))}")
    print("M455_SUMMARY pairs=5 thresholds=8,16 decoderConfirmed=0 readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
