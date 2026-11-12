#!/usr/bin/env python3
"""M380 function-anchored control-flow audit of two LE dispatchers.

Read-only. Requires capstone. Candidate function starts are heuristics, not
verified symbols; separate anchored decoding from arbitrary alignments.
"""
import argparse,hashlib,struct
from collections import deque,defaultdict
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86_const import X86_OP_IMM
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M380_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M380_ERROR unexpected executable")
 ps=u32(b,LE+0x28);pm=LE+u32(b,LE+0x48);origin=LE+u32(b,LE+0x80)
 ot=LE+u32(b,LE+0x40);base=u32(b,ot+4);first=u32(b,ot+12);size=u32(b,ot)
 if ps!=4096 or base!=0x10000:raise SystemExit("M380_ERROR unexpected layout")
 def loc(rva):
  if not 0<=rva<size:return None
  q=pm+4*(first+rva//ps-1);num=int.from_bytes(b[q:q+3],"big")
  if num<1 or b[q+3]:return None
  p=origin+(num-1)*ps+rva%ps
  return p if p<len(b) else None
 def raw(rva,n):
  v=bytearray()
  for j in range(n):
   p=loc(rva+j)
   if p is None:break
   v.append(b[p])
  return bytes(v)
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 def inst(rva):
  return next(md.disasm(raw(rva,15),base+rva,count=1),None)
 def dest(i):
  if i.operands and i.operands[0].type==X86_OP_IMM:return i.operands[0].imm-base
  return None
 def cfg(seed,lo,hi,limit=10000):
  todo=deque([seed]);seen={};edges=set();blockstarts=set()
  while todo and len(seen)<limit:
   p=todo.popleft()
   if not lo<=p<hi:continue
   blockstarts.add(p)
   while lo<=p<hi and p not in seen and len(seen)<limit:
    i=inst(p)
    if i is None or p+i.size>hi:break
    seen[p]=i;nextp=p+i.size
    m=i.mnemonic
    if m=="jmp" or (m.startswith("j") and m!="jmp"):
     d=dest(i)
     if d is not None:
      edges.add((p,d,m))
      if lo<=d<hi:todo.append(d)
     if m=="jmp":break
     todo.append(nextp);break
    if m.startswith("ret") or m in ("ud2","int3","iret","hlt"):break
    if m.startswith("call"):
     d=dest(i)
     if d is not None:edges.add((p,d,m))
    p=nextp
  return seen,edges,blockstarts
 configs=(("object_state",0xf579,0x12334,0x1238e,0x12429,0xe900,0xf600),
          ("global_state",0x106c1,0x13488,0x134ed,0x13598,0x10000,0x10700))
 for name,jump,table,tmin,tmax,lo,hi in configs:
  print(f"M380_DISPATCH name={name} rva=0x{jump:x} bytes={raw(jump,8).hex()}")
  # Scan for common function prologues, including frame and callee-save
  # variants. These are candidates only.
  patterns=((b"\x55\x89\xe5","push_ebp_mov_ebp_esp"),
            (b"\x55\x8b\xec","push_ebp_mov_ebp_esp_alt"),
            (b"\x53\x56","push_ebx_push_esi"),
            (b"\x56\x57","push_esi_push_edi"))
  candidates=[]
  for p in range(lo,jump):
   for sig,kind in patterns:
    if raw(p,len(sig))==sig:candidates.append((p,kind))
  print(f"M380_ANCHORS name={name} window=0x{lo:x}-0x{jump:x} candidateCount={len(candidates)} nearest={','.join(f'0x{p:x}:{k}' for p,k in candidates[-24:]) or 'none'}")
  # Each anchor is decoded as an independent hypothetical function start.
  results=[]
  for p,kind in candidates:
   decoded,edges,blocks=cfg(p,p,min(hi,jump+32))
   if jump in decoded:
    results.append((p,kind,decoded,edges,blocks))
  print(f"M380_REACH name={name} anchoredCandidatesReachingJump={len(results)}")
  for p,kind,decoded,edges,blocks in results[-12:]:
   print(f"M380_ANCHORED_PATH name={name} anchor=0x{p:x} kind={kind} instructions={len(decoded)} branches={len(edges)} jumpDecoded={int(jump in decoded)}")
   for at in sorted(decoded):
    if jump-32<=at<=jump:
     i=decoded[at]
     print(f"M380_ANCHORED_NEAR name={name} anchor=0x{p:x} rva=0x{at:x} bytes={i.bytes.hex()} mnemonic={i.mnemonic} operands={i.op_str}")
  # A separate bounded analysis of the suspected table-region as ordinary
  # code, with anchor candidates rather than seeds at arbitrary offsets.
  regionlo=table-512;regionhi=tmax+128
  regionanchors=[]
  for p in range(regionlo,table):
   for sig,kind in patterns[:2]:
    if raw(p,len(sig))==sig:regionanchors.append((p,kind))
  print(f"M380_TABLE_ANCHORS name={name} candidates={len(regionanchors)} nearest={','.join(hex(p) for p,k in regionanchors[-20:]) or 'none'}")
  table_results=[]
  for p,kind in regionanchors:
   decoded,edges,blocks=cfg(p,p,regionhi)
   hits=sorted(x for x in decoded if table<=x<table+28)
   incoming=sorted((src,dst,m) for src,dst,m in edges if table<=dst<table+28)
   if hits or incoming:
    table_results.append((p,kind,decoded,edges,hits,incoming))
  print(f"M380_TABLE_REACH name={name} anchoredCandidatesEnteringTable={len(table_results)}")
  for p,kind,decoded,edges,hits,incoming in table_results[-12:]:
   print(f"M380_TABLE_PATH name={name} anchor=0x{p:x} kind={kind} decodedTableStarts={','.join(hex(x) for x in hits) or 'none'} inbound={','.join(f'0x{s:x}->0x{d:x}:{m}' for s,d,m in incoming[:16]) or 'none'}")
  targets=([0x1238e,0x1239b,0x123ca,0x123eb,0x123f8,0x12429] if name=="object_state" else [0x134ed,0x13561,0x1356e,0x13598])
  for t in targets:
   hits=[p for p,kind,decoded,edges,_,_ in table_results if t in decoded]
   incoming=[(p,s,m) for p,kind,decoded,edges,_,_ in table_results for s,d,m in edges if d==t]
   print(f"M380_HANDLER_REACH name={name} rva=0x{t:x} candidateAnchorsDecodingAtTarget={len(hits)} candidateAnchorsWithDirectInbound={len(incoming)} raw={raw(t,12).hex()} runtimeEntryUnverified=1")
  if not results:print(f"M380_WARNING name={name} noFunctionPrologueAnchorReachesDispatcher; this does not prove dispatcher unreachable")
  if not table_results:print(f"M380_WARNING name={name} noFunctionPrologueAnchorReachesTable; this does not prove table is data")
 print("M380_LIMITATIONS prologueCandidatesNotSymbols=1 CFGLocalOnly=1 indirectBranchesNotFollowed=1 noRuntimeRelocation=1 noObservedSelector=1 noSPRDecoder=1")
if __name__=="__main__":main()
