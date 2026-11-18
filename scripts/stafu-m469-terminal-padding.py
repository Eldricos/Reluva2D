#!/usr/bin/env python3
"""STAFU M469: terminal-bit padding hypotheses and suffix endpoints.
Usage: python scripts/stafu-m469-terminal-padding.py m325-record-structure.txt games/STTNG games/STAFU-CD > m469-terminal-padding.txt
Read-only. Padding candidates are hypotheses, not verified format semantics.
"""
from pathlib import Path
from collections import defaultdict
from itertools import combinations
import hashlib,sys,zipfile
T="BRDGTROI.SPR";W="BRDGWORF.SPR"
def textfile(p):
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
    if len(hits)>1:raise ValueError("ambiguous entry "+name)
    if hits:return z.read(hits[0]),str(p)+":"+hits[0]
 raise FileNotFoundError(name)
def bits(data):return ''.join(format(x,'08b') for x in data)
def common_suffix(a,b):
 i=0;limit=min(len(a),len(b))
 while i<limit and a[-1-i]==b[-1-i]:i+=1
 return i
def common_prefix(a,b):
 i=0;limit=min(len(a),len(b))
 while i<limit and a[i]==b[i]:i+=1
 return i
def main():
 if len(sys.argv)<3:print(__doc__.strip());return 2
 p=Path(sys.argv[1])
 if not p.is_file():print("M469_ERROR missing_report");return 2
 rows=defaultdict(list)
 for line in textfile(p).splitlines():
  if not line.startswith("SPR_M325_IMAGE "):continue
  d=dict(x.split("=",1) for x in line.split()[1:] if "=" in x)
  if d.get("resource","").upper() in (T,W) and d.get("kind")=="direct":rows[d["resource"].upper()].append(d)
 frames={}
 for name in (T,W):
  try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
  except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M469_ERROR resource={name} detail={e}");return 2
  print(f"M469_RESOURCE resource={name} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
  if not rows[name]:print(f"M469_ERROR no_frames={name}");return 2
  for d in rows[name]:
   i=int(d["ordinal"],0);s=int(d["dataOffset"],0);n=int(d["dataBytes"],0);e=int(d["chunkEnd"],0)
   if s<0 or n<=0 or s+n!=e or e>len(raw) or (name,i) in frames:
    print(f"M469_ERROR invalid_frame={name}:{i}");return 2
   frames[name,i]=(bits(raw[s:e]),s,raw[s:e])
 for name,ids in ((T,range(1,10)),(W,range(1,9))):
  if any((name,i) not in frames for i in ids):print(f"M469_ERROR missing_frames={name}");return 2
 for name,ids in ((T,range(1,10)),(W,range(1,9))):
  for i in ids:
   b,base,raw=frames[name,i]
   print(f"M469_TERMINAL resource={name} frame={i} markerGroup={'troi1' if name==T and i!=6 else 'troi6' if name==T else 'worf2'} bytes={len(raw)} lastByte=0x{raw[-1]:02x} final32={b[-32:]} final16={b[-16:]} fileEnd={base+len(raw)}")
 ids=[1,2,3,4,5,7,8,9]
 pairs=list(combinations(ids,2))
 # Every candidate pair of terminal trim lengths is evaluated independently.
 # Note that trimming changes the effective end point, not the underlying bits.
 scores={}
 for a,b in pairs:
  A=frames[T,a][0];B=frames[T,b][0]
  options=[]
  for pa in range(8):
   for pb in range(8):
    aa=A[:-pa] if pa else A;bb=B[:-pb] if pb else B
    s=common_suffix(aa,bb)
    options.append((s,pa,pb))
  best=max(x[0] for x in options)
  winners=sorted((pa,pb) for s,pa,pb in options if s==best)
  baseline=common_suffix(A,B)
  scores[a,b]={(pa,pb):s for s,pa,pb in options}
  # Show all ties to avoid arbitrarily implying a unique padding convention.
  print(f"M469_PAIR a={a} b={b} rawSuffix={baseline} bestTrimmedSuffix={best} improvement={best-baseline} bestAssignments={';'.join(str(pa)+','+str(pb) for pa,pb in winners)} bestCount={len(winners)}")
  for pa,pb in winners[:8]:
   aa=A[:-pa] if pa else A;bb=B[:-pb] if pb else B
   s=common_suffix(aa,bb)
   print(f"M469_ENDPOINT a={a} b={b} trimA={pa} trimB={pb} suffixBits={s} startA={len(aa)-s} startB={len(bb)-s} endA={len(aa)} endB={len(bb)} preA={aa[len(aa)-s-1] if len(aa)>s else '-'} preB={bb[len(bb)-s-1] if len(bb)>s else '-'}")
 # Global consistency: choose a single padding length per frame, maximizing
 # pairwise suffix score, with tie-breaking favoring fewer total trimmed bits.
 # Coordinate ascent from several deterministic starts; not guaranteed global optimum.
 def evaluate(assign):
  return sum(scores[a,b][assign[a],assign[b]] for a,b in pairs)
 candidates=[]
 for seed in range(9):
  assign={i:(seed if seed<8 else 0) for i in ids}
  for iteration in range(40):
   changed=False
   for i in ids:
    previous=assign[i]
    opts=[]
    for pad in range(8):
     assign[i]=pad
     opts.append((evaluate(assign),-sum(assign.values()),-pad,pad))
    assign[i]=max(opts)[-1]
    if assign[i]!=previous:changed=True
   if not changed:break
  candidates.append((evaluate(assign),-sum(assign.values()),tuple(assign[i] for i in ids)))
 bestscore=max(x[0] for x in candidates)
 bests=sorted(set(x[2] for x in candidates if x[0]==bestscore))
 baseline=sum(scores[a,b][0,0] for a,b in pairs)
 print(f"M469_MODEL baselineSumSuffix={baseline} bestFoundSumSuffix={bestscore} improvement={bestscore-baseline} starts=9 localOptimum=1 globallyProven=0")
 for assignment in bests:
  assign=dict(zip(ids,assignment))
  longpairs=[f"{a}-{b}:{scores[a,b][assign[a],assign[b]]}" for a,b in pairs if scores[a,b][assign[a],assign[b]]>=64]
  print(f"M469_ASSIGNMENT trims={','.join(str(i)+':'+str(assign[i]) for i in ids)} longPairs={','.join(longpairs) if longpairs else '-'}")
 # Check how much pairwise suffix improves with independently chosen trims for controls.
 for kind,keys in (("troi6",[(T,6,T,i) for i in ids]),("worf",[(T,6,W,i) for i in range(1,9)])):
  for na,ia,nb,ib in keys:
   A=frames[na,ia][0];B=frames[nb,ib][0]
   baseline=common_suffix(A,B)
   best=max(common_suffix(A[:-pa] if pa else A,B[:-pb] if pb else B) for pa in range(8) for pb in range(8))
   print(f"M469_CONTROL group={kind} a={ia} b={ib} rawSuffix={baseline} bestTrimmedSuffix={best}")
 print("M469_SUMMARY troiPairs=28 controlPairs=16 paddingCandidatesPerPair=64 readOnly=1 codecConfirmed=0")
 print("M469_NOTE candidatePaddingUnverified=1 noMeaningfulBitCountKnown=1 pairwiseBestCanOverfit=1 globalModelCoordinateAscentNotExhaustive=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
