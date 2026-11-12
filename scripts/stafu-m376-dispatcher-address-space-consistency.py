#!/usr/bin/env python3
"""STAFU M376: dispatcher address-space consistency (read-only).

Separates raw-file observations from hypothetical LE relocation and selector
base assumptions. Requires only Python 3 and the user's own STTNG.OVL.
"""
import argparse,hashlib,struct
from pathlib import Path
from collections import Counter
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M376_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M376_ERROR unexpected OVL")
 size=u32(b,LE+0x28);n=u32(b,LE+0x14)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48);fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[dict(size=u32(b,ot+24*i),base=u32(b,ot+24*i+4),first=u32(b,ot+24*i+12),pages=u32(b,ot+24*i+16)) for i in range(u32(b,LE+0x44))]
 if size!=4096 or n!=371 or len(objs)!=2:raise SystemExit("M376_ERROR unexpected layout")
 ix=[u32(b,fi+4*i) for i in range(n+1)]
 def physical(obj,rva):
  o=objs[obj-1]
  if rva<0 or rva>=o["size"]:return None
  logical=o["first"]+rva//size
  if not 1<=logical<=n:return None
  q=pm+4*(logical-1);page=int.from_bytes(b[q:q+3],"big")
  if page<1 or b[q+3]:return None
  result=origin+(page-1)*size+rva%size
  return result if 0<=result<len(b) else None
 def data(obj,rva,length):
  result=bytearray()
  for i in range(length):
   p=physical(obj,rva+i)
   if p is None:return None
   result.append(b[p])
  return bytes(result)
 def fixups(logical):
  p=fr+ix[logical-1];end=fr+ix[logical];out=[]
  while p<end:
   start=p
   if p+7>end:raise ValueError("truncated record")
   typ,flags=b[p],b[p+1];p+=2
   if typ!=7 or flags not in (0,16):raise ValueError(f"unsupported record {typ}/{flags}")
   source=u16(b,p);p+=2;obj=b[p];p+=1
   width=4 if flags==16 else 2
   if p+width>end:raise ValueError("truncated target")
   target=u32(b,p) if width==4 else u16(b,p);p+=width
   out.append(dict(record=start-fr,source=source,obj=obj,target=target,flags=flags))
  return out
 print(f"M376_LAYOUT object1Base=0x{objs[0]['base']:x} object1Size=0x{objs[0]['size']:x} object2Base=0x{objs[1]['base']:x} object2Size=0x{objs[1]['size']:x}")
 configurations=(
  ("object_state",0xf579,bytes.fromhex("2eff248534230100"),0x12334,19,0x334),
  ("global_state",0x106c1,bytes.fromhex("2eff248588340100"),0x13488,20,0x488))
 for name,jump,signature,operand,logical,source in configurations:
  raw=data(1,jump,len(signature));p=physical(1,jump)
  print(f"M376_JUMP name={name} jumpRva=0x{jump:x} physical={f'0x{p:x}' if p is not None else 'none'} expected={signature.hex()} observed={raw.hex() if raw else 'none'} matches={int(raw==signature)} operand=0x{operand:x}")
  rs=fixups(logical);slots=[]
  for i in range(7):
   rva=operand+4*i
   matches=[r for r in rs if r["source"]==source+4*i]
   if len(matches)!=1:raise ValueError(f"{name}: missing/ambiguous slot {i}")
   r=matches[0];original=data(1,rva,4)
   if original is None:raise ValueError("table read")
   linear=objs[r["obj"]-1]["base"]+r["target"]
   slots.append((rva,r,linear))
   print(f"M376_SLOT name={name} index={i} sourceRva=0x{rva:x} original={original.hex()} record={r['record']} flags=0x{r['flags']:x} targetObject={r['obj']} targetOffset=0x{r['target']:x} hypotheticalLinear=0x{linear:x} hypotheticalBytes={struct.pack('<I',linear).hex()}")
  # A CS-relative absolute displacement can only reach these source RVAs
  # if the runtime segment base and the target object base are compatible.
  for csbase in (0,objs[0]["base"],objs[1]["base"]):
   effective=csbase+operand
   candidates=[(i,effective-o["base"]) for i,o in enumerate(objs,1) if o["base"]<=effective<o["base"]+o["size"]]
   matches=sum(1 for i,rva in candidates if i==1 and rva==operand)
   print(f"M376_SELECTOR name={name} assumedCSBase=0x{csbase:x} effectiveLinear=0x{effective:x} objectRvaCandidates={','.join(f'{i}:0x{rva:x}' for i,rva in candidates) or 'none'} reachesProposedTable={int(matches>0)} selectorObserved=0")
  targets=sorted(set((r["obj"],r["target"]) for _,r,_ in slots))
  for obj,target in targets:
   raw=data(obj,target,24)
   before=data(obj,target-8,8) if target>=8 else None
   print(f"M376_TARGET name={name} object={obj} rva=0x{target:x} physical={f'0x{physical(obj,target):x}' if physical(obj,target) is not None else 'none'} rawBefore={before.hex() if before else 'none'} rawAt={raw.hex() if raw else 'none'} runtimeCodeUnverified=1")
  # Enumerate exact raw 32-bit references and hypothetical relocated
  # references within a bounded surrounding region. A byte hit alone is
  # not evidence of a code reference.
  for key,value in (("operand",operand),("tableLinear",objs[0]["base"]+operand)):
   needle=struct.pack("<I",value)
   hits=[]
   lo=max(0,jump-512);hi=min(objs[0]["size"]-4,jump+512)
   for rva in range(lo,hi+1):
    if data(1,rva,4)==needle:hits.append(rva)
   print(f"M376_NEAR_REFERENCE name={name} needle={key} value=0x{value:x} windowRva=0x{lo:x}-0x{hi:x} rawByteHits={len(hits)} positions={','.join(hex(x) for x in hits[:24]) or 'none'} semanticReferencesUnverified=1")
  # Adjacent raw instruction decoding is intentionally optional, not a
  # prerequisite for structural conclusions.
  try:
   from capstone import Cs,CS_ARCH_X86,CS_MODE_32
   md=Cs(CS_ARCH_X86,CS_MODE_32)
   for obj,target in targets:
    buf=data(obj,target,32)
    if buf is None:continue
    for delta in (-1,0,1):
     start=target+delta
     chunk=data(obj,start,20) if start>=0 else None
     if chunk is None:continue
     ins=list(md.disasm(chunk,objs[obj-1]["base"]+start,count=4))
     desc=" | ".join(f"{x.mnemonic} {x.op_str}".strip() for x in ins) or "undecodable"
     print(f"M376_DISASM name={name} object={obj} targetRva=0x{target:x} shift={delta:+d} instructions={desc} rawOnly=1")
  except ImportError:
   print(f"M376_DISASM name={name} available=0 reason=capstone_not_installed")
 print("M376_LIMITATIONS runtimeCSUnknown=1 fixupApplicationHypothetical=1 rawByteReferencesNotControlFlowProof=1 noRuntimeLoader=1 noSPRDecoder=1")
if __name__=="__main__":main()
