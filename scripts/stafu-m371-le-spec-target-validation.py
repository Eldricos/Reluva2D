#!/usr/bin/env python3
"""M371: LE fixup target validation against published IBM LE/LX field semantics.

Reference: IBM OS/2 16/32-bit Object Module Format, LX Fixup Record Table,
Internal Fixup Record. Source 07=32-bit offset, target flag 10=32-bit
target offset, internal TRGOFF=offset within target object.
https://komh.github.io/os2books/os2tk45/lxref.htm

Read-only; checks source and target encodings and contrasts raw/relocated
instruction boundaries, without claiming runtime execution.
"""
import argparse,hashlib,struct
from pathlib import Path
from collections import Counter
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780;SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",19,0x334,1),("global_state",20,0x488,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print("M371_INPUT bytes=%d sha256=%s"%(len(b),sha))
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M371_ERROR unexpected input")
 ps=u32(b,LE+0x28);np=u32(b,LE+0x14);ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[(u32(b,ot+i*24),u32(b,ot+i*24+4),u32(b,ot+i*24+12),u32(b,ot+i*24+16)) for i in range(u32(b,LE+0x44))]
 if ps!=4096 or np!=371 or len(objs)!=2:raise SystemExit("M371_ERROR layout")
 idx=[u32(b,fi+i*4) for i in range(np+1)]
 def getpage(n):
  e=b[pm+4*(n-1):pm+4*n];num=int.from_bytes(e[:3],"big")
  if num<1 or e[3]:raise ValueError("unsupported page map")
  phys=origin+(num-1)*ps
  return bytearray(b[phys:phys+ps]),phys
 cs=Cs(CS_ARCH_X86,CS_MODE_32)
 print("M371_SPEC source07=offset32 target00=internal target10=offset32 target00width=offset16 internalTargetOffset=objectRelative")
 print("M371_OBJECTS "+" ".join("object%d:size0x%x:base0x%x:first%d:pages%d"%(i+1,*o) for i,o in enumerate(objs)))
 for name,logical,first,state0 in CASES:
  original,physical=getpage(logical);page=bytearray(original)
  blob=b[fr+idx[logical-1]:fr+idx[logical]];pos=0;records={};covered=set();widths=Counter()
  while pos<len(blob):
   rec=pos
   if pos+7>len(blob):raise ValueError("truncated record")
   src,flags=blob[pos:pos+2];pos+=2
   if src!=7 or flags not in (0,0x10):raise ValueError("unsupported record at %d: %02x %02x"%(rec,src,flags))
   off=u16(blob,pos);pos+=2;obj=blob[pos];pos+=1
   width=4 if flags&0x10 else 2
   if pos+width>len(blob):raise ValueError("truncated target")
   target=u32(blob,pos) if width==4 else u16(blob,pos);pos+=width
   if obj<1 or obj>len(objs) or target>=objs[obj-1][0] or off+4>ps or any(i in covered for i in range(off,off+4)):
    raise ValueError("invalid record at %d"%rec)
   covered.update(range(off,off+4));widths[width]+=1
   value=objs[obj-1][1]+target
   page[off:off+4]=struct.pack("<I",value)
   records[off]=(rec,src,flags,obj,target,value)
  print("M371_PAGE name=%s logical=%d physical=0x%x records=%d width16=%d width32=%d complete=%d"%
   (name,logical,physical,len(records),widths[2],widths[4],int(pos==len(blob))))
  base_rva=(logical-objs[0][2])*ps
  for i in range(7):
   state=state0+i;off=first+4*i
   if off not in records:raise ValueError("missing table source")
   rec,src,flags,obj,target,linear=records[off]
   local=target-base_rva
   print("M371_TARGET name=%s state=%d sourceOffset=0x%x record=%d sourceType=0x%x flags=0x%x targetObject=%d targetOffset=0x%x targetLinear=0x%x targetPageOffset=0x%x validObjectOffset=%d"%
    (name,state,off,rec,src,flags,obj,target,linear,local,int(target<objs[obj-1][0])))
   if not 0<=local<ps:continue
   for delta in (-2,-1,0,1,2):
    start=local+delta
    if start<0 or start>=ps:continue
    raw=list(cs.disasm(bytes(original[start:min(ps,start+32)]),target+delta,count=4))
    patched=list(cs.disasm(bytes(page[start:min(ps,start+32)]),target+delta,count=4))
    def fmt(seq):
     return "|".join("%s_%s"%(ins.mnemonic,ins.op_str.replace(" ","_")) for ins in seq)
    print("M371_BOUNDARY name=%s state=%d delta=%+d rawCount=%d patchedCount=%d raw=%s patched=%s"%
     (name,state,delta,len(raw),len(patched),fmt(raw),fmt(patched)))
   print("M371_FIXUP_NEAR name=%s state=%d targetInSource=%d sourceBytesWithinNext16=%d"%
    (name,state,int(local in covered),sum((local+j) in covered for j in range(16))))
  print("M371_PAGE_END name=%s"%name)
 print("M371_CONCLUSION_SCOPE specFieldsCrosschecked=1 runtimeSelectorUnknown=1 runtimeRelocationNotObserved=1 instructionBoundariesNotProven=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
