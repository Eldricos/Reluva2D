#!/usr/bin/env python3
"""M466: exact conserved SPR bit-fragment boundaries and controls.
Usage: python scripts/stafu-m466-bit-fragment-boundaries.py m325-record-structure.txt games/STTNG games/STAFU-CD > m466-bit-fragment-boundaries.txt
Read-only; no codec assumptions.
"""
from pathlib import Path
from collections import defaultdict
import hashlib,sys,zipfile

PAIRS=[(1,3),(2,4),(2,7),(2,8),(4,5),(4,7),(4,8),(5,7),(5,8),(7,8)]
def report_text(path):
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
  for p in archives:
   if not p.is_file() or not zipfile.is_zipfile(p):continue
   with zipfile.ZipFile(p) as z:
    hits=[n for n in z.namelist() if n.replace("\\","/").split("/")[-1].upper()==name]
    if len(hits)!=1:
     if hits:raise ValueError("ambiguous archive entry "+name)
     continue
    return z.read(hits[0]),str(p)+":"+hits[0]
 raise FileNotFoundError(name)
def bitseq(data,order):
 shifts=range(7,-1,-1) if order=="msb" else range(8)
 return bytes((v>>shift)&1 for v in data for shift in shifts)
def common_prefix(a,b):
 i=0
 while i<min(len(a),len(b)) and a[i]==b[i]:i+=1
 return i
def longest(a,b,maxshift=15):
 # Shift d means b index = a index + d; runs are maximal within each diagonal.
 results=[]
 for d in range(-maxshift,maxshift+1):
  ia=max(0,-d);ib=ia+d;n=min(len(a)-ia,len(b)-ib)
  best=0;starts=[]
  k=0
  while k<n:
   if a[ia+k]!=b[ib+k]:k+=1;continue
   s=k
   while k<n and a[ia+k]==b[ib+k]:k+=1
   length=k-s
   if length>best:best=length;starts=[(ia+s,ib+s,length,d)]
   elif length==best:starts.append((ia+s,ib+s,length,d))
  if best:results.extend(starts)
 if not results:return []
 m=max(x[2] for x in results)
 return sorted(set(x for x in results if x[2]==m),key=lambda x:(abs(x[3]),x[0],x[1]))
def neighborhood(data,bitstart,bitend):
 # Display 4 surrounding bytes on either side of fragment's touched bytes.
 lo=bitstart//8;hi=(bitend+7)//8
 return f"{data[max(0,lo-4):lo].hex()}/{data[lo:min(hi,len(data))][:12].hex()}/{data[hi:min(len(data),hi+4)].hex()}"
def main():
 if len(sys.argv)<3:print(__doc__.strip());return 2
 report=Path(sys.argv[1]);roots=[Path(x) for x in sys.argv[2:]]
 if not report.is_file():print("M466_ERROR missing_report");return 2
 rows=defaultdict(list)
 for line in report_text(report).splitlines():
  if not line.startswith("SPR_M325_IMAGE "):continue
  d=dict(t.split("=",1) for t in line.split()[1:] if "=" in t)
  name=d.get("resource","").upper()
  if name in ("BRDGTROI.SPR","BRDGWORF.SPR") and d.get("kind")=="direct":rows[name].append(d)
 frames={}
 for name in ("BRDGTROI.SPR","BRDGWORF.SPR"):
  try:raw,source=locate(name,roots)
  except (OSError,ValueError,zipfile.BadZipFile) as e:print(f"M466_ERROR resource={name} detail={e}");return 2
  print(f"M466_RESOURCE resource={name} sha256={hashlib.sha256(raw).hexdigest()} source={source}")
  if not rows[name]:print(f"M466_ERROR no_frames resource={name}");return 2
  for d in rows[name]:
   i=int(d["ordinal"],0);start=int(d["dataOffset"],0);n=int(d["dataBytes"],0);end=int(d["chunkEnd"],0)
   if start<0 or n<=0 or start+n!=end or end>len(raw) or (name,i) in frames:
    print(f"M466_ERROR invalid_frame resource={name} ordinal={i}");return 2
   frames[name,i]=dict(data=raw[start:end],base=start,marker=d.get("marker","NA"))
 troi=[frames["BRDGTROI.SPR",i] for i in range(1,10)]
 common=min(common_prefix(troi[0]["data"],f["data"]) for j,f in enumerate(troi) if j not in (0,5))
 print(f"M466_COMMON resource=BRDGTROI.SPR marker1Prefix={common}")
 comparisons=[("troi",("BRDGTROI.SPR",a),("BRDGTROI.SPR",b)) for a,b in PAIRS]
 comparisons += [("troi6",("BRDGTROI.SPR",6),("BRDGTROI.SPR",i)) for i in range(1,10) if i!=6]
 comparisons += [("worf",("BRDGTROI.SPR",6),("BRDGWORF.SPR",i)) for i in range(1,9)]
 boundaries=defaultdict(list)
 for kind,ak,bk in comparisons:
  a=frames[ak];b=frames[bk]
  # Exclude common marker1 Troi prefix, then any remaining pairwise common bytes.
  ca=common if ak[0]=="BRDGTROI.SPR" and ak[1]!=6 else 0
  cb=common if bk[0]=="BRDGTROI.SPR" and bk[1]!=6 else 0
  # For Worf controls, omit the common Worf prefix across all eight frames.
  if bk[0]=="BRDGWORF.SPR":
   wf=[frames["BRDGWORF.SPR",i]["data"] for i in range(1,9)]
   cb=min(common_prefix(wf[0],x) for x in wf[1:])
  pa=a["data"][ca:];pb=b["data"][cb:]
  pair=common_prefix(pa,pb);ca+=pair;cb+=pair
  pa=a["data"][ca:];pb=b["data"][cb:]
  for order in ("msb","lsb"):
   matches=longest(bitseq(pa,order),bitseq(pb,order))
   if not matches:continue
   for ia,ib,n,shift in matches[:3]:
    abs_a=ca*8+ia;abs_b=cb*8+ib
    byte_a=abs_a//8;byte_b=abs_b//8
    # Absolute file offset is payload base + byte offset.
    print(f"M466_MATCH group={kind} a={ak[0]}:{ak[1]} b={bk[0]}:{bk[1]} order={order} lengthBits={n} shift={shift} aStartBit={abs_a} aEndBit={abs_a+n} bStartBit={abs_b} bEndBit={abs_b+n} aStartByte={byte_a} bStartByte={byte_b} aBitPhase={abs_a%8} bBitPhase={abs_b%8} aFileByte={a['base']+byte_a} bFileByte={b['base']+byte_b} aContext={neighborhood(a['data'],abs_a,abs_a+n)} bContext={neighborhood(b['data'],abs_b,abs_b+n)} removedA={ca} removedB={cb}")
    if kind=="troi" and order=="msb" and n>=64:
     boundaries[(ak[1],byte_a,abs_a%8)].append((bk[1],n))
 print(f"M466_SUMMARY comparisons={len(comparisons)} troi={len(PAIRS)} troi6=8 worf=8 readOnly=1 codecConfirmed=0")
 for key,vals in sorted(boundaries.items(),key=lambda x:(-len(x[1]),x[0]))[:30]:
  print(f"M466_RECURRENT anchorFrame={key[0]} startByte={key[1]} phase={key[2]} comparisons={len(vals)} partners={','.join(str(x[0]) for x in vals)} lengths={','.join(str(x[1]) for x in vals)}")
 print("M466_NOTE shiftIsRelativeBitIndexNotCodecBoundary=1 LSBOrderIsDiagnosticOnly=1 contextsShowBeforeMatchedPrefixAfter=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
