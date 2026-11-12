#!/usr/bin/env python3
"""M373: independently audit LE fixup source offsets, page boundaries and chains.

Read-only. Separates format-level record decoding from executable-code heuristics.
No original game data is stored in this script.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def s16(n):return n-65536 if n>=32768 else n
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print("M373_INPUT bytes=%d sha256=%s"%(len(b),sha))
 if sha!=SHA or len(b)!=2008455 or b[LE:LE+2]!=b"LE":raise SystemExit("M373_ERROR unexpected executable")
 ps=u32(b,LE+0x28);np=u32(b,LE+0x14)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[(u32(b,ot+i*24),u32(b,ot+i*24+4),u32(b,ot+i*24+12),u32(b,ot+i*24+16)) for i in range(u32(b,LE+0x44))]
 if ps!=4096 or np!=371 or len(objs)!=2:raise SystemExit("M373_ERROR unexpected LE layout")
 index=[u32(b,fi+4*i) for i in range(np+1)]
 def page(n):
  e=b[pm+4*(n-1):pm+4*n];pn=int.from_bytes(e[:3],"big")
  if not pn or e[3]:raise ValueError("unsupported page")
  physical=origin+(pn-1)*ps
  return b[physical:physical+ps],physical
 cs=Cs(CS_ARCH_X86,CS_MODE_32)
 def parse(n):
  stream=b[fr+index[n-1]:fr+index[n]]
  p=0;out=[]
  while p<len(stream):
   at=p
   if p+7>len(stream):raise ValueError("truncated fixup page %d"%n)
   source,flags=stream[p:p+2];p+=2
   if source!=7 or flags not in (0,16):raise ValueError("unhandled source/flags page %d at %d: %02x %02x"%(n,at,source,flags))
   off=u16(stream,p);p+=2;obj=stream[p];p+=1
   w=4 if flags&16 else 2
   if p+w>len(stream):raise ValueError("truncated target")
   target=u32(stream,p) if w==4 else u16(stream,p);p+=w
   out.append((at,source,flags,off,s16(off),obj,target))
  return out
 print("M373_SPEC_AUDIT sourceType07=offset32 sourceOffsetField=signed16 possibleCrossPage=1 chainBitSource=0x20 chainBitTarget=0x08 chainNotObservedByRestrictedParser=1")
 print("M373_LAYOUT pageSize=%d modulePages=%d object1Base=0x%x object1FirstPage=%d object1Pages=%d"%
  (ps,np,objs[0][1],objs[0][2],objs[0][3]))
 for n,name in ((18,"preceding_object_state"),(19,"object_state"),(20,"global_state"),(21,"following_global_state")):
  data,phys=page(n);recs=parse(n);negative=[r for r in recs if r[4]<0]
  beyond=[r for r in recs if r[3]>ps-4]
  print("M373_PAGE name=%s logical=%d physical=0x%x records=%d signedNegative=%d unsignedBeyondPage=%d flags=%s"%
   (name,n,phys,len(recs),len(negative),len(beyond),dict(sorted(Counter(r[2] for r in recs).items()))))
  for r in negative[:12]+beyond[:12]:
   print("M373_EDGE name=%s record=%d rawOffset=0x%x signedOffset=%d targetObject=%d targetOffset=0x%x"%
    (name,r[0],r[3],r[4],r[5],r[6]))
 for n,name,table,handlers in ((19,"object_state",0x334,(0x1239b,0x1238e,0x123ca,0x123f8,0x123eb,0x12429)),
                                (20,"global_state",0x488,(0x13598,0x1356e,0x134ed,0x13561))):
  data,phys=page(n);recs=parse(n);base=(n-objs[0][2])*ps
  print("M373_REGION name=%s pageBaseRva=0x%x tableOffset=0x%x"%(name,base,table))
  for i in range(7):
   off=table+4*i
   matching=[r for r in recs if r[4]==off]
   if len(matching)!=1:raise ValueError("table fixup missing/duplicate")
   r=matching[0]
   print("M373_TABLE name=%s index=%d source=0x%x record=%d flags=0x%x targetObject=%d targetOffset=0x%x"%
    (name,i,off,r[0],r[2],r[5],r[6]))
  for target in handlers:
   local=target-base
   print("M373_HANDLER name=%s target=0x%x local=0x%x"%(name,target,local))
   for r in recs:
    at,source,flags,raw,signed,obj,dst=r
    if abs(signed-local)>24:continue
    before=data[signed:signed+4] if 0<=signed<=ps-4 else b""
    val=objs[obj-1][1]+dst if 1<=obj<=len(objs) else -1
    # Independently inspect whether the source is inside an immediate/
    # displacement of plausible instruction starting up to 6 bytes earlier.
    overlaps=[]
    for shift in range(1,7):
     start=signed-shift
     if start<0:continue
     ins=list(cs.disasm(data[start:min(ps,start+16)],base+start,count=1))
     if ins and ins[0].size>shift:
      overlaps.append("%+d:%s_%s"%( -shift,ins[0].mnemonic,ins[0].op_str.replace(" ","_")))
    print("M373_NEAR name=%s handler=0x%x record=%d signedSourceRva=0x%x delta=%+d original=%s targetObject=%d targetOffset=0x%x relocatedValue=0x%x precedingInstructionCandidates=%s"%
     (name,target,at,base+signed,signed-local,before.hex(),obj,dst,val,"|".join(overlaps) or "none"))
  # Compare alternate source-offset interpretations on a selected cluster.
  for off in ((0x4f8,0x5a0,0x574) if n==20 else (0x40e,0x423,0x3b1)):
   hits=[r for r in recs if r[4]==off]
   for r in hits:
    print("M373_ALTERNATIVES name=%s source=0x%x signedPageRva=0x%x unsignedPageRva=0x%x fileRelative=0x%x pagePhysical=0x%x raw=%s"%
     (name,off,base+r[4],base+r[3],phys+off,phys,data[off:off+4].hex()))
 print("M373_LIMITATIONS parserOnlySupportsSource07AndFlags00_10=1 noRuntimeLoader=1 noInstructionReachabilityProof=1 noSPRDecoder=1")
if __name__=="__main__":main()
