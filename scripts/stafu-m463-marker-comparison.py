#!/usr/bin/env python3
"""M463: marker-controlled cross-resource SPR exact-match comparison.
Usage: python scripts/stafu-m463-marker-comparison.py m325-record-structure.txt games/STTNG games/STAFU-CD > m463-marker-comparison.txt
Read-only byte-level diagnostics. Cross-resource matching is confounded by image
content/dimensions; no codec interpretation is made.
"""
from pathlib import Path
from collections import Counter
import hashlib,itertools,sys,zipfile,statistics
NAMES=("BRDGDATA.SPR","BRDGPICA.SPR","BRDGTROI.SPR")
SHA={"BRDGDATA.SPR":"0ac490081b394b094b9c445b3081ae881c3b919328295e5fed9392264da9a4f9","BRDGPICA.SPR":"59d3a864d115be6f6d57a52ae19c06a5174b1d1d695494706d012e26f8f5f360","BRDGTROI.SPR":"664fe7a00169223719172bc7e0db482743229ea7e26ac512f0f4f0d2674bb3a7"}
THRESHOLDS=(2,3,4,6,8,16)
def text_report(path):
    b=path.read_bytes()
    if b.startswith((b"\xff\xfe",b"\xfe\xff")):return b.decode("utf-16")
    return b.decode("utf-16-le" if b[:256].count(b"\0")>32 else "utf-8-sig")
def load(name,roots):
    for root in roots:
        if root.is_dir():
            for p in (root/name,root/name.lower(),root/"STTNG"/name):
                if p.is_file():return p.read_bytes(),str(p)
            zips=(root/"STTNG.ZIP",root/"cd"/"STTNG.ZIP")
        elif root.is_file() and zipfile.is_zipfile(root):zips=(root,)
        else:continue
        for zp in zips:
            if not zp.is_file() or not zipfile.is_zipfile(zp):continue
            with zipfile.ZipFile(zp) as z:
                found=[entry for entry in z.namelist() if entry.replace("\\","/").split("/")[-1].upper()==name]
                if len(found)==1:return z.read(found[0]),str(zp)+":"+found[0]
                if len(found)>1:raise ValueError("ambiguous archive entry "+name)
    raise FileNotFoundError(name)
def runs(a,b):
    # Every maximal exact diagonal run >=2; no best-alignment assumption.
    result=[]
    for shift in range(1-len(a),len(b)):
        ia=max(0,-shift);ib=ia+shift;limit=min(len(a)-ia,len(b)-ib);k=0
        while k<limit:
            if a[ia+k]!=b[ib+k]:k+=1;continue
            begin=k
            while k<limit and a[ia+k]==b[ib+k]:k+=1
            if k-begin>=2:result.append((ia+begin,ib+begin,k-begin))
    return result
def coverage(a,b,rr,t):
    ma=bytearray(len(a));mb=bytearray(len(b))
    for x,y,n in rr:
        if n>=t:ma[x:x+n]=b"\x01"*n;mb[y:y+n]=b"\x01"*n
    return sum(ma),sum(mb)
def main():
    if len(sys.argv)<3:print(__doc__.strip());return 2
    report=Path(sys.argv[1])
    if not report.is_file():print("M463_ERROR missing_report");return 2
    rows={}
    for line in text_report(report).splitlines():
        if not line.startswith("SPR_M325_IMAGE "):continue
        fields=dict(s.split("=",1) for s in line.split()[1:] if "=" in s)
        name=fields.get("resource","").upper()
        if name not in NAMES or fields.get("kind")!="direct":continue
        ordinal=int(fields["ordinal"],0)
        key=(name,ordinal)
        if key in rows:print(f"M463_ERROR duplicate={key}");return 2
        rows[key]=fields
    roots=[Path(p) for p in sys.argv[2:]]
    frames=[]
    for name in NAMES:
        try:raw,source=load(name,roots)
        except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M463_ERROR resource={name} detail={e}");return 2
        digest=hashlib.sha256(raw).hexdigest()
        if digest!=SHA[name]:print(f"M463_ERROR resource={name} sha256={digest} expected={SHA[name]}");return 2
        ordinals=sorted(i for n,i in rows if n==name)
        if ordinals!=list(range(1,10)):print(f"M463_ERROR resource={name} expected_frames_1_to_9 actual={ordinals}");return 2
        print(f"M463_RESOURCE resource={name} sha256={digest} source={source}")
        for i in ordinals:
            r=rows[name,i]
            start=int(r["dataOffset"],0);size=int(r["dataBytes"],0);end=int(r["chunkEnd"],0)
            if start<0 or size<=0 or start+size!=end or end>len(raw):print(f"M463_ERROR bounds={name}:{i}");return 2
            data=raw[start:end]
            frame={"name":name,"ordinal":i,"marker":r.get("marker","NA"),"width":int(r["headerA"],0),"height":int(r["headerB"],0),"data":data}
            frames.append(frame)
            print(f"M463_FRAME resource={name} frame={i} marker={frame['marker']} width={frame['width']} height={frame['height']} bytes={len(data)} prefix8={data[:8].hex()} suffix8={data[-8:].hex()}")
    grouped={}
    total=0
    for a,b in itertools.combinations(frames,2):
        rr=runs(a["data"],b["data"])
        same_marker=a["marker"]==b["marker"]
        same_resource=a["name"]==b["name"]
        category=("within" if same_resource else "cross")+"_"+("same" if same_marker else "different")
        counts={t:sum(n>=t for _,_,n in rr) for t in THRESHOLDS}
        cov6=coverage(a["data"],b["data"],rr,6)
        prefix=0
        while prefix<min(len(a["data"]),len(b["data"])) and a["data"][prefix]==b["data"][prefix]:prefix+=1
        longest=max((n for _,_,n in rr),default=0)
        grouped.setdefault(category,[]).append((longest,prefix,counts,cov6,len(a["data"]),len(b["data"])))
        total+=1
        print(f"M463_PAIR category={category} a={a['name']}:{a['ordinal']} b={b['name']}:{b['ordinal']} markerA={a['marker']} markerB={b['marker']} bytesA={len(a['data'])} bytesB={len(b['data'])} prefix={prefix} maxRun={longest} runs2={counts[2]} runs3={counts[3]} runs4={counts[4]} runs6={counts[6]} runs8={counts[8]} runs16={counts[16]} coverage6A={cov6[0]} coverage6B={cov6[1]}")
    for category,items in sorted(grouped.items()):
        def mean(xs):return statistics.mean(xs) if xs else 0
        print(f"M463_GROUP category={category} pairs={len(items)} meanLongest={mean([x[0] for x in items]):.3f} meanPrefix={mean([x[1] for x in items]):.3f} pairsRun3={sum(x[2][3]>0 for x in items)} pairsRun6={sum(x[2][6]>0 for x in items)} meanCoverage6A={mean([x[3][0]/x[4] for x in items]):.4f} meanCoverage6B={mean([x[3][1]/x[5] for x in items]):.4f}")
    print(f"M463_SUMMARY frames={len(frames)} pairs={total} groups={len(grouped)} thresholds={','.join(map(str,THRESHOLDS))} readOnly=1 decoderConfirmed=0")
    print("M463_NOTE crossResourceComparisonsConfoundedByImageContentAndDimensions=1 sameMarkerNotProofOfSameCodec=1 exactMatchNotOpcode=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
