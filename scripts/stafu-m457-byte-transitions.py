#!/usr/bin/env python3
"""M457: adjacent-byte transition distributions in direct SPR image payloads.
Usage: python scripts/stafu-m457-byte-transitions.py m325-record-structure.txt games/STTNG games/STAFU-CD > m457-byte-transitions.txt
Read-only; emits aggregate counts only, not original game bytes.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,math,sys,zipfile
TARGETS=[("BRDGDATA.SPR",1,4),("BRDGDATA.SPR",2,8),("BRDGPICA.SPR",6,7),("BRDGTROI.SPR",2,4),("BRDGTROI.SPR",5,8)]
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}
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
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):continue
            with zipfile.ZipFile(archive) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(archive)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entries")
    raise FileNotFoundError(name)
def prefix(a,b):
    for k,(x,y) in enumerate(zip(a,b)):
        if x!=y:return k
    return min(len(a),len(b))
def suffix(a,b,p):
    n=0
    while n<min(len(a),len(b))-p and a[-1-n]==b[-1-n]:n+=1
    return n
def transitions(b):
    diff=Counter();xor=Counter();signed=Counter()
    for x,y in zip(b,b[1:]):
        d=(y-x)&255
        diff[d]+=1;xor[x^y]+=1;signed[d if d<=127 else d-256]+=1
    return diff,xor,signed
def entropy(c):
    total=sum(c.values())
    return -sum((n/total)*math.log2(n/total) for n in c.values()) if total else 0.
def summarize(label,b):
    d,x,s=transitions(b);n=sum(d.values())
    small=sum(v for k,v in s.items() if -4<=k<=4)
    near=sum(v for k,v in s.items() if -16<=k<=16)
    topD=",".join(f"{k}:{v}" for k,v in sorted(d.items(),key=lambda kv:(-kv[1],kv[0]))[:8]) or "none"
    topX=",".join(f"{k}:{v}" for k,v in sorted(x.items(),key=lambda kv:(-kv[1],kv[0]))[:8]) or "none"
    missingD=sum(d[k]==0 for k in range(256))
    missingX=sum(x[k]==0 for k in range(256))
    print(f"M457_TRANSITION {label} bytes={len(b)} transitions={n} equal={d[0]} diffDistinct={len(d)} xorDistinct={len(x)} diffMissing={missingD} xorMissing={missingX} diffEntropy={entropy(d):.3f} xorEntropy={entropy(x):.3f} signedWithin4={small} signedWithin16={near} diffTop={topD} xorTop={topX}")
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    path=Path(sys.argv[1])
    if not path.is_file():print("M457_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in read_report(path).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print(f"M457_ERROR duplicate resource={name} ordinal={idx}");return 2
        rows[name][idx]=r
    data={}
    for name in sorted({x[0] for x in TARGETS}):
        try:raw,source=locate(name,[Path(p) for p in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M457_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M457_ERROR hash resource={name} sha256={digest}");return 2
        print(f"M457_RESOURCE resource={name} sha256={digest} source={source}")
        if not rows[name]:print(f"M457_ERROR no_direct_images resource={name}");return 2
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M457_ERROR bounds resource={name} ordinal={idx}");return 2
            data[name,idx]=raw[start:end]
    for name in sorted({t[0] for t in TARGETS}):
        combinedD=Counter();combinedX=Counter();total=0
        for idx in sorted(rows[name]):
            b=data[name,idx];summarize(f"resource={name} ordinal={idx} segment=whole",b)
            d,x,_=transitions(b);combinedD.update(d);combinedX.update(x);total+=len(b)-1
        print(f"M457_AGGREGATE resource={name} frames={len(rows[name])} transitions={total} equal={combinedD[0]} diffDistinct={len(combinedD)} xorDistinct={len(combinedX)} diffEntropy={entropy(combinedD):.3f} xorEntropy={entropy(combinedX):.3f} diffMissing={256-len(combinedD)} xorMissing={256-len(combinedX)}")
    for name,i,j in TARGETS:
        a=data[name,i];b=data[name,j];p=prefix(a,b);s=suffix(a,b,p)
        print(f"M457_PAIR resource={name} a={i} b={j} prefix={p} suffix={s} middleA={len(a)-p-s} middleB={len(b)-p-s}")
        for side,blob in (("A",a),("B",b)):
            end=len(blob)-s
            for seg,part in (("prefix",blob[:p]),("middle",blob[p:end]),("suffix",blob[end:])):
                summarize(f"resource={name} a={i} b={j} side={side} segment={seg}",part)
        print(f"M457_CHECK resource={name} a={i} b={j} valid={int(p+s<=min(len(a),len(b)))}")
    print("M457_SUMMARY frames=27 pairs=5 readOnly=1 decoderConfirmed=0")
    return 0
if __name__=="__main__":raise SystemExit(main())
