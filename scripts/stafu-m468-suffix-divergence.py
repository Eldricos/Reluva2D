#!/usr/bin/env python3
"""STAFU M468: conserved bitstream suffixes and divergence points.
Usage: python scripts/stafu-m468-suffix-divergence.py m325-record-structure.txt games/STTNG games/STAFU-CD > m468-suffix-divergence.txt
Read-only observational diagnostics; no decoder or codec assumptions.
"""
from pathlib import Path
from collections import defaultdict,Counter
import hashlib,sys,zipfile

TROI="BRDGTROI.SPR";WORF="BRDGWORF.SPR"
def report_text(p):
 b=p.read_bytes()
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
    hits=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
    if len(hits)>1:raise ValueError("ambiguous archive entry "+name)
    if hits:return z.read(hits[0]),str(p)+":"+hits[0]
 raise FileNotFoundError(name)
def bits(data):
 return bytes((x>>k)&1 for x in data for k in range(7,-1,-1))
def prefix(a,b):
 i=0
 while i<min(len(a),len(b)) and a[i]==b[i]:i+=1
 return i
def suffix(a,b):
 i=0
 while i<min(len(a),len(b)) and a[-1-i]==b[-1-i]:i+=1
 return i
def sam(a):
 nxt=[{}];links=[-1];length=[0];first=[-1];last=0
 for i,c in enumerate(a):
  cur=len(nxt);nxt.append({});length.append(length[last]+1);links.append(0);first.append(i)
  p=last
  while p>=0 and c not in nxt[p]:
   nxt[p][c]=cur;p=links[p]
  if p>=0:
   q=nxt[p][c]
   if length[p]+1==length[q]:links[cur]=q
   else:
    clone=len(nxt);nxt.append(nxt[q].copy());length.append(length[p]+1);links.append(links[q]);first.append(first[q])
    while p>=0 and nxt[p].get(c)==q:
     nxt[p][c]=clone;p=links[p]
    links[q]=links[cur]=clone
  last=cur
 return nxt,links,length,first
def longest(a,b):
 if not a or not b:return (0,0,0)
 nxt,links,length,first=sam(a);state=0;size=0;best=(0,0,0)
 for j,c in enumerate(b):
  while state and c not in nxt[state]:state=links[state];size=length[state]
  if c in nxt[state]:state=nxt[state][c];size+=1
  else:state=0;size=0
  if size>best[0]:best=(size,first[state]-size+1,j-size+1)
 return best
def inspect(name,indices,frames):
 data=[frames[name,i]["data"] for i in indices]
 return min(prefix(data[0],d) for d in data[1:])
