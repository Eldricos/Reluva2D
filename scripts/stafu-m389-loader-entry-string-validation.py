#!/usr/bin/env python3
"""M389: validate DOS/4GW table names and 16-bit code-entry hypotheses.

Read-only. Capstone optional: pip install capstone. Candidate address bases
are hypotheses; instruction decoding does not prove control-flow reachability.
"""
import argparse,collections,hashlib,struct
from pathlib import Path
TABLE=0x329f0; N=17; LIMIT=0x38194
def dword(b,p):return struct.unpack_from("<I",b,p)[0]
def string_start(b,p):
 if p<0 or p>=LIMIT or (p and b[p-1]!=0):return None
 q=b.find(b"\0",p,min(LIMIT,p+100))
 if q<=p:return None
 v=b[p:q]
 if len(v)<3 or not all(32<=x<127 for x in v):return None
 return v.decode("ascii")
def scan_starts(b):
 result={}
 p=0
 while p<LIMIT:
  if p==0 or b[p-1]==0:
   v=string_start(b,p)
   if v:result[p]=v;p+=len(v)+1;continue
  p+=1
 return result
def disassemble(b,p,capstone):
 if not (0<=p<LIMIT):return "out-of-range"
 if capstone is None:return "capstone-unavailable"
 md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_16)
 md.detail=False
 ins=list(md.disasm(b[p:min(p+32,LIMIT)],p,count=6))
 return " | ".join(f"{x.address:05x}:{x.mnemonic} {x.op_str}" for x in ins) if ins else "undecodable"
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes()
 print(f"M389_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[LIMIT:LIMIT+2]!=b"LE":raise SystemExit("M389_ERROR expected LE header")
 try:
  import capstone
  print(f"M389_DISASSEMBLER capstone={getattr(capstone,'__version__','available')} mode=16")
 except ImportError:
  capstone=None;print("M389_DISASSEMBLER capstone=unavailable mode=none")
 pairs=[(dword(b,TABLE+i*8),dword(b,TABLE+i*8+4)) for i in range(N)]
 starts=scan_starts(b)
 print(f"M389_STRINGS exactStarts={len(starts)}")
 # Vote on all base hypotheses using only true NUL-delimited string starts.
 votes=collections.Counter()
 for v,_ in pairs:
  for p in starts:votes[v-p]+=1
 ranked=[]
 for base,_ in votes.items():
  matches=[(i,v-base,starts[v-base]) for i,(v,_) in enumerate(pairs) if v-base in starts]
  if len(matches)>=2:ranked.append((len(matches),base,matches))
 ranked.sort(key=lambda x:(-x[0],x[1]))
 for score,base,matches in ranked[:12]:
  print(f"M389_NAME_BASE base=0x{base:x} exactStarts={score}/{N}")
  for i,p,name in matches:print(f"M389_NAME_MATCH base=0x{base:x} index={i} file=0x{p:x} name={name!r}")
 if not ranked:print("M389_NAME_BASE noneWithTwoExactStarts=1")
 # Explicitly check plausible 16-bit DOS/4GW segment bases and their
 # off-by-one behavior, to avoid conflating strings with preceding NULs.
 for base in (0xa80000,0xa50000,0xa60000,0xa40000,0x880000):
  valid=0
  for i,(v,_) in enumerate(pairs):
   p=v-base; name=starts.get(p)
   if name:valid+=1
   raw=b[p:p+20].hex() if 0<=p<LIMIT else ""
   print(f"M389_NAME_PROBE base=0x{base:x} index={i} file=0x{p:x} exact={int(name is not None)} name={name!r} bytes={raw}")
  print(f"M389_NAME_PROBE_SUMMARY base=0x{base:x} exact={valid}/{N}")
 for base in (0x880000,0x800000,0x840000,0x0):
  valid=sum(0<=v-base<LIMIT for _,v in pairs)
  print(f"M389_CODE_BASE base=0x{base:x} inLoader={valid}/{N}")
  if valid!=N:continue
  for i,(_,v) in enumerate(pairs):
   p=v-base
   prev=b[max(0,p-4):p].hex()
   raw=b[p:p+16].hex()
   print(f"M389_CODE_ENTRY index={i} addr=0x{v:x} file=0x{p:x} preceding={prev} bytes={raw} disasm={disassemble(b,p,capstone)!r}")
 # Test whether all names and code entries belong to a compact segment
 # and whether candidate starts have common prologue patterns.
 for base in (0x880000,):
  entries=[v-base for _,v in pairs]
  prologues=collections.Counter(b[p:p+3].hex() for p in entries)
  print(f"M389_CODE_CLUSTER base=0x{base:x} minFile=0x{min(entries):x} maxFile=0x{max(entries):x} prologue3={dict(prologues)}")
 print("M389_LIMITATIONS codeDisassemblyNotCFG=1 exactStringsNotProofOfTableSemantics=1 runtimeBasesUnobserved=1")
if __name__=="__main__":main()
