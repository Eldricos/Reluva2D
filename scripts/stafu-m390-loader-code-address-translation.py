#!/usr/bin/env python3
"""STAFU M390: audit 16-bit loader code-address translation hypotheses.

Read-only, requires Capstone (python -m pip install capstone). Scores candidate
constant shifts by x86 function-entry evidence, not mere decodability.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_16
TABLE=0x329f0;COUNT=17;LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes()
 print(f"M390_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[LE:LE+2]!=b"LE":raise SystemExit("M390_ERROR expected LE at 0x38194")
 md=Cs(CS_ARCH_X86,CS_MODE_16)
 names=["INIT","DEINIT","LOADER","UNLOAD","FREEMAP","REL","UNREL","CANLOAD","GETLOADTABLE","GETLOADNAME","GETERRSTR","LINEXE_LOADMODULE","LINEXE_GETPROCADDR","LINEXE_GETFLATREGS","LINEXE_FREEMODULE","LINEXE_GETMODHANDLE","XXXGET_OBJECT_MAP"]
 addrs=[u32(b,TABLE+i*8+4) for i in range(COUNT)]
 raw=[x-0x880000 for x in addrs]
 header=u32(b,0x3c) if len(b)>0x40 else 0
 mzheader=struct.unpack_from("<H",b,8)[0]*16
 print(f"M390_MZ headerBytes={mzheader} e_lfanew=0x{header:x} entryFile=0x{mzheader+struct.unpack_from('<H',b,22)[0]*16+struct.unpack_from('<H',b,20)[0]:x}")
 print("M390_NOTE loaderCodeAddressBase=0x880000 is candidate; a single shift need not describe segment mapping")
 def ins(p,n=5):
  if p<0 or p>=LE:return []
  return list(md.disasm(b[p:min(LE,p+32)],p,count=n))
 def features(p):
  if not 0<=p<LE:return (0,0,0)
  d=ins(p,4)
  if not d:return (0,0,0)
  # Prologues are suggestive only; optimized/assembly routines need not use them.
  head=b[p:p+5]
  strong=int(head.startswith((b"\x55\x8b\xec",b"\x55\x89\xe5",b"\x56\x57\x55\x8b\xec")))
  weak=int(head[0] in (0x55,0x56,0x57,0x53,0x51,0x52,0x1e,0x06,0x9c,0x8b,0xb8,0x50,0x31,0x33))
  prior=b[max(0,p-4):p]
  boundary=int(bool(prior) and prior[-1] in (0xc3,0xcb,0xcf,0xcc) or prior.endswith(b"\x8b\xe5\x5d\xc3"))
  return strong,weak,boundary
 candidates=[-0x400,-0x200,-0x100,-0x20,-0x10,-1,0,1,0x10,0x20,0x100,0x200,0x400]
 # Exhaustive shifts can surface false positives. Keep a clearly labeled
 # ranked sample and report scores rather than asserting a winner.
 ranking=[]
 for delta in range(-0x800,0x801):
  positions=[p+delta for p in raw]
  if any(p<0 or p>=LE for p in positions):continue
  f=[features(p) for p in positions]
  score=sum(5*a+2*c+z for a,z,c in f)
  ranking.append((score,delta,sum(x[0] for x in f),sum(x[2] for x in f)))
 ranking.sort(key=lambda x:(-x[0],abs(x[1]),x[1]))
 for score,delta,strong,bound in ranking[:20]:
  print(f"M390_SHIFT_RANK delta={delta:+d} score={score} strongPrologues={strong} precedingTerminator={bound}")
 for delta in candidates:
  f=[features(p+delta) for p in raw]
  print(f"M390_SHIFT_CHECK delta={delta:+d} strong={sum(x[0] for x in f)} weak={sum(x[1] for x in f)} priorTerminators={sum(x[2] for x in f)}")
 for i,(name,addr,p) in enumerate(zip(names,addrs,raw)):
  print(f"M390_ENTRY index={i} name={name} stored=0x{addr:x} subtract880000=0x{p:x} raw={b[p:p+12].hex()}")
  for delta in (-0x200,-0x10,-3,-2,-1,0,1,2,3,0x10,0x200):
   q=p+delta
   d=ins(q,4)
   fs=features(q)
   print(f"M390_ENTRY_WINDOW index={i} delta={delta:+d} file=0x{q:x} strong={fs[0]} priorTerminator={fs[2]} instructions={' ; '.join(f'{x.mnemonic} {x.op_str}' for x in d)!r}")
 # Check whether the stored values are 16:16 segment:offset packed
 # (segment in high 16 bits), rather than ordinary linear pointers.
 for i,(name,addr) in enumerate(zip(names,addrs)):
  seg=(addr>>16)&0xffff;off=addr&0xffff
  real=seg*16+off
  print(f"M390_FARPTR index={i} name={name} segment=0x{seg:04x} offset=0x{off:04x} realModeLinear=0x{real:x} withinFile={int(real<len(b))}")
 print("M390_LIMITATIONS heuristicProloguesNotProof=1 codeEntryPointsNotEstablished=1 runtimeSelectorsNotObserved=1")
if __name__=="__main__":main()