def main():
 if len(sys.argv)<3:print(__doc__.strip());return 2
 p=Path(sys.argv[1])
 if not p.is_file():print("M468_ERROR missing_report");return 2
 rows=defaultdict(list)
 for line in report_text(p).splitlines():
  if not line.startswith("SPR_M325_IMAGE "):continue
  d=dict(x.split("=",1) for x in line.split()[1:] if "=" in x)
  name=d.get("resource","").upper()
  if name in (TROI,WORF) and d.get("kind")=="direct":rows[name].append(d)
 frames={}
 for name in (TROI,WORF):
  try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
  except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M468_ERROR resource={name} detail={e}");return 2
  print(f"M468_RESOURCE resource={name} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
  if not rows[name]:print(f"M468_ERROR no_frames resource={name}");return 2
  for d in rows[name]:
   i=int(d["ordinal"],0);start=int(d["dataOffset"],0);n=int(d["dataBytes"],0);end=int(d["chunkEnd"],0)
   if start<0 or n<=0 or start+n!=end or end>len(raw) or (name,i) in frames:
    print(f"M468_ERROR invalid_frame={name}:{i}");return 2
   frames[name,i]={"data":raw[start:end],"base":start,"marker":d.get("marker","NA")}
 for name,ids in ((TROI,range(1,10)),(WORF,range(1,9))):
  missing=[i for i in ids if (name,i) not in frames]
  if missing:print(f"M468_ERROR missing_frames={name}:{missing}");return 2
 cp_t=inspect(TROI,[1,2,3,4,5,7,8,9],frames)
 cp_w=inspect(WORF,list(range(1,9)),frames)
 print(f"M468_PREFIX troiMarker1={cp_t} worfMarker2={cp_w}")
 pairs=[("troi",(TROI,a),(TROI,b)) for a in range(1,10) if a!=6 for b in range(a+1,10) if b!=6]
 pairs +=[("troi6",(TROI,6),(TROI,i)) for i in range(1,10) if i!=6]
 pairs +=[("worf",(TROI,6),(WORF,i)) for i in range(1,9)]
 groups=defaultdict(list)
 for kind,ka,kb in pairs:
  a=frames[ka]["data"];b=frames[kb]["data"]
  A=bits(a);B=bits(b)
  # The suffix is anchored to the *actual final bit* of each payload.
  raw_s=suffix(A,B)
  # This independent longest-common-substring calculation is unrestricted.
  n,ia,ib=longest(A,B)
  # Prefix-removed suffix is included only as a control; suffix must not change.
  oa=cp_t if ka[0]==TROI and ka[1]!=6 else 0
  ob=cp_t if kb[0]==TROI and kb[1]!=6 else cp_w if kb[0]==WORF else 0
  trimmed_s=suffix(A[oa*8:],B[ob*8:])
  suffix_start_a=len(A)-raw_s;suffix_start_b=len(B)-raw_s
  end_gap_a=len(A)-(ia+n);end_gap_b=len(B)-(ib+n)
  delta=ib-ia
  print(f"M468_PAIR group={kind} a={ka[0]}:{ka[1]} b={kb[0]}:{kb[1]} bytesA={len(a)} bytesB={len(b)} suffixBits={raw_s} suffixStartA={suffix_start_a} suffixStartB={suffix_start_b} suffixPhaseA={suffix_start_a%8} suffixPhaseB={suffix_start_b%8} suffixBytesFloor={raw_s//8} trimmedSuffixBits={trimmed_s} globalBits={n} globalStartA={ia} globalStartB={ib} globalEndGapA={end_gap_a} globalEndGapB={end_gap_b} deltaBits={delta} deltaModulo8={delta%8} globalIsSuffix={int(end_gap_a==0 and end_gap_b==0)}")
  if kind=="troi":
   groups["troi"].append((ka[1],kb[1],raw_s,n,delta%8,end_gap_a,end_gap_b))
   if raw_s:
    # The first mismatching bit immediately before a common suffix is
    # at index suffixStart-1, if both payloads contain that bit.
    pre_a=A[suffix_start_a-1] if suffix_start_a>0 else -1
    pre_b=B[suffix_start_b-1] if suffix_start_b>0 else -1
    print(f"M468_BOUNDARY a={ka[1]} b={kb[1]} suffixBits={raw_s} preBitA={pre_a} preBitB={pre_b} startAByte={suffix_start_a//8} startBByte={suffix_start_b//8} startAPhase={suffix_start_a%8} startBPhase={suffix_start_b%8} fileByteA={frames[ka]['base']+suffix_start_a//8} fileByteB={frames[kb]['base']+suffix_start_b//8} pre8A={A[max(0,suffix_start_a-8):suffix_start_a].hex()} pre8B={B[max(0,suffix_start_b-8):suffix_start_b].hex()}")
 vals=groups["troi"]
 print(f"M468_GROUP troiPairs={len(vals)} suffixAtLeast64={sum(v[2]>=64 for v in vals)} globalAtLeast64={sum(v[3]>=64 for v in vals)} globalIsSuffix={sum(v[5]==0 and v[6]==0 for v in vals)} nonByteGlobal={sum(v[4]!=0 for v in vals)}")
 for mod in range(8):
  selected=[f"{a}-{b}" for a,b,s,n,m,ea,eb in vals if m==mod and n>=64]
  print(f"M468_ALIGNMENT deltaModulo8={mod} pairs={len(selected)} members={','.join(selected) if selected else '-'}")
 print(f"M468_SUMMARY pairs={len(pairs)} troiPairs={len(vals)} troi6Pairs=8 worfPairs=8 readOnly=1 codecConfirmed=0")
 print("M468_NOTE suffixIsEndAnchored=1 globalIsUnrestricted=1 noPaddingBitCountKnown=1 trailingPaddingCouldAffectSuffix=1 boundaryNotCodecCommand=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
