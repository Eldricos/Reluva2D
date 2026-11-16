#!/usr/bin/env python3
"""M452: cross-resource direct-image prefix and post-divergence matches.

Usage:
 python scripts/stafu-m452-cross-resource-prefixes.py m325-record-structure.txt games/STTNG games/STAFU-CD > m452-cross-resource-prefixes.txt

Read-only, prints aggregate statistics and offsets, never raw original bytes.
Tests are observational: matching boundaries do not establish codec commands.
"""
from __future__ import annotations
from collections import Counter, defaultdict
from itertools import combinations
from pathlib import Path
import hashlib
import sys
import zipfile

NAMES = ("BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR","BRDGTROI.SPR","BRDGWORF.SPR")
KNOWN = {"BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7",
         "BRDGWORF.SPR":"9ed0424d1cfda53d5237f0dab5f2e407260732604b22d9e53f80a56e3ac8c975"}

def report_text(path):
    raw=path.read_bytes()
    if raw.startswith((b"\xff\xfe",b"\xfe\xff")):
        return raw.decode("utf-16")
    if raw[:256].count(b"\0")>32:
        return raw.decode("utf-16-le")
    return raw.decode("utf-8-sig")

def locate(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file():
                    return p.read_bytes(),str(p)
            archives=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):
            archives=(root,)
        else:
            continue
        for archive in archives:
            if not archive.is_file() or not zipfile.is_zipfile(archive):
                continue
            with zipfile.ZipFile(archive) as zf:
                matches=[entry for entry in zf.namelist() if entry.replace("\\","/").split("/")[-1].upper()==name]
                if len(matches)==1:
                    return zf.read(matches[0]),str(archive)+":"+matches[0]
                if len(matches)>1:
                    raise ValueError(f"ambiguous_zip_entries={name}")
    raise FileNotFoundError(name)

def common_prefix(a,b):
    for i,(x,y) in enumerate(zip(a,b)):
        if x!=y:
            return i
    return min(len(a),len(b))

def longest_match(a,b):
    prev=[0]*(len(b)+1)
    best=(0,0,0)
    for i,x in enumerate(a):
        curr=[0]*(len(b)+1)
        for j,y in enumerate(b):
            if x==y:
                n=prev[j]+1
                curr[j+1]=n
                if n>best[0]:
                    best=(n,i-n+1,j-n+1)
        prev=curr
    return best

def main():
    if len(sys.argv)<3:
        print(__doc__.strip())
        return 2
    report=Path(sys.argv[1])
    if not report.is_file():
        print(f"M452_ERROR missing_report={report}")
        return 2
    groups=defaultdict(dict)
    for line in report_text(report).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):
            continue
        row=dict(token.split("=",1) for token in line.split()[1:] if "=" in token)
        name=row.get("resource","").upper()
        if name not in NAMES or row.get("kind")!="direct":
            continue
        ordinal=int(row["ordinal"],0)
        if ordinal in groups[name]:
            print(f"M452_ERROR duplicate_ordinal resource={name} ordinal={ordinal}")
            return 2
        groups[name][ordinal]=row
    roots=[Path(s) for s in sys.argv[2:]]
    grand=[]
    print("M452_INPUT source=M325 resources=5 readOnly=1")
    for name in NAMES:
        try:
            raw,source=locate(name,roots)
        except (OSError,ValueError,zipfile.BadZipFile) as exc:
            print(f"M452_ERROR resource={name} detail={exc}")
            return 2
        digest=hashlib.sha256(raw).hexdigest()
        if name in KNOWN and digest!=KNOWN[name]:
            print(f"M452_ERROR resource={name} unexpected_sha256={digest}")
            return 2
        entries=groups[name]
        if len(entries)<2:
            print(f"M452_ERROR resource={name} insufficient_direct_images={len(entries)}")
            return 2
        payloads={}
        for ordinal,row in sorted(entries.items()):
            start=int(row["dataOffset"],0)
            length=int(row["dataBytes"],0)
            end=int(row["chunkEnd"],0)
            if start<0 or length<=0 or start+length!=end or end>len(raw):
                print(f"M452_ERROR resource={name} invalid_bounds_ordinal={ordinal}")
                return 2
            payloads[ordinal]=raw[start:end]
        print(f"M452_RESOURCE resource={name} bytes={len(raw)} sha256={digest} source={source} directImages={len(entries)} dimensions={','.join(sorted(set(r['headerA']+'x'+r['headerB'] for r in entries.values())))} markers={','.join(sorted(set(r['marker'] for r in entries.values())))}")
        results=[]
        for i,j in combinations(sorted(payloads),2):
            a,b=payloads[i],payloads[j]
            p=common_prefix(a,b)
            run,oa,ob=longest_match(a,b)
            aligned=sum(x==y for x,y in zip(a,b))
            same=entries[i]["marker"]==entries[j]["marker"]
            # Only a post-divergence match if both run starts are beyond prefix.
            post=run>=16 and oa>=p and ob>=p and (oa>p or ob>p)
            item=dict(resource=name,a=i,b=j,prefix=p,run=run,offsetA=oa,offsetB=ob,shift=ob-oa,sameMarker=int(same),postMatch=int(post))
            results.append(item)
            grand.append(item)
            print("M452_PAIR "+" ".join(f"{k}={v}" for k,v in item.items())+f" alignedEqual={aligned}")
        prefix_hist=Counter(r["prefix"] for r in results)
        long_post=[r for r in results if r["postMatch"]]
        print(f"M452_DISTRIBUTION resource={name} "+" ".join(f"prefix{n}={count}" for n,count in sorted(prefix_hist.items())))
        for r in sorted(long_post,key=lambda x:(-x["run"],x["a"],x["b"]))[:12]:
            print(f"M452_POST resource={name} a={r['a']} b={r['b']} run={r['run']} offsetA={r['offsetA']} offsetB={r['offsetB']} shift={r['shift']} prefix={r['prefix']}")
        print(f"M452_SUMMARY_RESOURCE resource={name} pairs={len(results)} maxPrefix={max(r['prefix'] for r in results)} postMatches16={len(long_post)} sameMarkerPairs={sum(r['sameMarker'] for r in results)}")
    hist=Counter(r["prefix"] for r in grand)
    print("M452_GLOBAL_PREFIX "+" ".join(f"prefix{n}={c}" for n,c in sorted(hist.items())))
    print(f"M452_SUMMARY resources={len(NAMES)} pairs={len(grand)} postMatches16={sum(r['postMatch'] for r in grand)} codecBoundaryProven=0 decoderConfirmed=0")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
