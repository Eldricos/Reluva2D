#!/usr/bin/env python3
"""M465: SPR bit-position, bit-pattern, and alignment diagnostics.
Usage: python scripts/stafu-m465-bitstream-phase.py m325-record-structure.txt games/STTNG games/STAFU-CD > m465-bitstream-phase.txt
Observational and read-only. No decoder or opcode interpretation.
"""
from collections import Counter,defaultdict
from pathlib import Path
import hashlib,sys,zipfile,math,itertools
NAMES=("BRDGTROI.SPR","BRDGWORF.SPR","BRDGDATA.SPR","BRDGPICA.SPR","BRDGRIKR.SPR")
def report_text(p):
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
  for p in archives:
   if not p.is_file() or not zipfile.is_zipfile(p):continue
   with zipfile.ZipFile(p) as z:
    hits=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
    if len(hits)>1:raise ValueError("ambiguous "+name)
    if hits:return z.read(hits[0]),str(p)+":"+hits[0]
 raise FileNotFoundError(name)
def bits(b):
 return [((v>>shift)&1) for v in b for shift in range(7,-1,-1)]
def patterns(seq,k):
 c=Counter()
 for i in range(len(seq)-k+1):
  x=0
  for bit in seq[i:i+k]:x=(x<<1)|bit
  c[x]+=1
 return c
def entropy(c):
 n=sum(c.values())
 return -sum((v/n)*math.log2(v/n) for v in c.values() if v) if n else 0.
def prefix(a,b):
 i=0
 while i<min(len(a),len(b)) and a[i]==b[i]:i+=1
 return i
def phase(a,b,max_shift=15):
 # Longest equal-bit run for each relative bit shift. Identical shared prefixes
 # must be excluded by the caller; large runs alone are not codec evidence.
 out=[]
 for shift in range(-max_shift,max_shift+1):
  ia=max(0,-shift);ib=ia+shift;length=min(len(a)-ia,len(b)-ib)
  best=0;cur=0;best_a=-1
  for k in range(max(0,length)):
   if a[ia+k]==b[ib+k]:
    cur+=1
    if cur>best:best=cur;best_a=ia+k-cur+1
   else:cur=0
  out.append((shift,best,best_a))
 return sorted(out,key=lambda t:(-t[1],abs(t[0]),t[0]))
def fmt(seq):return ",".join(f"{x:.4f}" for x in seq)
def main():
 if len(sys.argv)<3:print(__doc__.strip());return 2
 path=Path(sys.argv[1])
 if not path.is_file():print("M465_ERROR missing_report");return 2
 rows=defaultdict(list)
 for line in report_text(path).splitlines():
  if not line.startswith("SPR_M325_IMAGE "):continue
  d=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
  name=d.get("resource","").upper()
  if name in NAMES and d.get("kind")=="direct":rows[name].append(d)
 roots=[Path(x) for x in sys.argv[2:]]
 frames=[]
 for name in NAMES:
  try:raw,source=locate(name,roots)
  except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M465_ERROR resource={name} detail={e}");return 2
  print(f"M465_RESOURCE resource={name} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
  if not rows[name]:print(f"M465_ERROR no_frames resource={name}");return 2
  seen=set()
  for d in sorted(rows[name],key=lambda x:int(x["ordinal"],0)):
   i=int(d["ordinal"],0);start=int(d["dataOffset"],0);n=int(d["dataBytes"],0);end=int(d["chunkEnd"],0)
   if i in seen or start<0 or n<=0 or start+n!=end or end>len(raw):print(f"M465_ERROR frame_bounds={name}:{i}");return 2
   seen.add(i);blob=raw[start:end]
   frames.append(dict(name=name,i=i,marker=d.get("marker","NA"),w=d.get("headerA","NA"),h=d.get("headerB","NA"),data=blob))
 # Exclude the maximal shared prefix of all frames within each (resource,marker)
 # group. This isolates variable tails without assuming any codec structure.
 grouped=defaultdict(list)
 for f in frames:grouped[f["name"],f["marker"]].append(f)
 for key,items in sorted(grouped.items()):
  cp=min((prefix(items[0]["data"],x["data"]) for x in items[1:]),default=0)
  for f in items:f["commonPrefix"]=cp
  print(f"M465_COMMON resource={key[0]} marker={key[1]} frames={len(items)} commonPrefix={cp}")
 for f in frames:
  for mode,blob in (("full",f["data"]),("variable",f["data"][f["commonPrefix"]:])):
   bb=bits(blob);c3=patterns(bb,3);c4=patterns(bb,4)
   fractions=[sum((v>>p)&1 for v in blob)/len(blob) for p in range(7,-1,-1)] if blob else [0.0]*8
   transitions=sum(x!=y for x,y in zip(bb,bb[1:]))/max(1,len(bb)-1)
   p3=",".join(f"{k:03b}:{c3[k]}" for k in range(8))
   p4=",".join(f"{k:04b}:{c4[k]}" for k in range(16))
   print(f"M465_FRAME resource={f['name']} ordinal={f['i']} marker={f['marker']} widthField={f['w']} heightField={f['h']} mode={mode} bytes={len(blob)} removedPrefix={0 if mode=='full' else f['commonPrefix']} bitPositions={fmt(fractions)} bitTransition={transitions:.5f} triEntropy={entropy(c3):.5f} quadEntropy={entropy(c4):.5f} tri={p3} quad={p4}")
 # Pairwise phase alignment, limited to selected controls and their variable tails.
 troi=[f for f in frames if f["name"]=="BRDGTROI.SPR"]
 worf=[f for f in frames if f["name"]=="BRDGWORF.SPR"]
 six=next((f for f in troi if f["i"]==6),None)
 if six is None:print("M465_ERROR missing_troi6");return 2
 comparisons=[]
 comparisons.extend((six,f,"troi6_worf") for f in worf)
 comparisons.extend((six,f,"troi6_troi") for f in troi if f["i"]!=6)
 comparisons.extend((a,b,"troi_marker1") for a,b in itertools.combinations([f for f in troi if f["i"]!=6],2))
 for a,b,kind in comparisons:
  # Remove shared per-marker resource prefix and pairwise common prefix;
  # for Troi marker-1 pairs, remove their *pairwise* common prefix to avoid
  # inflating alignment scores with invariant regions.
  pa=a["data"][a["commonPrefix"]:];pb=b["data"][b["commonPrefix"]:]
  pair_prefix=prefix(pa,pb);pa=pa[pair_prefix:];pb=pb[pair_prefix:]
  ranks=phase(bits(pa),bits(pb))
  top=";".join(f"{shift}:{length}:{start}" for shift,length,start in ranks[:5])
  zero=next(x[1] for x in ranks if x[0]==0)
  print(f"M465_PHASE group={kind} a={a['name']}:{a['i']} b={b['name']}:{b['i']} markerSame={int(a['marker']==b['marker'])} bytesA={len(pa)} bytesB={len(pb)} pairPrefixRemoved={pair_prefix} zeroShiftLongest={zero} bestShift={ranks[0][0]} bestLongest={ranks[0][1]} top={top}")
 print(f"M465_SUMMARY frames={len(frames)} phasePairs={len(comparisons)} bitOrder=msb_first analysisOnly=1 readOnly=1 decoderConfirmed=0")
 print("M465_NOTE bitShiftMatchIsNotBitstreamBoundary=1 commonPrefixRemovalIsStatisticalControl=1 markerNotCodecProof=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
