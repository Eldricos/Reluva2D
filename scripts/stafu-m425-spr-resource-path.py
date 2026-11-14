#!/usr/bin/env python3
"""STAFU M425: SPR resource-path reconnaissance (read-only).

Enumerate .spr strings in Object 2, their surrounding 12-byte metadata,
and potential Object 1 address references. Optionally inspect an installed
SPR header. This is a static candidate inventory, NOT a runtime trace.
"""
import argparse,collections,hashlib,struct
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--spr",type=Path,default=None,help="optional installed SPR file")
 args=ap.parse_args();b=args.ovl.read_bytes()
 print("M425_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M425_ERROR unexpected header")
 tab=LE+u32(b,LE+0x40);page=u32(b,LE+0x28);data=LE+u32(b,LE+0x80)
 if u32(b,LE+0x44)<2 or page!=4096:raise SystemExit("M425_ERROR unexpected object layout")
 objs=[]
 for i in range(2):
  size,base,flags,first,npages=struct.unpack_from("<IIIII",b,tab+i*24)
  origin=data+(first-1)*page
  objs.append((size,base,origin,min(len(b),origin+size)))
  print("M425_OBJECT index=%d base=0x%x size=0x%x origin=0x%x assumedLinear=1"%(i+1,base,size,origin))
 size,base,origin,end=objs[1];codebase,codeend=objs[0][2:]
 # Find NUL-terminated ASCII tokens ending in .spr; retain exact offsets.
 import re
 found=[]
 for m in re.finditer(rb"(?i)[a-z0-9_./\\-]{3,90}\.spr\x00",b[origin:end]):
  raw=m.group()[:-1];f=origin+m.start()
  found.append((f,f-origin,raw.decode("ascii","replace")))
 print("M425_SPR_STRINGS count=%d"%(len(found)))
 for f,rel,name in found[:120]:
  print("M425_SPR_STRING file=0x%x object2Rel=0x%x name=%r"%(f,rel,name))
 if len(found)>120:print("M425_SPR_STRINGS_TRUNCATED extra=%d"%(len(found)-120))
 # Existing table records can point at string starts or suffixes. Search
 # records in the characterized candidate region, no semantic assumption.
 lo,hi=0x164554,0x165028
 for f,rel,name in found:
  matches=[]
  for p in range(lo,min(hi,len(b)-11),12):
   v,a,c=struct.unpack_from("<III",b,p)
   if rel<=v<rel+len(name):matches.append((p,v-rel,a,c))
  if matches:
   print("M425_TABLE_REFS name=%r count=%d records=%s"%(name,len(matches),repr([(hex(p),delta,a,c) for p,delta,a,c in matches[:20]])))
 # Search object1 for direct 32-bit values of object2 string addresses;
 # this does not cover fixups or dynamically computed addresses.
 for f,rel,name in found[:120]:
  for mode,value in (("relative",rel),("linear",base+rel)):
   pat=struct.pack("<I",value);hits=[];p=codebase
   while True:
    p=b.find(pat,p,codeend)
    if p<0:break
    hits.append(p);p+=1
   if hits:
    print("M425_CODE_RAW32 name=%r mode=%s count=%d"%(name,mode,len(hits)))
    for p in hits[:8]:
     print("M425_CODE_HIT name=%r mode=%s file=0x%x context=%s"%(name,mode,p,b[max(codebase,p-12):min(codeend,p+16)].hex()))
 # Disassemble-free inventory of direct CALL rel32 bytes in Object 1
 # within +/- 96 bytes of raw code references, if any; do not claim
 # instruction alignment or resource call semantics.
 print("M425_PATH_STATUS noValidatedCallsite=1 noRuntimeTrace=1 noDecoderIdentified=1")
 if args.spr:
  s=args.spr.read_bytes()
  print("M425_SPR_INPUT file=%r bytes=%d sha256=%s header64=%s"%(
   str(args.spr),len(s),hashlib.sha256(s).hexdigest(),s[:64].hex()))
  for tag in (b"TRPS",b"TSIL",b"PMOC",b"MOCS",b"EMIT"):
   locations=[];p=0
   while True:
    p=s.find(tag,p)
    if p<0:break
    locations.append(p);p+=1
   print("M425_SPR_TAG tag=%s count=%d first=%s"%(tag.decode(),len(locations),repr([hex(x) for x in locations[:16]])))
 print("M425_LIMITATIONS rawAddressHitsNotXrefs=1 tableMetadataUnverified=1 objectMappingAssumed=1 noFixupDecode=1 noRuntimeTrace=1")
if __name__=="__main__":main()
