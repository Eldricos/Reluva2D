#!/usr/bin/env python3
"""STAFU M407: inspect indirect far-call neighborhoods for resolved loader exports.

Read-only static evidence. x86 decoding is deliberately limited to recognized
patterns; a byte-pattern match is not proof of reachable executable code.
"""
import argparse, hashlib, struct
from pathlib import Path

LIMIT=0x38194
EXPORTS={
 "VMFABIND":0x1A04,"VMFMA":0x1A10,"VMFILEIO":0x1A08,
 "DVX_Global_Seg_Associate":0x3290,"DVX_Global_Seg_Alloc":0x32AC,
 "DVX_Global_Seg_Broadcast":0x32B0,"DVX_Dos4gx_Get_Option":0x32A8,
 "VMALOCINST":0x3294,"VSUPERRESIZE":0x3298,"VSUPERFREE":0x329C,
 "DOS4G_GETENV":0x32A0,"DOS4G_GET_APPPATH":0x32A4,
}
def all_hits(b,pat):
 p=0
 while True:
  p=b.find(pat,p,LIMIT)
  if p<0:break
  yield p
  p+=1
def word(n):return struct.pack("<H",n)
def decode_before(b,p):
 # Backwards candidate argument patterns, without assuming arbitrary boundaries.
 out=[]
 for distance in range(1,41):
  q=p-distance
  if q<0:break
  if b[q]==0x68 and q+3<=p:out.append((q,f"push_imm16_0x{int.from_bytes(b[q+1:q+3],'little'):04x}"))
  if b[q:q+3]==b"\xff\x76"+b[q+2:q+3] and q+3<=p:
   d=b[q+2];out.append((q,f"push_bp_{d-256 if d>=128 else d:+d}"))
  if b[q:q+2]==b"\xff\x36" and q+4<=p:out.append((q,f"push_ds_absolute_0x{int.from_bytes(b[q+2:q+4],'little'):04x}"))
  if b[q] in (0x06,0x1e,0x50,0x51,0x52,0x53,0x56,0x57) and q+1<=p and distance<=16:
   out.append((q,{0x06:"push_es",0x1e:"push_ds",0x50:"push_ax",0x51:"push_cx",0x52:"push_dx",0x53:"push_bx",0x56:"push_si",0x57:"push_di"}[b[q]]))
 return sorted(out)
def scan_segment(b,p):
 start=max(0,p-96)
 found=[]
 for q in range(start,p):
  if b[q:q+5]==b"\xb8\xa8\x00\x8e\xd8":found.append((q,"mov_ax_00a8_mov_ds_ax"))
  if b[q:q+2]==b"\x1f":found.append((q,"pop_ds"))
  if b[q:q+2]==b"\x8e\xd8":found.append((q,"mov_ds_ax"))
  if b[q:q+2]==b"\x8e\xc0":found.append((q,"mov_es_ax"))
 return found
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print(f"M407_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LIMIT:LIMIT+2]!=b"LE":raise SystemExit("M407_ERROR MZ/LE layout mismatch")
 total=0
 for name,disp in EXPORTS.items():
  pattern=b"\xff\x1e"+word(disp)
  hits=list(all_hits(b,pattern))
  print(f"M407_EXPORT name={name} dsOffset=0x{disp:04x} callCount={len(hits)} sites={','.join(hex(p) for p in hits) or 'none'}")
  for p in hits:
   total+=1
   pre=b[max(0,p-48):p]
   post=b[p+4:min(LIMIT,p+44)]
   print(f"M407_SITE name={name} file=0x{p:x} instruction=ff1e{disp:04x} before={pre.hex()} after={post.hex()}")
   for q,kind in decode_before(b,p):
    print(f"M407_ARG_CANDIDATE name={name} call=0x{p:x} file=0x{q:x} kind={kind}")
   for q,kind in scan_segment(b,p):
    print(f"M407_SEGMENT_CANDIDATE name={name} call=0x{p:x} file=0x{q:x} kind={kind}")
   q=p+4
   if b[q:q+3]==b"\x83\xc4\x04":cleanup="add_sp_4"
   elif b[q:q+3]==b"\x83\xc4\x08":cleanup="add_sp_8"
   elif b[q:q+3]==b"\x83\xc4\x0c":cleanup="add_sp_12"
   elif b[q:q+2]==b"\x5b\x5b":cleanup="pop_bx_twice"
   else:cleanup="not_immediate"
   print(f"M407_POST name={name} call=0x{p:x} cleanup={cleanup}")
 print(f"M407_TOTAL exports={len(EXPORTS)} indirectFarCallMatches={total}")
 print("M407_LIMITATIONS instructionBoundariesNotProven=1 reachabilityNotProven=1 runtimeDsUnverified=1 argCandidatesMayOverlap=1")
if __name__=="__main__":main()
