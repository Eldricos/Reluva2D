#!/usr/bin/env python3
"""STAFU M404: compare paired DOS4GW services 0080:5BA0 and 0080:5A20.

Read-only static pattern study. Call-site bytes do not establish runtime control
flow, selector bases, function signatures, or module-lookup semantics.
"""
import argparse, hashlib, struct
from pathlib import Path

LIMIT=0x38194
SERVICES=(0x5BA0,0x5A20)
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def pattern(off):return b"\x9a"+struct.pack("<HH",off,0x80)
def locate(b,needle):
 p=0
 while True:
  p=b.find(needle,p,LIMIT)
  if p<0:break
  yield p
  p+=1
def segment(b,p,lo=48,hi=72):
 a=max(0,p-lo);z=min(LIMIT,p+hi)
 return f"start=0x{a:x} stop=0x{z:x} bytes={b[a:z].hex()}"
def arg_patterns(b,p):
 # Backward matching of complete push patterns ending at the call. A
 # sequential chain is reliable only as a byte-level candidate.
 cur=p
 out=[]
 for _ in range(10):
  candidates=[]
  if cur>=1 and b[cur-1] in (0x1e,0x06,0x50,0x52,0x53,0x56,0x57):
   names={0x1e:"push_ds",0x06:"push_es",0x50:"push_ax",0x52:"push_dx",0x53:"push_bx",0x56:"push_si",0x57:"push_di"}
   candidates.append((cur-1,names[b[cur-1]]))
  if cur>=3 and b[cur-3]==0x68:candidates.append((cur-3,f"push_imm16_0x{u16(b,cur-2):04x}"))
  if cur>=3 and b[cur-3:cur-1]==b"\xff\x76":
   d=b[cur-1];candidates.append((cur-3,f"push_bp_disp8_{d-256 if d>=128 else d:+d}"))
  if cur>=4 and b[cur-4:cur-2]==b"\xff\xb7":
   candidates.append((cur-4,f"push_bx_disp16_0x{u16(b,cur-2):04x}"))
  if not candidates:break
  # Prefer the longest recognized complete instruction.
  at,label=min(candidates,key=lambda x:x[0])
  out.append((at,label))
  cur=at
 return list(reversed(out))
def cleanup(b,p):
 q=p+5
 if b[q:q+3]==b"\x83\xc4\x04":return "add_sp_4"
 if b[q:q+3]==b"\x83\xc4\x08":return "add_sp_8"
 if b[q:q+2]==b"\x5b\x5b":return "pop_bx_twice"
 return "other"
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 args=ap.parse_args();b=args.ovl.read_bytes()
 print(f"M404_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LIMIT:LIMIT+2]!=b"LE":raise SystemExit("M404_ERROR MZ/LE mismatch")
 sites={off:list(locate(b,pattern(off))) for off in SERVICES}
 for off in SERVICES:
  print(f"M404_SERVICE offset=0x{off:04x} count={len(sites[off])} sites={','.join(hex(p) for p in sites[off])}")
  for p in sites[off]:
   pushes=arg_patterns(b,p)
   print(f"M404_CALL service=0x{off:04x} file=0x{p:x} cleanup={cleanup(b,p)} pushes={';'.join(f'0x{q:x}:{label}' for q,label in pushes) or 'none'}")
   print(f"M404_CONTEXT service=0x{off:04x} file=0x{p:x} {segment(b,p)}")
   post=b[p+5:p+35]
   for needle,label in ((b"\x89\x46\xfc\x89\x56\xfe","store_ax_dx_bp_minus4"),
                        (b"\x0b\xd0","or_dx_ax"),(b"\x83\xc4\x08","add_sp_8"),
                        (b"\x83\xc4\x04","add_sp_4")):
    at=post.find(needle)
    if at>=0:print(f"M404_POST service=0x{off:04x} call=0x{p:x} file=0x{p+5+at:x} kind={label}")
 # Only list pairs in nearby file order, not proof that one executes after the other.
 for p in sites[0x5BA0]:
  following=[q for q in sites[0x5A20] if p<q<=p+192]
  for q in following:
   middle=b[p+5:q]
   print(f"M404_PAIR first=0x{p:x} second=0x{q:x} gap={q-(p+5)} middle={middle.hex()}")
   for needle,label in ((b"\xff\x76\xfc","push_saved_ax"),(b"\xff\x76\xfe","push_saved_dx"),
                        (b"\x50","push_ax"),(b"\x52","push_dx"),
                        (b"\x0b\xd0","or_dx_ax")):
    offsets=[hex(p+5+i) for i in range(len(middle)-len(needle)+1) if middle[i:i+len(needle)]==needle]
    if offsets:print(f"M404_PAIR_HINT first=0x{p:x} second=0x{q:x} kind={label} locations={','.join(offsets[:20])}")
 # Selected mapping hypotheses, not asserted runtime addresses.
 for off in SERVICES:
  for model,pos in (("raw_offset",off),("real_mode_selector_x16",0x80*16+off)):
   print(f"M404_MAPPING service=0x{off:04x} model={model} file=0x{pos:x} {segment(b,pos,24,88)}")
 print("M404_LIMITATIONS noValidatedSelectorMapping=1 bytePatternAnalysis=1 pairsNotControlFlowProof=1 noVerifiedABI=1")
if __name__=="__main__":main()
