#!/usr/bin/env python3
"""STAFU M402: inspect name-table call sites, far targets, and mapping hypotheses.

Static byte analysis only. A segment:offset value is not a file address
without independently validated segment mapping.
"""
import argparse, hashlib, struct
from pathlib import Path

LE=0x38194
BASE=0xA4F48C
SITES=(("loader_names",0x2d6bd),("process_first",0x24c5a),("process_second",0x24c7c),("process_fallback",0x24c91))
def word(b,p): return struct.unpack_from("<H",b,p)[0]
def signed(w):return w-0x10000 if w&0x8000 else w
def dump(b,p,before=32,after=64):
 lo=max(0,p-before);hi=min(len(b),p+after)
 return f"start=0x{lo:x} stop=0x{hi:x} bytes={b[lo:hi].hex()}"
def call_details(b,p):
 op=b[p]
 if op==0x9a:
  return ("far_imm16_16",f"selector=0x{word(b,p+3):04x} offset=0x{word(b,p+1):04x}",5)
 if op==0xe8:
  return ("near_rel16",f"rel16={signed(word(b,p+1))} fileArithmeticTarget=0x{p+3+signed(word(b,p+1)):x}",3)
 return ("unexpected",f"opcode=0x{op:02x}",1)
def main():
 a=argparse.ArgumentParser(description=__doc__)
 a.add_argument("ovl",type=Path)
 args=a.parse_args()
 b=args.ovl.read_bytes()
 print(f"M402_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M402_ERROR invalid expected MZ/LE")
 for name,p in SITES:
  kind,desc,n=call_details(b,p)
  print(f"M402_CALL name={name} file=0x{p:x} kind={kind} {desc} instruction={b[p:p+n].hex()}")
  print(f"M402_CALL_CONTEXT name={name} {dump(b,p,48,64)}")
  # Explicitly report stack-related encodings in the preceding bounded
  # window. These are candidates, not a linear disassembly.
  for at in range(max(0,p-24),p):
   if b[at:at+2]==b"\xff\xb7" and at+4<=p:
    print(f"M402_ARG_HINT name={name} file=0x{at:x} kind=push_bx_disp16 disp=0x{word(b,at+2):04x}")
   if b[at]==0x68 and at+3<=p:
    print(f"M402_ARG_HINT name={name} file=0x{at:x} kind=push_imm16 value=0x{word(b,at+1):04x}")
   if b[at]==0x1e:
    print(f"M402_ARG_HINT name={name} file=0x{at:x} kind=possible_push_ds")
  if kind=="near_rel16":
   target=p+3+signed(word(b,p+1))
   if 0<=target<LE:
    print(f"M402_NEAR_TARGET name={name} candidate=0x{target:x} {dump(b,target,24,112)}")
  elif kind=="far_imm16_16":
   sel=word(b,p+3);off=word(b,p+1)
   # Several mutually exclusive address hypotheses. No candidate
   # is declared valid solely because its bytes look like code.
   models=(("offset_as_file",off),
           ("selector_times_16_plus_offset",sel*16+off),
           ("selector_times_65536_plus_offset",sel*65536+off),
           ("affine_segment_A80000_plus_offset",0xA80000+off))
   for model,value in models:
    valid=0<=value<LE
    print(f"M402_FAR_MAPPING name={name} model={model} calculated=0x{value:x} withinPrefix={int(valid)}")
    if valid:print(f"M402_FAR_WINDOW name={name} model={model} {dump(b,value,24,96)}")
 # Scan far immediate call sites sharing the exact selector:offset,
 # and near-call operands sharing the two near-call displacement targets.
 sel=0x80;off=0x5ba0
 pattern=b"\x9a"+struct.pack("<HH",off,sel)
 matches=[];pos=0
 while True:
  pos=b.find(pattern,pos,LE)
  if pos<0:break
  matches.append(pos);pos+=1
 print(f"M402_FAR_XREF selector=0x{sel:04x} offset=0x{off:04x} count={len(matches)} locations={','.join(hex(p) for p in matches[:100])}")
 for p in matches[:25]:print(f"M402_FAR_XREF_CONTEXT file=0x{p:x} {dump(b,p,16,24)}")
 # Check all near rel16 byte patterns that would target the same
 # file offset under the *unverified* file-arithmetic model.
 for name,call in SITES[1:]:
  target=call+3+signed(word(b,call+1))
  hits=[]
  for p in range(0,LE-2):
   if b[p]==0xe8 and p+3+signed(word(b,p+1))==target:hits.append(p)
  print(f"M402_NEAR_XREF name={name} candidateTarget=0x{target:x} count={len(hits)} first={','.join(hex(x) for x in hits[:60])}")
 print("M402_LIMITATIONS noRuntimeSelectorBase=1 noValidatedCSMapping=1 heuristicInstructionBoundaries=1 noCallSemanticsProof=1")
if __name__=="__main__":main()
