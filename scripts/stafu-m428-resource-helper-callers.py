#!/usr/bin/env python3
"""STAFU M428: find validated direct callers of known resource helpers.

Static, read-only, Capstone-based instruction-boundary analysis. The
helper addresses are hypotheses from M327-M348, not proven SPR decoder IDs.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=0x38194
TARGETS={
 "stream_position_wrapper":563123,
 "position_manager":517688,
 "resource_read_state":566260,
 "resource_lifecycle_a":566552,
 "resource_lifecycle_b":566876,
}
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--radius",type=lambda s:int(s,0),default=0x50)
 ap.add_argument("--max-per-target",type=int,default=30)
 args=ap.parse_args();b=args.ovl.read_bytes()
 print("M428_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M428_ERROR header")
 tab=LE+u32(b,LE+0x40);data=LE+u32(b,LE+0x80);page=u32(b,LE+0x28)
 if page!=4096:raise SystemExit("M428_ERROR page size")
 size,base,flags,first,npages=struct.unpack_from("<IIIII",b,tab)
 start=data+(first-1)*page;end=min(start+size,len(b))
 print("M428_OBJECT1 start=0x%x end=0x%x base=0x%x assumedLinear=1"%(start,end,base))
 try:
  from capstone import Cs,CS_ARCH_X86,CS_MODE_32
  from capstone.x86_const import X86_OP_IMM
 except ImportError:
  raise SystemExit("M428_ERROR capstone required: python -m pip install capstone")
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 # Sweep from known function prologues where possible. A single global
 # linear sweep can lose alignment after inline data; all matches need
 # further CFG validation.
 insns=list(md.disasm(b[start:end],base))
 index={start+(i.address-base):i for i in insns}
 print("M428_DISASM count=%d coveredEnd=0x%x mode=linearSweepNotCFG"%(len(insns),max(index) if index else start))
 targets={k:v for k,v in TARGETS.items() if start<=v<end}
 for name,p in targets.items():
  print("M428_TARGET name=%s file=0x%x linear=0x%x bytes=%s"%(name,p,base+p-start,b[p:p+48].hex()))
 calls=collections.defaultdict(list)
 for p,ins in index.items():
  if ins.mnemonic!="call" or not ins.operands or ins.operands[0].type!=X86_OP_IMM:continue
  dest=start+(ins.operands[0].imm-base)
  for name,target in targets.items():
   if dest==target:calls[name].append((p,ins))
 for name,target in targets.items():
  hits=calls[name]
  print("M428_CALLERS name=%s target=0x%x count=%d"%(name,target,len(hits)))
  for p,ins in hits[:args.max_per_target]:
   a=max(start,p-args.radius);z=min(end,p+args.radius)
   print("M428_CALLER name=%s file=0x%x object1Rel=0x%x contextStart=0x%x hex=%s"%(
    name,p,p-start,a,b[a:z].hex()))
   before=[(q,j) for q,j in index.items() if p-36<=q<=p+36]
   for q,j in before:
    print("M428_NEAR_INSN file=0x%x mnemonic=%s operands=%s"%(q,j.mnemonic,j.op_str))
  if len(hits)>args.max_per_target:print("M428_TRUNCATED name=%s remaining=%d"%(name,len(hits)-args.max_per_target))
 # Also find direct calls to helper at site of M426, to provide a
 # contrast with known text output path.
 for p in (0x8ff50,0x9f035):
  print("M428_REFERENCE_CONTEXT file=0x%x bytes=%s"%(p,b[p:p+64].hex()))
 print("M428_LIMITATIONS targetAddressesHistorical=1 linearSweepNotCFG=1 indirectCallsNotFound=1 relocationSemanticsNotProven=1 noRuntimeTrace=1 noDecoderIdentified=1")
if __name__=="__main__":main()
