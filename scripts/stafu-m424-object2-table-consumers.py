#!/usr/bin/env python3
"""STAFU M424: candidate LE fixups and code references to Object-2 table.

Read-only static reconnaissance. Raw operand matches are NOT validated
instruction boundaries; LE fixup byte hits are NOT parsed relocation records.
"""
import argparse,collections,hashlib,struct
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 args=ap.parse_args();b=args.ovl.read_bytes()
 print("M424_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M424_ERROR unexpected header")
 tab=LE+u32(b,LE+0x40);count=u32(b,LE+0x44);page=u32(b,LE+0x28);data=LE+u32(b,LE+0x80)
 if count<2 or page!=4096:raise SystemExit("M424_ERROR unexpected object layout")
 objs=[]
 for i in range(count):
  size,base,flags,first,npages=struct.unpack_from("<IIIII",b,tab+24*i)
  start=data+(first-1)*page;end=min(len(b),start+size)
  objs.append((size,base,first,npages,start,end))
  print("M424_OBJECT index=%d base=0x%x size=0x%x firstPage=%d pages=%d fileStart=0x%x fileEnd=0x%x assumedLinear=1"%(
   i+1,base,size,first,npages,start,end))
 fixpage=LE+u32(b,LE+0x68);fixrec=LE+u32(b,LE+0x6c)
 print("M424_FIXUP_SECTIONS pageTable=0x%x recordTable=0x%x headerOffsets=0x%x/0x%x"%(fixpage,fixrec,u32(b,LE+0x68),u32(b,LE+0x6c)))
 o2=objs[1];o1=objs[0]
 targets=[("table_start",0x164554),("table_end",0x16501c),("table_after",0x165028),
 ("bridge_primary",0x16486c),("bridge_secondary",0x164ed8),("door",0x164f14)]
 for label,f in targets:
  rel=f-o2[4];linear=o2[1]+rel
  print("M424_TARGET name=%s file=0x%x object=2 rel=0x%x linear=0x%x"%(label,f,rel,linear))
  # Look for four-byte constants in Object 1, with variants commonly seen
  # in x86 LE code; report byte context, never assert executable instruction.
  vals=[("object_relative",rel),("linear_address",linear),("file_offset",f)]
  for mode,value in vals:
   pat=struct.pack("<I",value)
   hits=[];pos=o1[4]
   while True:
    pos=b.find(pat,pos,o1[5])
    if pos<0:break
    hits.append(pos);pos+=1
   print("M424_RAW32 target=%s encoding=%s value=0x%x count=%d"%(label,mode,value,len(hits)))
   for p in hits[:25]:
    print("M424_RAW32_HIT target=%s encoding=%s at=0x%x object1Rel=0x%x context=%s"%(
     label,mode,p,p-o1[4],b[max(o1[4],p-12):min(o1[5],p+16)].hex()))
   if len(hits)>25:print("M424_RAW32_TRUNCATED target=%s encoding=%s extra=%d"%(label,mode,len(hits)-25))
  # LE internal fixup target encodings may contain object index 2 and
  # 16/32-bit target offsets. Raw search only; not a parsed fixup record.
  for width in (2,4):
   if rel>=(1<<(width*8)):continue
   pat=bytes([2])+rel.to_bytes(width,"little")
   hi=min(len(b),data)
   hits=[];pos=fixrec
   while 0<=pos<hi:
    pos=b.find(pat,pos,hi)
    if pos<0:break
    hits.append(pos);pos+=1
   print("M424_FIXUP_RAW target=%s pattern=object2_plus_%dbyte_offset count=%d"%(label,width,len(hits)))
   for p in hits[:12]:
    print("M424_FIXUP_RAW_HIT target=%s width=%d at=0x%x context=%s"%(
     label,width,p,b[max(fixrec,p-8):min(hi,p+12)].hex()))
 # 16-bit operands are common; report only when coupled with plausible
 # 32-bit address formation context, without interpreting them as xrefs.
 for label,f in targets:
  rel=f-o2[4]
  if rel>0xffff:continue
  pat=struct.pack("<H",rel)
  hits=[];pos=o1[4]
  while True:
   pos=b.find(pat,pos,o1[5])
   if pos<0:break
   hits.append(pos);pos+=1
  print("M424_RAW16 target=%s value=0x%x count=%d first=%s"%(label,rel,len(hits),repr([hex(x) for x in hits[:20]])))
 # Inspect byte-exact candidate boundary and a small record neighborhood.
 for label,f in targets:
  print("M424_TABLE_BYTES name=%s at=0x%x raw=%s"%(label,f,b[max(o2[4],f-24):min(o2[5],f+48)].hex()))
 print("M424_LIMITATIONS linearPageMappingAssumed=1 raw32NotInstructions=1 raw16NotInstructions=1 fixupRawNotParsed=1 noRuntimeTrace=1 noDecoderIdentified=1")
if __name__=="__main__":main()
