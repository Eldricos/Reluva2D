#!/usr/bin/env python3
"""M375: LE relocation application semantics and independent dispatcher search.

Read-only diagnostic; never executes or writes game data. Tests competing
source-write and target-address interpretations rather than assuming validity.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M375_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M375_ERROR unexpected OVL")
 ps=u32(b,LE+0x28);n=u32(b,LE+0x14)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48);fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[(u32(b,ot+24*i),u32(b,ot+24*i+4),u32(b,ot+24*i+12),u32(b,ot+24*i+16)) for i in range(u32(b,LE+0x44))]
 assert ps==4096 and n==371 and len(objs)==2
 ix=[u32(b,fi+4*i) for i in range(n+1)]
 def page(logical):
  j=logical-1;entry=b[pm+4*j:pm+4*j+4];pn=int.from_bytes(entry[:3],"big")
  if pn<1 or entry[3]:raise ValueError("unsupported page")
  off=origin+(pn-1)*ps
  return b[off:off+ps],off
 def records(logical):
  blob=b[fr+ix[logical-1]:fr+ix[logical]];p=0;out=[]
  while p<len(blob):
   start=p
   if p+7>len(blob):raise ValueError("truncated")
   src,flags=blob[p],blob[p+1];p+=2
   if src!=7 or flags not in (0,16):raise ValueError(f"unsupported record {src:02x}/{flags:02x}")
   off=u16(blob,p);p+=2;obj=blob[p];p+=1
   w=4 if flags&16 else 2
   if p+w>len(blob):raise ValueError("truncated target")
   target=u32(blob,p) if w==4 else u16(blob,p);p+=w
   if obj not in (1,2):raise ValueError("target object")
   out.append((start,off,obj,target,flags))
  return out
 # Decode real jump signatures across the complete object image, not
 # within the fixup-table pages. The previously observed operand bytes
 # are preserved as a separate independent cross-check.
 signatures=(("object_state",bytes.fromhex("2eff248534230100")),
             ("global_state",bytes.fromhex("2eff248588340100")))
 for name,signature in signatures:
  hits=[]
  for logical in range(objs[0][2],objs[0][2]+objs[0][3]):
   raw,physical=page(logical)
   for i in range(ps-len(signature)+1):
    if raw[i:i+len(signature)]==signature:
     rva=(logical-objs[0][2])*ps+i
     hits.append((rva,physical+i))
  print(f"M375_JUMP name={name} signature={signature.hex()} matches={len(hits)} positions={','.join(f'rva:0x{r:x}/file:0x{f:x}' for r,f in hits[:10]) or 'none'}")
 # The LE format source type 07 denotes a 32-bit offset; the 0x10
 # target flag denotes 32-bit target offset, NOT a 32-bit source width.
 print("M375_FIELD_MODEL sourceType07=32bitOffset targetFlag10=32bitTargetOffset sourceWidth=4 targetOffsetWidthByFlag=2or4 targetObject=oneBased")
 for logical,name,table,near in (
  (19,"object_state",0x334,(0x373,0x378,0x37d,0x3b1,0x3c6,0x40e,0x423)),
  (20,"global_state",0x488,(0x4cf,0x4df,0x4f8,0x545,0x574,0x5a0,0x5b5))):
  raw,physical=page(logical);rs=records(logical)
  base=(logical-objs[0][2])*ps
  print(f"M375_PAGE name={name} logical={logical} physical=0x{physical:x} records={len(rs)} flags={dict(Counter(r[4] for r in rs))}")
  for off in list(table+4*i for i in range(7))+list(near):
   match=[r for r in rs if r[1]==off]
   if not match:
    print(f"M375_SOURCE name={name} sourceRva=0x{base+off:x} record=none");continue
   if len(match)!=1:raise ValueError("duplicate source")
   record,source,obj,target,flags=match[0]
   old=raw[off:off+4]
   absolute=objs[obj-1][1]+target
   variants={
    "absolute_linear":absolute,
    "object_relative":target,
    "object_base_plus_target_minus_one":objs[obj-1][1]+target-1,
    "object_base_plus_target_plus_one":objs[obj-1][1]+target+1,
   }
   print(f"M375_SOURCE name={name} sourceRva=0x{base+off:x} record={record} flags=0x{flags:x} object={obj} target=0x{target:x} original={old.hex()} originalLE=0x{u32(raw,off):x} targetObjectBase=0x{objs[obj-1][1]:x}")
   for label,value in variants.items():
    print(f"M375_VALUE name={name} sourceRva=0x{base+off:x} interpretation={label} value=0x{value:x} bytes={struct.pack('<I',value&0xffffffff).hex()}")
   # Evaluate write placement in the source page without modifying it.
   for shift in (-2,-1,0,1,2):
    dest=off+shift
    if 0<=dest<=ps-4:
     print(f"M375_PLACEMENT name={name} recordedSourceRva=0x{base+off:x} shift={shift:+d} destRva=0x{base+dest:x} original={raw[dest:dest+4].hex()} isTableSlot={int(table<=dest<table+28 and (dest-table)%4==0)}")
  # Check expected seven slots under alternative write shifts.
  for shift in (-2,-1,0,1,2):
   exact=sum(any(r[1]+shift==table+4*i for r in rs) for i in range(7))
   print(f"M375_TABLE_ALIGNMENT name={name} sourceShift={shift:+d} exactSlots={exact}/7")
 print("M375_LIMITATIONS formatFieldInterpretationHypothesis=1 noRuntimeLoader=1 noProvenCodeBoundaries=1 noSPRDecoder=1")
if __name__=="__main__":main()
