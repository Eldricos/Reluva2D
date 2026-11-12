#!/usr/bin/env python3
"""M379: read-only dispatcher predecessor and table-origin control-flow audit.

Requires capstone (python -m pip install capstone). Heuristic local CFG:
decodes candidate entry points, follows direct relative branches, and reports
indirect jump index setup. No claim of exhaustive control-flow recovery.
"""
import argparse,hashlib,struct
from pathlib import Path
from collections import defaultdict,deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86_const import X86_OP_IMM,X86_OP_MEM,X86_OP_REG
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M379_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M379_ERROR unexpected executable")
 size=u32(b,LE+0x28);pm=LE+u32(b,LE+0x48);origin=LE+u32(b,LE+0x80)
 ot=LE+u32(b,LE+0x40);base=u32(b,ot+4);first=u32(b,ot+12);osize=u32(b,ot)
 if size!=4096 or base!=0x10000:raise SystemExit("M379_ERROR unexpected object layout")
 def physical(rva):
  if not 0<=rva<osize:return None
  logical=first+rva//size;q=pm+4*(logical-1)
  page=int.from_bytes(b[q:q+3],"big")
  if page==0 or b[q+3]!=0:return None
  p=origin+(page-1)*size+rva%size
  return p if p<len(b) else None
 def raw(rva,n):
  out=bytearray()
  for i in range(n):
   p=physical(rva+i)
   if p is None:break
   out.append(b[p])
  return bytes(out)
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 def instruction(rva):
  data=raw(rva,15)
  if not data:return None
  return next(md.disasm(data,base+rva,count=1),None)
 def target(ins):
  if not ins.operands or ins.operands[0].type!=X86_OP_IMM:return None
  return ins.operands[0].imm-base
 def fmt(ins):return f"rva=0x{ins.address-base:x} bytes={ins.bytes.hex()} {ins.mnemonic} {ins.op_str}".strip()
 # Find direct branch/call targets by linear decoding from several offsets.
 # This is intentionally not an exhaustive or authoritative CFG.
 def walk(start,lo,hi,limit=2500):
  todo=deque([start]);seen=set();edges=[];decoded={}
  while todo and len(seen)<limit:
   rva=todo.popleft()
   while lo<=rva<hi and rva not in seen and len(seen)<limit:
    ins=instruction(rva)
    if ins is None or ins.size<1 or rva+ins.size>hi:break
    seen.add(rva);decoded[rva]=ins
    nxt=rva+ins.size
    iscall=ins.mnemonic.startswith("call")
    isjmp=ins.mnemonic=="jmp"
    isjcc=ins.mnemonic.startswith("j") and not isjmp
    if iscall or isjmp or isjcc:
     dst=target(ins)
     if dst is not None:
      edges.append((rva,dst,ins.mnemonic))
      if lo<=dst<hi:todo.append(dst)
     if isjmp:break
    if ins.mnemonic.startswith("ret") or ins.mnemonic in ("int","int3","iret","ud2","hlt"):break
    rva=nxt
  return decoded,edges
 for name,jump,signature,table,lo,hi in (
  ("object_state",0xf579,bytes.fromhex("2eff248534230100"),0x12334,0xf100,0xf800),
  ("global_state",0x106c1,bytes.fromhex("2eff248588340100"),0x13488,0x10200,0x10900)):
  print(f"M379_DISPATCH name={name} jumpRva=0x{jump:x} jumpPhysical={hex(physical(jump))} exactSignature={int(raw(jump,8)==signature)} tableOperand=0x{table:x}")
  for back in (96,64,48,32,24,16,8):
   start=jump-back
   cursor=start;lines=[];hit=False
   while cursor<=jump:
    ins=instruction(cursor)
    if ins is None:break
    lines.append(ins)
    if cursor==jump:hit=True;break
    cursor+=ins.size
   print(f"M379_BACKTRACK name={name} startRva=0x{start:x} reachesJump={int(hit)} instructionCount={len(lines)}")
   if hit:
    for ins in lines[-20:]:print(f"M379_PREDECESSOR name={name} {fmt(ins)}")
  seeds=[lo,lo+16,lo+32,jump-96,jump-64,jump-32,jump-16]
  all_decoded={};all_edges=[]
  for seed in seeds:
   decoded,edges=walk(seed,lo,hi)
   all_decoded.update(decoded);all_edges.extend(edges)
   print(f"M379_CFG_SEED name={name} seed=0x{seed:x} decoded={len(decoded)} directEdges={len(edges)} jumpReached={int(jump in decoded)}")
  inbound=[(src,kind) for src,dst,kind in all_edges if dst==jump]
  print(f"M379_CFG_SUMMARY name={name} uniqueDecoded={len(all_decoded)} directEdges={len(set(all_edges))} directInboundToJump={len(set(inbound))} inbound={','.join(f'0x{s:x}:{k}' for s,k in sorted(set(inbound))) or 'none'}")
  for rva in sorted(all_decoded):
   if jump-96<=rva<=jump+8:
    ins=all_decoded[rva]
    print(f"M379_CFG_NEAR name={name} {fmt(ins)}")
  # Explicitly show jump operand and surrounding bytes; raw bytes are
  # not proof of an executed indirect branch.
  for rva in (table-16,table,table+28,table+40):
   print(f"M379_TABLE_RAW name={name} rva=0x{rva:x} file={hex(physical(rva)) if physical(rva) is not None else 'none'} bytes={raw(rva,32).hex()}")
  # Direct code references to table-region RVAs from bounded local CFG.
  for table_target in (table,table+4,table+28):
   refs=[(src,kind) for src,dst,kind in all_edges if dst==table_target]
   print(f"M379_DIRECT_TABLE_EDGES name={name} targetRva=0x{table_target:x} count={len(set(refs))} sources={','.join(f'0x{s:x}:{k}' for s,k in sorted(set(refs))) or 'none'}")
 # Analyze raw instruction starts and direct branch edges in table regions,
 # using several seed alignments to expose decoding ambiguity.
 for name,table in (("object_state",0x12334),("global_state",0x13488)):
  lo=table-128;hi=table+512
  for seed in (lo,lo+1,lo+2,table-32,table-16,table):
   decoded,edges=walk(seed,lo,hi,1200)
   into=[(src,dst,k) for src,dst,k in edges if table<=dst<table+28]
   print(f"M379_TABLE_CFG name={name} seed=0x{seed:x} decoded={len(decoded)} edgesIntoTable={len(into)} directHits={','.join(f'0x{s:x}->0x{d:x}:{k}' for s,d,k in into[:12]) or 'none'}")
 print("M379_LIMITATIONS heuristicCFG=1 seedSensitive=1 noObservedRuntimeSelector=1 noObservedRelocatedBytes=1 noSPRDecoder=1")
if __name__=="__main__":main()
