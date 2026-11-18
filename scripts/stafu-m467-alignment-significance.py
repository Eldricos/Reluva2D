#!/usr/bin/env python3
"""M467: global longest-common-bit-substring and deterministic null controls.
Usage: python scripts/stafu-m467-alignment-significance.py m325-record-structure.txt games/STTNG games/STAFU-CD > m467-alignment-significance.txt
Read-only; tests byte-aligned alternatives, arbitrary offsets and deterministic shuffles.
"""
from collections import defaultdict
from pathlib import Path
import hashlib,random,sys,zipfile
TROI="BRDGTROI.SPR"; WORF="BRDGWORF.SPR"
PAIRS=[(1,3),(2,4),(2,7),(2,8),(4,5),(4,7),(4,8),(5,7),(5,8),(7,8)]
def read_text(p):
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
    if len(hits)>1:raise ValueError("ambiguous archive entry "+name)
    if hits:return z.read(hits[0]),str(p)+":"+hits[0]
 raise FileNotFoundError(name)
def bits(data):
 return bytes((v>>k)&1 for v in data for k in range(7,-1,-1))
def prefix(a,b):
 i=0
 while i<min(len(a),len(b)) and a[i]==b[i]:i+=1
 return i
def sam(a):
 # Suffix automaton for exact bit substrings, with representative source endpoint.
 nxt=[{}];link=[-1];length=[0];first=[-1];last=0
 for i,c in enumerate(a):
  cur=len(nxt);nxt.append({});length.append(length[last]+1);link.append(0);first.append(i)
  p=last
  while p>=0 and c not in nxt[p]:nxt[p][c]=cur;p=link[p]
  if p>=0:
   q=nxt[p][c]
   if length[p]+1==length[q]:link[cur]=q
   else:
    clone=len(nxt);nxt.append(nxt[q].copy());length.append(length[p]+1);link.append(link[q]);first.append(first[q])
    while p>=0 and nxt[p].get(c)==q:nxt[p][c]=clone;p=link[p]
    link[q]=link[cur]=clone
  last=cur
 return nxt,link,length,first
def global_match(a,b):
 if not a or not b:return (0,0,0)
 nxt,link,length,first=sam(a);state=0;size=0;best=(0,0,0)
 for j,c in enumerate(b):
  while state and c not in nxt[state]:state=link[state];size=length[state]
  if c in nxt[state]:state=nxt[state][c];size+=1
  else:state=0;size=0
  if size>best[0]:best=(size,first[state]-size+1,j-size+1)
 return best
def byte_match(a,b):
 # Find exact longest common byte substring (SAM over bytes).
 return global_match(a,b)
def shuffled(data,seed):
 arr=list(data);random.Random(seed).shuffle(arr);return bytes(arr)
def main():
 if len(sys.argv)<3:print(__doc__.strip());return 2
 p=Path(sys.argv[1])
 if not p.is_file():print("M467_ERROR missing_report");return 2
 rows=defaultdict(list)
 for line in read_text(p).splitlines():
  if not line.startswith("SPR_M325_IMAGE "):continue
  d=dict(x.split("=",1) for x in line.split()[1:] if "=" in x)
  name=d.get("resource","").upper()
  if name in (TROI,WORF) and d.get("kind")=="direct":rows[name].append(d)
 frames={}
 for name in (TROI,WORF):
  try:raw,source=locate(name,[Path(x) for x in sys.argv[2:]])
  except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M467_ERROR resource={name} detail={e}");return 2
  print(f"M467_RESOURCE resource={name} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
  if not rows[name]:print(f"M467_ERROR no_frames resource={name}");return 2
  for d in rows[name]:
   i=int(d["ordinal"],0);s=int(d["dataOffset"],0);n=int(d["dataBytes"],0);e=int(d["chunkEnd"],0)
   if s<0 or n<=0 or s+n!=e or e>len(raw) or (name,i) in frames:
    print(f"M467_ERROR invalid_frame={name}:{i}");return 2
   frames[name,i]=dict(data=raw[s:e],base=s,marker=d.get("marker","NA"))
 for name,indices in ((TROI,range(1,10)),(WORF,range(1,9))):
  missing=[i for i in indices if (name,i) not in frames]
  if missing:print(f"M467_ERROR missing_frames={name}:{missing}");return 2
 troi=[frames[TROI,i]["data"] for i in (1,2,3,4,5,7,8,9)]
 worf=[frames[WORF,i]["data"] for i in range(1,9)]
 cp_t=min(prefix(troi[0],x) for x in troi[1:])
 cp_w=min(prefix(worf[0],x) for x in worf[1:])
 print(f"M467_PREFIX troiMarker1={cp_t} worfMarker2={cp_w}")
 selected=[("troi",(TROI,a),(TROI,b)) for a,b in PAIRS]
 selected +=[("troi6",(TROI,6),(TROI,i)) for i in range(1,10) if i!=6]
 selected +=[("worf",(TROI,6),(WORF,i)) for i in range(1,9)]
 for kind,ka,kb in selected:
  aa=frames[ka]["data"];bb=frames[kb]["data"]
  offa=cp_t if ka[0]==TROI and ka[1]!=6 else 0
  offb=cp_t if kb[0]==TROI and kb[1]!=6 else cp_w if kb[0]==WORF else 0
  a=aa[offa:];b=bb[offb:]
  # Remove pairwise common prefix as in M466, but then search globally,
  # with no restriction on relative starting positions.
  common=prefix(a,b);offa+=common;offb+=common
  a=aa[offa:];b=bb[offb:]
  length,ia,ib=global_match(bits(a),bits(b))
  by,ba,bbidx=byte_match(a,b)
  abs_a=offa*8+ia;abs_b=offb*8+ib
  delta=abs_b-abs_a
  print(f"M467_GLOBAL group={kind} a={ka[0]}:{ka[1]} b={kb[0]}:{kb[1]} bits={length} aStartBit={abs_a} bStartBit={abs_b} aEndBit={abs_a+length} bEndBit={abs_b+length} deltaBits={delta} deltaModulo8={delta%8} aPhase={abs_a%8} bPhase={abs_b%8} aFileByte={frames[ka]['base']+abs_a//8} bFileByte={frames[kb]['base']+abs_b//8} exactByteRun={by} byteStartA={offa+ba} byteStartB={offb+bbidx} bitRunContainsBytes={length//8} pairPrefixRemoved={common}")
  if kind=="troi" and (ka[1],kb[1]) in ((2,7),(2,8),(5,8),(7,8)):
   for trial in range(1,6):
    # Null preserves each payload's byte histogram and length, not its local ordering.
    sa=shuffled(a,467000+ka[1]*100+trial)
    sb=shuffled(b,467000+kb[1]*1000+trial)
    null=global_match(bits(sa),bits(sb))[0]
    print(f"M467_NULL group={kind} a={ka[1]} b={kb[1]} trial={trial} shuffledLongestBits={null} observedBits={length} seedA={467000+ka[1]*100+trial} seedB={467000+kb[1]*1000+trial}")
 print(f"M467_SUMMARY pairs={len(selected)} nullTrials=20 globalSearch=1 deterministic=1 readOnly=1 codecConfirmed=0")
 print("M467_NOTE nullControlsPreserveByteCountsNotDependencies=1 exactByteRunIsIndependentOfBitRun=1 fiveNullTrialsNotFormalPValue=1 suffixAutomatonReturnsOneMaximum=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
