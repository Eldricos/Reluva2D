#!/usr/bin/env python3
"""M372: classify LE relocation sources around global-state handler targets.

Read-only analysis. Compare original and patched bytes, record target object,
local instruction decoding, and direct-branch evidence. No runtime claims.
"""
import argparse,hashlib,struct
from pathlib import Path
from collections import Counter
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780;SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",19,(0x1239b,0x1238e,0x123ca,0x123f8,0x123eb,0x12429)),("global_state",20,(0x13598,0x1356e,0x134ed,0x13561)))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print("M372_INPUT bytes=%d sha256=%s"%(len(b),sha))
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M372_ERROR unexpected OVL")
 ps=u32(b,LE+0x28);np=u32(b,LE+0x14);ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[(u32(b,ot+i*24),u32(b,ot+i*24+4),u32(b,ot+i*24+12),u32(b,ot+i*24+16)) for i in range(u32(b,LE+0x44))]
 if ps!=4096 or np!=371 or len(objs)!=2:raise SystemExit("M372_ERROR layout")
 idx=[u32(b,fi+i*4) for i in range(np+1)]
 def getpage(n):
  e=b[pm+4*(n-1):pm+4*n];num=int.from_bytes(e[:3],"big")
  if num<1 or e[3]:raise ValueError("unsupported page map")
  phys=origin+(num-1)*ps
  return bytearray(b[phys:phys+ps]),phys
 cs=Cs(CS_ARCH_X86,CS_MODE_32);cs.detail=True
 def instructions(buf,base,start,count=6):
  return list(cs.disasm(bytes(buf[start:min(len(buf),start+64)]),base+start,count=count))
 def format_ins(ins):
  return "%x:%s_%s"%(ins.address,ins.mnemonic,ins.op_str.replace(" ","_"))
 for name,logical,targets in CASES:
  raw,physical=getpage(logical);patched=bytearray(raw)
  blob=b[fr+idx[logical-1]:fr+idx[logical]];pos=0;records=[];covered=set()
  while pos<len(blob):
   rec=pos
   if pos+7>len(blob):raise ValueError("truncated fixup")
   src,flags=blob[pos:pos+2];pos+=2
   if src!=7 or flags not in (0,0x10):raise ValueError("unsupported record at %d: %02x %02x"%(rec,src,flags))
   off=u16(blob,pos);pos+=2;obj=blob[pos];pos+=1
   width=4 if flags&0x10 else 2
   if pos+width>len(blob):raise ValueError("truncated target")
   target=u32(blob,pos) if width==4 else u16(blob,pos);pos+=width
   if obj<1 or obj>len(objs) or target>=objs[obj-1][0] or off+4>ps or any(j in covered for j in range(off,off+4)):
    raise ValueError("invalid record at %d"%rec)
   before=bytes(raw[off:off+4]);value=objs[obj-1][1]+target
   after=struct.pack("<I",value);patched[off:off+4]=after
   covered.update(range(off,off+4))
   records.append((off,rec,obj,target,before,after))
  base=(logical-objs[0][2])*ps
  print("M372_PAGE name=%s logical=%d physical=0x%x records=%d bytesChanged=%d"%
   (name,logical,physical,len(records),sum(x!=y for x,y in zip(raw,patched))))
  for target in targets:
   local=target-base
   if not 0<=local<ps:continue
   nearby=[r for r in records if target-32<=base+r[0]<=target+64]
   print("M372_TARGET name=%s targetRva=0x%x pageOffset=0x%x nearbySources=%d"%(name,target,local,len(nearby)))
   for off,rec,obj,dst,before,after in nearby:
    distance=off-local
    # An instruction starting at source-8 can overlap the source; that is
    # only a heuristic, not a verified control-flow boundary.
    print("M372_SOURCE name=%s targetRva=0x%x sourceRva=0x%x delta=%+d record=%d targetObject=%d targetOffset=0x%x original=%s relocated=%s originalAscii=%s"%
     (name,target,base+off,distance,rec,obj,dst,before.hex(),after.hex(),
      "".join(chr(v) if 32<=v<127 else "." for v in before)))
    for shift in (-4,-2,0):
     st=off+shift
     if st<0:continue
     ri=instructions(raw,base,st,4);pi=instructions(patched,base,st,4)
     raw_overlap=any(ins.address<base+off+4 and ins.address+ins.size>base+off for ins in ri)
     patch_overlap=any(ins.address<base+off+4 and ins.address+ins.size>base+off for ins in pi)
     print("M372_SOURCE_DECODE name=%s sourceRva=0x%x startDelta=%+d rawOverlap=%d patchedOverlap=%d raw=%s patched=%s"%
      (name,base+off,shift,int(raw_overlap),int(patch_overlap),
       "|".join(map(format_ins,ri)),"|".join(map(format_ins,pi))))
   for delta in (-1,0,1):
    st=local+delta
    if st<0:continue
    ri=instructions(raw,base,st);pi=instructions(patched,base,st)
    print("M372_HANDLER name=%s targetRva=0x%x delta=%+d raw=%s patched=%s"%
     (name,target,delta,"|".join(map(format_ins,ri)),"|".join(map(format_ins,pi))))
  # whole-page linear sweep is a weak boundary heuristic; report as such
  raw_linear=list(cs.disasm(bytes(raw),base));patched_linear=list(cs.disasm(bytes(patched),base))
  def crossings(insns):
   return sum(any(ins.address<base+off+4 and ins.address+ins.size>base+off for ins in insns) for off,*_ in records)
  print("M372_PAGE_HEURISTIC name=%s rawLinearInstructions=%d patchedLinearInstructions=%d rawSourcesTouchedBySweep=%d patchedSourcesTouchedBySweep=%d"%
   (name,len(raw_linear),len(patched_linear),crossings(raw_linear),crossings(patched_linear)))
  print("M372_PAGE_END name=%s"%name)
 print("M372_LIMITATIONS instructionBoundaryHeuristicsOnly=1 noRuntimeRelocationVerification=1 noRuntimeSelectorVerification=1 noSPRDecoderIdentification=1")
if __name__=="__main__":main()
