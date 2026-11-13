#!/usr/bin/env python3
"""STAFU M403: static comparison of calls to DOS4GW far service 0080:5BA0.

Read-only, bounded pattern analysis. Does not establish runtime selector mapping.
"""
import argparse,hashlib,struct
from pathlib import Path
END=0x38194
TARGET=b"\x9a\xa0\x5b\x80\x00"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def s16(x):return x-65536 if x>=32768 else x
def matches(b,pat):
 p=0
 while True:
  p=b.find(pat,p,END)
  if p<0:break
  yield p;p+=1
def insn_hint(b,p):
 if p<0 or p>=END:return None
 if b[p]==0x1e:return (1,"push ds")
 if b[p]==0x06:return (1,"push es")
 if b[p]==0x68 and p+2<END:return (3,f"push imm16 0x{u16(b,p+1):04x}")
 if b[p:p+2] in (b"\xff\xb7",b"\xff\x76") and p+3<END:
  if b[p+1]==0xb7:return (4,f"push word [bx+0x{u16(b,p+2):04x}]")
  return (3,f"push word [bp{(b[p+2]-256 if b[p+2]>=128 else b[p+2]):+d}]")
 if b[p]==0x83 and p+2<END and b[p+1]==0xc4:return (3,f"add sp,{b[p+2]}")
 if b[p]==0x8e and p+1<END and b[p+1]==0xd8:return (2,"mov ds,ax")
 if b[p]==0xb8 and p+2<END:return (3,f"mov ax,0x{u16(b,p+1):04x}")
 if b[p]==0x0b and p+1<END and b[p+1]==0xd0:return (2,"or dx,ax")
 if b[p]==0x89 and p+1<END and b[p+1] in (0x46,0x56) and p+2<END:return (3,f"mov [bp+{b[p+2]:02x}],reg")
 if b[p]==0x5b:return (1,"pop bx")
 if b[p]==0x9a and p+4<END:return (5,f"call far {u16(b,p+3):04x}:{u16(b,p+1):04x}")
 if b[p]==0xe8 and p+2<END:return (3,f"call rel16 {s16(u16(b,p+1)):+d}")
 return None
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes()
 print(f"M403_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[END:END+2]!=b"LE":raise SystemExit("M403_ERROR MZ/LE header mismatch")
 sites=list(matches(b,TARGET))
 print(f"M403_SERVICE selector=0x0080 offset=0x5ba0 callCount={len(sites)} sites={','.join(hex(p) for p in sites)}")
 for p in sites:
  lo=max(0,p-40);hi=min(END,p+48)
  print(f"M403_CALL file=0x{p:x} before={b[lo:p].hex()} after={b[p+5:hi].hex()}")
  # Print exact preceding patterns ending at call, without guessing boundaries.
  for back in range(1,25):
   q=p-back
   if q<0:break
   hint=insn_hint(b,q)
   if hint and q+hint[0]<=p and (q+hint[0]==p or hint[1].startswith("push")):
    print(f"M403_PRE_HINT call=0x{p:x} file=0x{q:x} distance={back} instruction={hint[1]!r} bytes={b[q:q+hint[0]].hex()}")
  # Detect cleanup and return tests at fixed offsets, label as raw patterns.
  for delta in range(5,26):
   q=p+delta
   hint=insn_hint(b,q)
   if hint and (hint[1].startswith("add sp") or hint[1].startswith("mov [bp") or hint[1].startswith("or dx")):
    print(f"M403_POST_HINT call=0x{p:x} file=0x{q:x} delta={delta} instruction={hint[1]!r} bytes={b[q:q+hint[0]].hex()}")
  # Check specific conventional sequences after call, without claiming return ABI.
  post=b[p+5:p+24]
  for pattern,label in ((b"\x5b\x5b","pop_bx_twice"),(b"\x83\xc4\x04","add_sp_4"),(b"\x89\x46\xfc\x89\x56\xfe","store_ax_dx"),(b"\x0b\xd0","or_dx_ax")):
   q=post.find(pattern)
   if q>=0:print(f"M403_POST_PATTERN call=0x{p:x} file=0x{p+5+q:x} kind={label}")
 # Investigate other far calls using selector 0080 and distribution of offsets.
 targets={}
 for p in range(0,END-4):
  if b[p]==0x9a and u16(b,p+3)==0x80:
   off=u16(b,p+1);targets.setdefault(off,[]).append(p)
 print(f"M403_SELECTOR80 distinctOffsets={len(targets)} totalCallPatterns={sum(map(len,targets.values()))}")
 for off,locs in sorted(targets.items(),key=lambda x:(-len(x[1]),x[0]))[:35]:
  print(f"M403_SELECTOR80_TARGET offset=0x{off:04x} count={len(locs)} first={','.join(hex(x) for x in locs[:12])}")
 # Probe selected file offsets and arithmetic models, without treating them as validated.
 for name,at in (("raw_offset",0x5ba0),("real_mode_arithmetic",0x63a0)):
  lo=max(0,at-32);hi=min(END,at+128)
  print(f"M403_MAPPING_CANDIDATE model={name} file=0x{at:x} bytes={b[lo:hi].hex()}")
 print("M403_LIMITATIONS instructionBoundariesUnproven=1 callPatternMayBeData=1 noRuntimeSelectorBase=1 noVerifiedReturnABI=1")
if __name__=="__main__":main()
