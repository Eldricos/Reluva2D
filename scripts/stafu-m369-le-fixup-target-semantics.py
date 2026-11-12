#!/usr/bin/env python3
"""M369: independent LE fixup-field and handler-boundary audit (read-only).

For both dispatcher pages, parses each record byte-by-byte, verifies source
and target fields, tests plausible address-base interpretations, and searches
near each putative handler for coherent instruction starts. No runtime claims.
Requires capstone. Never writes game resources.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",19,0x334,1),("global_state",20,0x488,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print("M369_INPUT bytes=%d sha256=%s"%(len(b),sha))
 if len(b)!=2008455 or sha!=EXPECTED or b[LE:LE+2]!=b"LE":raise SystemExit("M369_ERROR unexpected OVL")
 ps=u32(b,LE+0x28);total=u32(b,LE+0x14)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[]
 for i in range(u32(b,LE+0x44)):
  p=ot+i*24;objs.append((u32(b,p),u32(b,p+4),u32(b,p+12),u32(b,p+16)))
 if ps!=4096 or total!=371 or len(objs)!=2:raise SystemExit("M369_ERROR layout")
 starts=[u32(b,fi+i*4) for i in range(total+1)]
 def page(n):
  entry=b[pm+4*(n-1):pm+4*n];num=int.from_bytes(entry[:3],"big")
  if num<1 or entry[3]:raise ValueError("unsupported page")
  p=origin+(num-1)*ps;return bytearray(b[p:p+ps]),p
 cs=Cs(CS_ARCH_X86,CS_MODE_32)
 for name,logical,first,state0 in CASES:
  original,physical=page(logical);relocated=bytearray(original)
  blob=b[fr+starts[logical-1]:fr+starts[logical]]
  pos=0;records=[];covered=set()
  while pos<len(blob):
   start=pos
   if pos+9>len(blob):raise SystemExit("M369_ERROR truncated record")
   src,flags=blob[pos],blob[pos+1];pos+=2
   if src!=7 or flags!=0x10:
    raise SystemExit("M369_UNSUPPORTED name=%s pos=%d src=0x%x flags=0x%x"%(name,start,src,flags))
   off=u16(blob,pos);pos+=2;obj=blob[pos];pos+=1;target=u32(blob,pos);pos+=4
   if obj not in (1,2) or off+4>ps or target>=objs[obj-1][0] or any(x in covered for x in range(off,off+4)):
    raise SystemExit("M369_ERROR invalid fixup name=%s record=%d"%(name,start))
   covered.update(range(off,off+4))
   value=objs[obj-1][1]+target
   relocated[off:off+4]=struct.pack("<I",value)
   records.append((start,off,obj,target,value))
  print("M369_PAGE name=%s logical=%d records=%d bytes=%d allSource07Flags10=1 overlap=0"%(name,logical,len(records),len(blob)))
  bysource={off:(start,obj,target,value) for start,off,obj,target,value in records}
  pagebase=(logical-objs[0][2])*ps
  for i in range(7):
   state=state0+i;source=first+4*i
   if source not in bysource:raise SystemExit("M369_ERROR missing table fixup")
   rec,obj,target,value=bysource[source]
   print("M369_RECORD name=%s state=%d record=%d sourcePageOffset=0x%x sourceBytes=%s object=%d targetField=0x%x preferredLinear=0x%x"%
         (name,state,rec,source,blob[rec:rec+9].hex(),obj,target,value))
   # Test offset as object RVA, linear absolute, or relative to preferred base.
   for interpretation,rva in (("object_rva",target),("linear_absolute",target-objs[0][1]),("base_added_twice",target+objs[0][1])):
    inobj=0<=rva<objs[0][0];inpage=pagebase<=rva<pagebase+ps
    print("M369_ADDRESS name=%s state=%d interpretation=%s rva=0x%x inObject=%d inCurrentPage=%d"%
          (name,state,interpretation,rva,int(inobj),int(inpage)))
   off=target-pagebase
   if not 0<=off<ps:continue
   for delta in range(-4,5):
    startoff=off+delta
    if not 0<=startoff<ps:continue
    ins=list(cs.disasm(bytes(original[startoff:min(ps,startoff+48)]),pagebase+startoff,count=5))
    totalbytes=sum(x.size for x in ins)
    crosses=any((x.address-pagebase+j) in covered for x in ins for j in range(x.size))
    firstins=(ins[0].mnemonic+"_"+ins[0].op_str.replace(" ","_")) if ins else "none"
    suspicious=sum(x.mnemonic in ("cli","sti","hlt","lldt","ltr","out","in","iret","retf","int3") for x in ins)
    print("M369_BOUNDARY name=%s state=%d delta=%+d startRva=0x%x decoded=%d decodedBytes=%d fixupOverlap=%d privilegedOrUnusual=%d first=%s"%
          (name,state,delta,pagebase+startoff,len(ins),totalbytes,int(crosses),suspicious,firstins))
  # Search whether a relocation writes across an exact candidate target.
  for state,target in ((state0+i,bysource[first+4*i][2]) for i in range(7)):
   off=target-pagebase
   print("M369_TARGET_OVERLAP name=%s state=%d targetRva=0x%x targetInFixup=%d next16FixupBytes=%d"%
         (name,state,target,int(off in covered),sum(x in covered for x in range(off,off+16))))
  print("M369_PAGE_END name=%s"%name)
 print("M369_LIMITATIONS grammarRestrictedTo07_10=1 alternativesHeuristic=1 originalBytesForBoundaryScan=1 noExternalLESpecValidation=1 noRuntimeSelectorVerification=1 noReachabilityProof=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
