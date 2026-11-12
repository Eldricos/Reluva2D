#!/usr/bin/env python3
"""STAFU M400: inspect segmented table-access candidates without assuming DS."""
import argparse,hashlib,struct
from pathlib import Path
LE=0x38194
BASE=0xA4F48C
SITES=(("loader_names",0x2D6BB,0x338FE,0x2D8A),("process_manager_a",0x24C41,0x31D64,0x11F0),("process_manager_b",0x24C54,0x31D64,0x11F0),("process_manager_c",0x24C76,0x31D64,0x11F0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def signed8(v):return v-256 if v>=128 else v
def decode16(b,start,end):
 """Small explicit 16-bit pattern recognizer, not a general disassembler."""
 p=start
 while p<end:
  op=b[p];seg=None;rep=None;operand32=False;prefixstart=p
  while op in (0x26,0x2e,0x36,0x3e,0x66,0x67) and p+1<end:
   if op in (0x26,0x2e,0x36,0x3e):seg={0x26:"ES",0x2e:"CS",0x36:"SS",0x3e:"DS"}[op]
   if op==0x66:operand32=True
   p+=1;op=b[p]
  size=1;label=f"db 0x{op:02x}";disp=None
  if op==0xc1 and p+2<end and b[p+1]==0xe3:
   size=3;label=f"shl bx, {b[p+2]}"
  elif op==0x8b and p+3<end and b[p+1] in (0x87,0x9f,0x97):
   size=4;disp=u16(b,p+2);label=f"mov reg, [{seg or 'DS'}:bx+0x{disp:04x}]"
  elif op==0xff and p+3<end and b[p+1] in (0xb7,0x97):
   size=4;disp=u16(b,p+2);label=f"push word [{seg or 'DS'}:bx+0x{disp:04x}]" if b[p+1]==0xb7 else f"call word [{seg or 'DS'}:bx+0x{disp:04x}]"
  elif op==0x0b and p+3<end and b[p+1]==0x87:
   size=4;disp=u16(b,p+2);label=f"or ax, [{seg or 'DS'}:bx+0x{disp:04x}]"
  elif op==0x8b and p+2<end and b[p+1]==0x5e:
   size=3;label=f"mov bx, [SS:bp{signed8(b[p+2]):+d}]"
  elif op==0x9a and p+4<end:
   size=5;label=f"call far 0x{u16(b,p+3):04x}:0x{u16(b,p+1):04x}"
  elif op in (0x68,) and p+2<end:
   size=3;label=f"push 0x{u16(b,p+1):04x}"
  elif op in (0x50,0x53,0x57,0x5b,0x5e,0x58,0x1e,0x06):
   label={0x50:"push ax",0x53:"push bx",0x57:"push di",0x5b:"pop bx",0x5e:"pop si",0x58:"pop ax",0x1e:"push ds",0x06:"push es"}[op]
  elif op==0x83 and p+2<end:
   size=3;label=f"grp83 modrm=0x{b[p+1]:02x} imm8=0x{b[p+2]:02x}"
  elif op in (0x74,0x75,0x73,0x72,0xeb) and p+1<end:
   size=2;label=f"jcc/short 0x{p+2+signed8(b[p+1]):x}"
  elif op==0xe8 and p+2<end:
   size=3;d=u16(b,p+1);label=f"call near rel16 target=0x{(p+3+(d-65536 if d>=32768 else d)):x}"
  consumed=p-prefixstart+size
  yield prefixstart,consumed,label,disp,seg,operand32
  p+=size
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes()
 print(f"M400_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M400_ERROR expected MZ and LE")
 for name,site,array,offset in SITES:
  lo=max(0,site-24);hi=min(LE,site+56)
  print(f"M400_SITE name={name} file=0x{site:x} arrayFile=0x{array:x} arrayAffine=0x{BASE+array:08x} observedOffset=0x{offset:04x} bytes={b[lo:hi].hex()}")
  # Two address models: file-relative and affine linear address.
  for model,target in (("file",array),("affine",BASE+array)):
   delta=target-offset
   print(f"M400_BASE name={name} model={model} candidateBase=0x{delta:x} paragraphAligned={int(delta%16==0)} low16=0x{delta&65535:04x} expectedArray=0x{target:x}")
  # Decode from several nearby starts; only report recognized table
  # patterns and exact bytes, not speculative full control flow.
  for begin in (site-10,site-6,site-4,site-2,site):
   if begin<0:continue
   for p,size,insn,disp,seg,operand32 in decode16(b,begin,min(LE,site+25)):
    if disp is not None or (p<=site<p+size):
     print(f"M400_DECODE start=0x{begin:x} at=0x{p:x} size={size} bytes={b[p:p+size].hex()} mnemonic={insn!r} segment={seg or 'default'} disp={hex(disp) if disp is not None else '-'} operand32={int(operand32)}")
  # Recognize indexed pair access with optional segment override,
  # based on exact byte pattern, independently of decoder starts.
  for p in range(max(0,site-32),min(LE-8,site+32)):
   if b[p:p+2] == b"\xff\xb7" and u16(b,p+2)==offset and b[p+4:p+6]==b"\xff\xb7" and u16(b,p+6)==((offset+2)&65535):
    print(f"M400_PAIR name={name} file=0x{p:x} order=low_then_high bytes={b[p:p+8].hex()}")
   if b[p:p+2] == b"\xff\xb7" and u16(b,p+2)==((offset+2)&65535) and b[p+4:p+6]==b"\xff\xb7" and u16(b,p+6)==offset:
    print(f"M400_PAIR name={name} file=0x{p:x} order=high_then_low bytes={b[p:p+8].hex()}")
  # Scan for segment-register setup hints within bounded context.
  for p in range(max(0,site-96),min(LE-1,site+24)):
   if b[p] in (0x8e,0x1e,0x07,0x0e,0x06,0x17,0x1f):
    print(f"M400_SEGMENT_HINT name={name} file=0x{p:x} bytes={b[p:min(LE,p+5)].hex()} opcode=0x{b[p]:02x}")
 print("M400_LIMITATIONS patternDecoderNotFullDisassembler=1 instructionBoundariesUnproven=1 segmentBasesHypothetical=1 noRuntimeExecution=1")
if __name__=="__main__":main()
