#!/usr/bin/env python3
"""STAFU M394: read-only comparison of candidate DOS/4GW loader registration structures."""
import argparse,hashlib,struct
from pathlib import Path
LE=0x38194;BASE=0xA4F48C
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def ascii_at(b,p):
 if p<0 or p>=LE:return None
 q=b.find(b"\0",p,min(LE,p+100))
 if q<=p:return None
 z=b[p:q]
 return z.decode("ascii") if all(32<=c<127 for c in z) else None
def findall(b,needle,lo=0,hi=LE):
 out=[];p=lo
 while True:
  p=b.find(needle,p,hi)
  if p<0:break
  out.append(p);p+=1
 return out
def fmt(v):return ",".join(f"0x{x:x}" for x in v)
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes()
 print(f"M394_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M394_ERROR expected MZ and LE")
 for name,start,end in (("linexe_descriptor",0x32a70,0x32ab0),("linexe_following",0x32a78,0x32afc),("registry_strings",0x33890,0x33970),("registry_context",0x33840,0x339c0),("loader_early",0x31d40,0x31e10)):
  print(f"M394_REGION name={name} start=0x{start:x} end=0x{end:x} hex={b[start:end].hex()}")
  for p in range(start,end-3,4):
   v=u32(b,p);resolved=ascii_at(b,v-BASE)
   if v or resolved:
    print(f"M394_FIELD region={name} file=0x{p:x} u32=0x{v:08x} low16=0x{v&65535:04x} high16=0x{v>>16:04x} nameViaBase={resolved!r}")
 for name in ("EXP_LOADER","SEGEXE_LOADER","LINEXE_LOADER","DVX_Which_Loader","DVX_Process_Manager","LINEXE_MOD_INFO_SEG"):
  for p in findall(b,name.encode()+b"\0"):
   ptr=BASE+p
   refs=findall(b,struct.pack("<I",ptr))
   print(f"M394_STRING name={name!r} file=0x{p:x} affine=0x{ptr:08x} refs={fmt(refs)}")
   for r in refs:
    print(f"M394_REF_CONTEXT name={name!r} pointerFile=0x{r:x} before24={b[max(0,r-24):r].hex()} after40={b[r+4:r+44].hex()}")
 # Inspect candidate pointer array around 0x33902, and adjacent words.
 for p in range(0x338f0,0x33950,2):
  print(f"M394_REGISTRY_WORD file=0x{p:x} u16=0x{u16(b,p):04x}")
 for p in range(0x338f0,0x33950,4):
  v=u32(b,p)
  print(f"M394_REGISTRY_DWORD file=0x{p:x} u32=0x{v:08x} nameViaBase={ascii_at(b,v-BASE)!r}")
 # For each named module, compare all adjacent candidate references using
 # a fixed ±32-byte window. Only exact matches are considered evidence.
 for name in ("EXP_LOADER","SEGEXE_LOADER","LINEXE_LOADER"):
  for p in findall(b,name.encode()+b"\0"):
   refs=findall(b,struct.pack("<I",BASE+p))
   for r in refs:
    print(f"M394_MODULE_REFERENCE module={name} string=0x{p:x} ref=0x{r:x} aligned4={int(r%4==0)} surroundingDwords=" +
      ",".join(f"0x{u32(b,q):08x}" for q in range(max(0,r-16),min(LE-3,r+24),4)))
 # Scan for exact descriptor signature fields near LINEXE descriptor;
 # occurrences do not prove shared struct semantics.
 for label,v in (("versionLike",0x00020001),("linexeName",0x00A81E6E),("candidateName",0x00A81E7C),("field15",0x0000000F),("fieldFFFF",0xFFFF0000)):
  matches=findall(b,struct.pack("<I",v))
  print(f"M394_SIGNATURE label={label} value=0x{v:08x} count={len(matches)} positions={fmt(matches[:100])}")
 print("M394_LIMITATIONS candidateDescriptorUnverified=1 affineMappingNotRuntimeBase=1 noCodeExecution=1")
if __name__=="__main__":main()
