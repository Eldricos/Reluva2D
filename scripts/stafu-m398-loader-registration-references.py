#!/usr/bin/env python3
"""STAFU M398: static, read-only DOS4GW module registration reference survey."""
import argparse, hashlib, struct
from pathlib import Path
BASE=0xA4F48C
LIMIT=0x38194
DESCRIPTORS={"D32_INT31":0x323EC,"LINEXE_LOADER":0x32A78}
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def ascii_at(b,p):
 if not 0<=p<LIMIT:return None
 q=b.find(b"\0",p,min(LIMIT,p+100))
 if q<=p:return None
 s=b[p:q]
 return s.decode("ascii") if all(32<=v<127 for v in s) else None
def hits(b,pattern):
 result=[];start=0
 while True:
  p=b.find(pattern,start,LIMIT)
  if p<0:return result
  result.append(p);start=p+1
def window(b,p,n=32):
 lo=max(0,p-n);hi=min(LIMIT,p+n)
 return f"start=0x{lo:x} end=0x{hi:x} bytes={b[lo:hi].hex()}"
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes()
 print(f"M398_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LIMIT:LIMIT+2]!=b"LE":raise SystemExit("M398_ERROR missing MZ/LE")
 for name,off in DESCRIPTORS.items():
  print(f"M398_DESCRIPTOR name={name} file=0x{off:x} linear=0x{BASE+off:08x} raw={b[off:off+24].hex()}")
  patterns={
   "linear32":struct.pack("<I",BASE+off),
   "file32":struct.pack("<I",off),
   "linear16low":struct.pack("<H",(BASE+off)&65535),
   "file16low":struct.pack("<H",off&65535)
  }
  for kind,pat in patterns.items():
   found=hits(b,pat)
   print(f"M398_REFS name={name} encoding={kind} count={len(found)} offsets={','.join(hex(p) for p in found[:100])} truncated={int(len(found)>100)}")
   if kind.endswith("32"):
    for p in found[:32]:
     print(f"M398_REF_CONTEXT name={name} encoding={kind} file=0x{p:x} {window(b,p,24)}")
 # Discover four-byte pointers to descriptors or nearby fields. Relative
 # offsets are hypotheses only, not evidence of registration.
 for name,off in DESCRIPTORS.items():
  for delta in (-32,-24,-22,-16,-12,-8,-4,0,4,8,12,16,18,22,24,32):
   target=off+delta
   found=hits(b,struct.pack("<I",BASE+target))
   if found:
    print(f"M398_NEARBY_REF name={name} delta={delta:+d} target=0x{target:x} refs={','.join(hex(p) for p in found[:40])} total={len(found)}")
 # Inspect known name-list arrays and their exact 32-bit pointer values.
 for label,start,count in (("process_manager",0x31D64,5),("loader_names",0x338FE,4)):
  print(f"M398_NAME_ARRAY label={label} file=0x{start:x} count={count}")
  for i in range(count):
   p=start+4*i;v=u32(b,p);t=v-BASE
   print(f"M398_NAME_ENTRY array={label} index={i} file=0x{p:x} raw=0x{v:08x} target=0x{t:x} text={ascii_at(b,t)!r}")
 # Look for 32-bit affine pointers to descriptor names, and inspect
 # neighboring pointer-sized values without interpreting them as code.
 for name,off in DESCRIPTORS.items():
  name_target=u32(b,off+4)-BASE
  found=hits(b,struct.pack("<I",BASE+name_target))
  print(f"M398_NAME_POINTER_REFS name={name} stringFile=0x{name_target:x} count={len(found)}")
  for p in found[:30]:
   print(f"M398_NAME_REF_CONTEXT name={name} file=0x{p:x} {window(b,p,32)}")
 # Survey pointers into descriptor range, rather than just exact start.
 for name,off in DESCRIPTORS.items():
  inbound=[]
  for p in range(0,LIMIT-3):
   v=u32(b,p)-BASE
   if off<=v<off+24:
    inbound.append((p,v))
  print(f"M398_INBOUND_RANGE name={name} count={len(inbound)} refs={','.join(f'0x{p:x}->0x{v:x}' for p,v in inbound[:60])}")
 print("M398_LIMITATIONS noDisassembly=1 noExecution=1 registrationSemanticsUnproven=1 affineModelOnly=1")
if __name__=="__main__":main()
