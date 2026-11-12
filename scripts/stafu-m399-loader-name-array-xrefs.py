#!/usr/bin/env python3
"""STAFU M399: read-only module-name array cross-reference analysis."""
import argparse,hashlib,struct
from pathlib import Path
BASE=0xA4F48C
END=0x38194
ARRAYS=(("process_manager",0x31D64,5),("loader_names",0x338FE,4))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def occurrences(b,needle):
 out=[];start=0
 while True:
  p=b.find(needle,start,END)
  if p<0:return out
  out.append(p);start=p+1
def window(b,p,n=24):
 a=max(0,p-n);z=min(END,p+n+8)
 return f"start=0x{a:x} end=0x{z:x} hex={b[a:z].hex()}"
def string(b,p):
 if p<0 or p>=END:return None
 q=b.find(b"\x00",p,min(END,p+100))
 if q<=p:return None
 v=b[p:q]
 return v.decode("ascii") if all(32<=c<=126 for c in v) else None
def classify(b,p,kind):
 # Only instruction-form hints; these do not prove a disassembly boundary.
 prior=b[max(0,p-2):p]
 if kind=="linear32":
  if p>=1 and b[p-1] in (0x68,0xA1,0xA3):return "possible_push_or_moffs_imm32"
  if p>=1 and 0xB8<=b[p-1]<=0xBF:return "possible_mov_reg_imm32"
  if p>=2 and b[p-2] in (0x8B,0x89,0x8D,0xC7):return "possible_modrm_disp32"
 if kind=="rel32":
  if p>=1 and b[p-1] in (0xE8,0xE9):return "possible_call_or_jmp_rel32"
 return "unclassified"
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 args=ap.parse_args()
 b=args.ovl.read_bytes()
 print(f"M399_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[END:END+2]!=b"LE":raise SystemExit("M399_ERROR missing MZ/LE")
 for label,start,count in ARRAYS:
  print(f"M399_ARRAY name={label} file=0x{start:x} affine=0x{BASE+start:08x} count={count}")
  for i in range(count):
   at=start+4*i;value=u32(b,at);target=value-BASE
   print(f"M399_ARRAY_ENTRY name={label} index={i} file=0x{at:x} pointer=0x{value:08x} target=0x{target:x} string={string(b,target)!r}")
  for kind,value in (("linear32",BASE+start),("file32",start)):
   found=occurrences(b,struct.pack("<I",value))
   print(f"M399_ABSOLUTE_REFS name={label} kind={kind} value=0x{value:08x} count={len(found)} offsets={','.join(hex(x) for x in found[:100])}")
   for p in found[:40]:
    print(f"M399_REF_CONTEXT name={label} kind={kind} source=0x{p:x} hint={classify(b,p,kind)} {window(b,p)}")
  # Scan for x86 near rel32 instructions whose computed file-relative
  # targets hit array start or individual entries. Rel32 uses next-IP.
  # The resulting locations are candidates only, not validated code.
  for target in [start]+[start+4*i for i in range(1,count)]:
   found=[]
   for op in (0xE8,0xE9):
    for p in range(0,END-5):
     if b[p]!=op:continue
     disp=struct.unpack_from("<i",b,p+1)[0]
     if p+5+disp==target:
      found.append((p,op))
   print(f"M399_REL32_CANDIDATES name={label} target=0x{target:x} count={len(found)}")
   for p,op in found[:20]:
    print(f"M399_REL32_CONTEXT name={label} target=0x{target:x} source=0x{p:x} opcode=0x{op:02x} {window(b,p)}")
  # Low16 matches can be common. Show counts and instruction-like
  # neighborhoods only, never classify them as true references.
  low=struct.pack("<H",(BASE+start)&0xFFFF)
  found=occurrences(b,low)
  print(f"M399_LOW16 name={label} value=0x{(BASE+start)&65535:04x} count={len(found)} first={','.join(hex(x) for x in found[:40])}")
  for p in found[:20]:
   print(f"M399_LOW16_CONTEXT name={label} source=0x{p:x} {window(b,p,12)}")
  # Find references to the first and last array element strings, which
  # can reveal users of entries without a pointer to the array itself.
  for i in sorted(set((0,count-1))):
   val=u32(b,start+4*i)
   found=occurrences(b,struct.pack("<I",val))
   print(f"M399_ELEMENT_REFS name={label} index={i} value=0x{val:08x} count={len(found)} offsets={','.join(hex(x) for x in found[:50])}")
 print("M399_LIMITATIONS staticBytesOnly=1 noInstructionBoundaryProof=1 rel32FileOffsetHeuristic=1 runtimeAddressingUnknown=1")
if __name__=="__main__":main()
