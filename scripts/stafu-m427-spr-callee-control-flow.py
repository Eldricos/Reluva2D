#!/usr/bin/env python3
"""STAFU M427: static control-flow reconnaissance of candidate function 0x8FF50.

Read-only analysis of original OVL. Uses Capstone if installed; otherwise
prints hex windows and conservative opcode candidates. No runtime claims.
"""
import argparse, hashlib, struct
from pathlib import Path

LE=0x38194
def u32(b,p): return struct.unpack_from("<I",b,p)[0]
def s32(b,p): return struct.unpack_from("<i",b,p)[0]

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--start",type=lambda s:int(s,0),default=0x8ff50)
 ap.add_argument("--length",type=lambda s:int(s,0),default=0x300)
 args=ap.parse_args()
 b=args.ovl.read_bytes()
 print("M427_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M427_ERROR unexpected header")
 tab=LE+u32(b,LE+0x40); page=u32(b,LE+0x28); data=LE+u32(b,LE+0x80)
 if page!=4096 or u32(b,LE+0x44)<2:raise SystemExit("M427_ERROR unexpected LE object layout")
 objs=[]
 for i in range(2):
  size,base,flags,first,n=struct.unpack_from("<IIIII",b,tab+i*24)
  origin=data+(first-1)*page
  objs.append((origin,origin+size,base))
  print("M427_OBJECT index=%d start=0x%x end=0x%x base=0x%x assumedLinear=1"%(i+1,origin,origin+size,base))
 lo,hi,base=objs[0];start=args.start;end=min(start+args.length,hi)
 if not lo<=start<hi:raise SystemExit("M427_ERROR start outside Object 1")
 def addr(p):return base+(p-lo)
 print("M427_RANGE fileStart=0x%x fileEnd=0x%x object1Rva=0x%x assumedLinear=0x%x"%(start,end,start-lo,addr(start)))
 print("M427_BYTES hex=%s"%b[start:end].hex())
 # Report byte candidates without pretending they are instructions.
 for p in range(start,end):
  op=b[p]
  if op==0xe8 and p+5<=end:
   dest=p+5+s32(b,p+1)
   print("M427_RAW_CALL at=0x%x dest=0x%x inObject1=%d context=%s"%(p,dest,int(lo<=dest<hi),b[max(start,p-8):min(end,p+10)].hex()))
  elif op==0xe9 and p+5<=end:
   dest=p+5+s32(b,p+1)
   print("M427_RAW_JMP at=0x%x dest=0x%x inObject1=%d"%(p,dest,int(lo<=dest<hi)))
  elif op==0xc3:
   print("M427_RAW_RET at=0x%x"%p)
  elif op==0xc2 and p+3<=end:
   print("M427_RAW_RET_IMM at=0x%x stackBytes=%d"%(p,struct.unpack_from("<H",b,p+1)[0]))
 # Capstone is optional: if available, linear sweep is still not a CFG.
 try:
  from capstone import Cs, CS_ARCH_X86, CS_MODE_32
 except ImportError:
  print("M427_DISASSEMBLY available=0 reason=capstone_not_installed")
 else:
  md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=False
  print("M427_DISASSEMBLY available=1 mode=linear_sweep_not_cfg")
  for ins in md.disasm(b[start:end],addr(start)):
   f=start+(ins.address-addr(start))
   print("M427_INSN file=0x%x linear=0x%x size=%d mnemonic=%s operands=%s"%(f,ins.address,ins.size,ins.mnemonic,ins.op_str))
 # Explicitly examine candidates from M426; the first target is an
 # assumed-linear file offset, not an independently validated function.
 for target in (0x11efcc,0x7c2d0,0x900ab):
  if lo<=target<hi:
   print("M427_CALLEE_CANDIDATE file=0x%x object1Rva=0x%x bytes=%s"%(target,target-lo,b[target:min(target+160,hi)].hex()))
  else:print("M427_CALLEE_CANDIDATE file=0x%x outsideObject1=1"%target)
 print("M427_LIMITATIONS linearMappingAssumed=1 rawOpcodesMayBeOperands=1 optionalLinearSweepNotCFG=1 noFixupDecode=1 noRuntimeTrace=1 noLoaderConfirmed=1")
if __name__=="__main__":main()
