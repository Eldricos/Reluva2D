#!/usr/bin/env python3
"""M456: whole SPR payload repeated-byte and segment comparison.
Usage: python scripts/stafu-m456-whole-payload-patterns.py m325-record-structure.txt games/STTNG games/STAFU-CD > m456-whole-payload-patterns.txt
Read-only. Emits aggregate statistics, never original resource bytes.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,math,sys,zipfile
TARGETS=[("BRDGDATA.SPR",1,4),("BRDGDATA.SPR",2,8),("BRDGPICA.SPR",6,7),("BRDGTROI.SPR",2,4),("BRDGTROI.SPR",5,8)]
HASHES={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}
def report(p):
    b=p.read_bytes()
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
        for zpath in archives:
            if not zpath.is_file() or not zipfile.is_zipfile(zpath):continue
            with zipfile.ZipFile(zpath) as z:
                found=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(zpath)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entries")
    raise FileNotFoundError(name)
def prefix(a,b):
    for k,(x,y) in enumerate(zip(a,b)):
        if x!=y:return k
    return min(len(a),len(b))
def suffix(a,b,pre):
    n=0
    while n<min(len(a),len(b))-pre and a[-1-n]==b[-1-n]:n+=1
    return n
def stats(data):
    if not data:return "bytes=0 distinct=0 entropy=NA adjacentEqual=0 longestRepeat=0 repeatRuns=0 zeroCount=0 ffCount=0"
    c=Counter(data);entropy=-sum((v/len(data))*math.log2(v/len(data)) for v in c.values())
    adjacent=sum(data[k]==data[k-1] for k in range(1,len(data)))
    longest=1;current=1;runs=0
    for k in range(1,len(data)):
        if data[k]==data[k-1]:current+=1
        else:
            if current>1:runs+=1
            longest=max(longest,current);current=1
    if current>1:runs+=1
    longest=max(longest,current)
    return f"bytes={len(data)} distinct={len(c)} entropy={entropy:.3f} adjacentEqual={adjacent} longestRepeat={longest} repeatRuns={runs} zeroCount={c[0]} ffCount={c[255]}"
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    p=Path(sys.argv[1])
    if not p.is_file():print("M456_ERROR missing_report");return 2
    rows=defaultdict(dict)
    for line in report(p).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        r=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
        if r.get("kind")!="direct":continue
        name=r["resource"].upper();idx=int(r["ordinal"],0)
        if idx in rows[name]:print(f"M456_ERROR duplicate resource={name} ordinal={idx}");return 2
        rows[name][idx]=r
    data={}
    for name in sorted({t[0] for t in TARGETS}):
        try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M456_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=HASHES[name]:print(f"M456_ERROR hash resource={name} sha256={digest}");return 2
        print(f"M456_RESOURCE resource={name} sha256={digest} source={source}")
        for idx,r in rows[name].items():
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M456_ERROR bounds resource={name} ordinal={idx}");return 2
            data[name,idx]=raw[start:end]
    for name in sorted({t[0] for t in TARGETS}):
        ordinals=sorted(rows[name])
        print(f"M456_SET resource={name} directFrames={len(ordinals)}")
        for idx in ordinals:
            a=data[name,idx]
            print(f"M456_FRAME resource={name} ordinal={idx} {stats(a)}")
    for name,i,j in TARGETS:
        a=data[name,i];b=data[name,j];pre=prefix(a,b);suf=suffix(a,b,pre)
        print(f"M456_PAIR resource={name} a={i} b={j} prefix={pre} suffix={suf} middleA={len(a)-pre-suf} middleB={len(b)-pre-suf}")
        for label,aa,bb in (("prefix",a[:pre],b[:pre]),("middle",a[pre:len(a)-suf if suf else len(a)],b[pre:len(b)-suf if suf else len(b)]),("suffix",a[len(a)-suf:] if suf else b"",b[len(b)-suf:] if suf else b"")):
            print(f"M456_SEGMENT resource={name} a={i} b={j} segment={label} side=A {stats(aa)}")
            print(f"M456_SEGMENT resource={name} a={i} b={j} segment={label} side=B {stats(bb)}")
        print(f"M456_CHECK resource={name} a={i} b={j} reconstructA={pre+(len(a)-pre-suf)+suf} reconstructB={pre+(len(b)-pre-suf)+suf} valid={int(pre+suf<=min(len(a),len(b)))}")
    print("M456_SUMMARY readOnly=1 decoderConfirmed=0")
    return 0
if __name__=="__main__":raise SystemExit(main())
