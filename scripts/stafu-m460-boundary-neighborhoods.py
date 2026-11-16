#!/usr/bin/env python3
"""M460: SPR exact-match boundary neighborhoods (read-only).
Usage: python scripts/stafu-m460-boundary-neighborhoods.py m325-record-structure.txt games/STTNG games/STAFU-CD > m460-boundary-neighborhoods.txt
Reports local bytes and transitions; observations are NOT decoded opcodes.
"""
from collections import defaultdict,Counter
from pathlib import Path
import hashlib,sys,zipfile
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}
PAIRS=[("BRDGDATA.SPR",1,4),("BRDGDATA.SPR",2,8),("BRDGPICA.SPR",6,7),("BRDGTROI.SPR",2,4),("BRDGTROI.SPR",5,8),("BRDGTROI.SPR",3,6),("BRDGTROI.SPR",6,7)]
MIN_RUN=8;WINDOW=8
def report_text(path):
    b=path.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")):return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")
def locate(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file():return p.read_bytes(),str(p)
            archives=[root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP"]
        elif root.is_file() and zipfile.is_zipfile(root):archives=[root]
        else:continue
        for p in archives:
            if not p.is_file() or not zipfile.is_zipfile(p):continue
            with zipfile.ZipFile(p) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(p)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entries")
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
def chain(candidates):
    dp=[]
    for a,b,n in candidates:
        choices=[(n,((a,b,n),))]
        for score,items in dp:
            x,y,z=items[-1]
            if x+z<=a and y+z<=b:choices.append((score+n,items+((a,b,n),)))
        dp.append(max(choices,key=lambda t:(t[0],-len(t[1]),tuple((-x,-y,-z) for x,y,z in t[1]))))
    return max(dp,key=lambda t:(t[0],-len(t[1]),tuple((-x,-y,-z) for x,y,z in t[1])))[1] if dp else ()
def hexspan(blob,start,end):return blob[max(0,start):min(len(blob),end)].hex(" ") or "empty"
def local(blob,pos):
    lo=max(0,pos-WINDOW);hi=min(len(blob),pos+WINDOW)
    return f"offset={pos} before={hexspan(blob,lo,pos)} after={hexspan(blob,pos,hi)}"
def transition(blob,pos):
    if not 0<pos<len(blob):return "NA"
    return str((blob[pos]-blob[pos-1])&255)
def marker(rows,name,idx):return rows[name][idx].get("marker","unknown")
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    path=Path(sys.argv[1])
    if not path.is_file():print("M460_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in report_text(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(part.split("=",1) for part in line.split()[1:] if "=" in part)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print(f"M460_ERROR duplicate resource={name} ordinal={idx}");return 2
        rows[name][idx]=r
    payloads={}
    for name in HASHES:
        try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M460_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M460_ERROR hash resource={name} sha256={digest}");return 2
        if len(rows[name])!=9:print(f"M460_ERROR frames resource={name} actual={len(rows[name])}");return 2
        print(f"M460_RESOURCE resource={name} sha256={digest} source={source}")
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M460_ERROR bounds resource={name} ordinal={idx}");return 2
            payloads[name,idx]=raw[start:end]
    signatures=Counter();boundary_count=0
    for name,i,j in PAIRS:
        a=payloads[name,i];b=payloads[name,j];ma=marker(rows,name,i);mb=marker(rows,name,j)
        selected=chain(runs(a,b,MIN_RUN))
        print(f"M460_PAIR resource={name} a={i} b={j} markerA={ma} markerB={mb} bytesA={len(a)} bytesB={len(b)} matchedRuns={len(selected)} matchedBytes={sum(n for _,_,n in selected)}")
        if not selected:
            print(f"M460_NO_MATCH resource={name} a={i} b={j} threshold={MIN_RUN} noBoundaryInferred=1")
            continue
        for ri,(x,y,n) in enumerate(selected,1):
            print(f"M460_RUN resource={name} a={i} b={j} run={ri} startA={x} startB={y} length={n} shift={y-x}")
            for side,pa,pb in (("start",x,y),("end",x+n,y+n)):
                boundary_count+=1
                ta=transition(a,pa);tb=transition(b,pb)
                same=(ta==tb and ta!="NA")
                print(f"M460_BOUNDARY resource={name} a={i} b={j} run={ri} side={side} transitionA={ta} transitionB={tb} equalTransition={int(same)} A:{local(a,pa)} B:{local(b,pb)}")
                if ta!="NA" and tb!="NA":signatures[side,ta,tb]+=1
        previous_a=previous_b=0
        for gi,(x,y,n) in enumerate(selected,1):
            gap_a=a[previous_a:x];gap_b=b[previous_b:y]
            print(f"M460_GAP resource={name} a={i} b={j} gap={gi} startA={previous_a} endA={x} startB={previous_b} endB={y} bytesA={len(gap_a)} bytesB={len(gap_b)} firstA={hexspan(gap_a,0,4)} firstB={hexspan(gap_b,0,4)} lastA={hexspan(gap_a,max(0,len(gap_a)-4),len(gap_a))} lastB={hexspan(gap_b,max(0,len(gap_b)-4),len(gap_b))}")
            previous_a=x+n;previous_b=y+n
        print(f"M460_GAP resource={name} a={i} b={j} gap={len(selected)+1} startA={previous_a} endA={len(a)} startB={previous_b} endB={len(b)} bytesA={len(a)-previous_a} bytesB={len(b)-previous_b}")
    for (side,ta,tb),count in sorted(signatures.items(),key=lambda item:(-item[1],item[0]))[:20]:
        print(f"M460_TRANSITION_PATTERN side={side} transitionA={ta} transitionB={tb} count={count}")
    print(f"M460_SUMMARY pairs={len(PAIRS)} boundaries={boundary_count} minRun={MIN_RUN} window={WINDOW} readOnly=1 decoderConfirmed=0 boundariesAreExactMatchEdgesNotCodecCommands=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
