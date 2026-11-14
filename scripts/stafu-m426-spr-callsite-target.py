#!/usr/bin/env python3
"""STAFU M426: inspect x86 candidate callsite around brdgrikr.spr offset.

Read-only. Decode only a narrowly known byte pattern; surrounding hex is
not a complete disassembly and does not establish function boundaries.
"""
import argparse,hashlib,struct
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def i32(b,p):return struct.unpack_from("<i",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--site",type=lambda s:int(s,0),default=0x9f03c)
 args=ap.parse_args();b=args.ovl.read_bytes()
 print("M426_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M426_ERROR header mismatch")
 tab=LE+u32(b,LE+0x40);page=u32(b,LE+0x28);data=LE+u32(b,LE+0x80)
 if page!=4096 or u32(b,LE+0x44)<2:raise SystemExit("M426_ERROR object layout")
 objs=[]
 for i in range(2):
  size,base,flags,first,npages=struct.unpack_from("<IIIII",b,tab+24*i)
  start=data+(first-1)*page
  objs.append((size,base,start,min(len(b),start+size)))
  print("M426_OBJECT index=%d base=0x%x size=0x%x fileStart=0x%x assumedLinear=1"%(i+1,base,size,start))
 o1=objs[0];o2=objs[1];site=args.site
 print("M426_SITE file=0x%x object1Rel=0x%x assumedLinear=0x%x"%(site,site-o1[2],o1[1]+site-o1[2]))
 if not o1[2]<=site<o1[3]:raise SystemExit("M426_ERROR site outside object1")
 print("M426_SITE_CONTEXT start=0x%x bytes=%s"%(site-64,b[site-64:site+112].hex()))
 # M425 hit was an immediate inside mov eax,imm32, not necessarily
 # the instruction start. Locate exact pair of mov/push sequences.
 pat=b"\xb8\x3c\x3e\x00\x00\x50\xb8\x54\x3e\x00\x00\x50\xe8"
 hits=[];p=max(o1[2],site-128)
 while True:
  p=b.find(pat,p,min(o1[3],site+128))
  if p<0:break
  hits.append(p);p+=1
 print("M426_PATTERN count=%d positions=%s"%(len(hits),repr([hex(x) for x in hits])))
 for p in hits:
  v1=u32(b,p+1);v2=u32(b,p+7);disp=i32(b,p+13)
  call=p+12;target=call+5+disp
  print("M426_CALLSITE mov1=0x%x mov2=0x%x callFile=0x%x rel32=%d targetFile=0x%x inObject1=%d"%(
   v1,v2,call,disp,target,int(o1[2]<=target<o1[3])))
  for tag,val in (("arg1",v1),("arg2",v2)):
   q=o2[2]+val
   if o2[2]<=q<o2[3]:
    e=b.find(b"\0",q,min(q+160,o2[3]))
    s=b[q:e] if e>=0 else b[q:min(q+60,o2[3])]
    print("M426_ARGUMENT name=%s object2Rel=0x%x file=0x%x string=%r"%(tag,val,q,s.decode("ascii","backslashreplace")))
   else:print("M426_ARGUMENT name=%s object2Rel=0x%x outOfRange=1"%(tag,val))
  print("M426_AFTER_CALL file=0x%x bytes=%s"%(call+5,b[call+5:call+45].hex()))
  if o1[2]<=target<o1[3]:
   print("M426_TARGET_CONTEXT file=0x%x bytes=%s"%(target,b[target:min(target+256,o1[3])].hex()))
   print("M426_TARGET_BEFORE file=0x%x bytes=%s"%(max(o1[2],target-48),b[max(o1[2],target-48):target].hex()))
   # Enumerate candidate rel32 call instructions in first 256 bytes;
   # opcode E8 may occur in non-instruction data or immediates.
   for k in range(target,min(target+256,o1[3]-4)):
    if b[k]!=0xe8:continue
    d=i32(b,k+1);dest=k+5+d
    print("M426_TARGET_RAW_CALL at=0x%x displacement=%d dest=0x%x inObject1=%d bytes=%s"%(
     k,d,dest,int(o1[2]<=dest<o1[3]),b[max(target,k-8):k+9].hex()))
   # Show possible return sequences as byte occurrences, not confirmed ret.
   rets=[hex(k) for k in range(target,min(target+512,o1[3])) if b[k] in (0xc3,0xc2)]
   print("M426_TARGET_RET_BYTES first=%s"%(repr(rets[:32])))
 # Confirm matching literal elsewhere, but do not infer xrefs.
 for val in (0x3e3c,0x3e54):
  q=o2[2]+val
  print("M426_OFFSET_CHECK value=0x%x file=0x%x context=%s"%(val,q,b[max(o2[2],q-16):min(o2[3],q+48)].hex()))
 print("M426_LIMITATIONS patternAlignedOnly=1 targetRawCallsNotDisassembled=1 noFixupDecode=1 noRuntimeTrace=1 noResourceLoaderConfirmed=1")
if __name__=="__main__":main()
