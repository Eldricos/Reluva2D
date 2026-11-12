#!/usr/bin/env python3
"""M378: LE object, page, selector and relocation address-model audit.

Read-only Python 3 diagnostic. All runtime linear addresses are hypotheses,
not observed selector descriptors or observed loader output.
"""
import argparse,hashlib,struct
from pathlib import Path
LE=229780
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();digest=hashlib.sha256(b).hexdigest()
 print(f"M378_INPUT bytes={len(b)} sha256={digest}")
 if len(b)!=2008455 or digest!=EXPECTED or b[LE:LE+2]!=b"LE":raise SystemExit("M378_ERROR unexpected executable")
 n=u32(b,LE+0x14);size=u32(b,LE+0x28)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);dataorigin=LE+u32(b,LE+0x80)
 cnt=u32(b,LE+0x44)
 objects=[dict(size=u32(b,ot+i*24),base=u32(b,ot+i*24+4),flags=u32(b,ot+i*24+8),first=u32(b,ot+i*24+12),pages=u32(b,ot+i*24+16)) for i in range(cnt)]
 if n!=371 or size!=4096 or cnt!=2:raise SystemExit("M378_ERROR unexpected LE layout")
 ix=[u32(b,fi+4*i) for i in range(n+1)]
 def resolve(obj,rva):
  if not 1<=obj<=cnt:return None
  o=objects[obj-1]
  if not 0<=rva<o["size"]:return None
  logical=o["first"]+rva//size
  if not 1<=logical<=n:return None
  ent=b[pm+4*(logical-1):pm+4*logical]
  if len(ent)!=4 or ent[3]!=0:return None
  page=int.from_bytes(ent[:3],"big")
  if page<1:return None
  physical=dataorigin+(page-1)*size+rva%size
  return (logical,page,physical) if physical<len(b) else None
 def raw(obj,rva,length):
  result=bytearray()
  for i in range(length):
   loc=resolve(obj,rva+i)
   if loc is None:return None
   result.append(b[loc[2]])
  return bytes(result)
 def records(logical):
  p=fr+ix[logical-1];end=fr+ix[logical];rs=[]
  while p<end:
   if p+7>end:raise ValueError("truncated fixup")
   st,fl=b[p],b[p+1];p+=2
   if st!=7 or fl not in (0,16):raise ValueError("unsupported fixup")
   src=u16(b,p);p+=2;obj=b[p];p+=1
   width=4 if fl&16 else 2
   if p+width>end:raise ValueError("truncated target")
   target=u32(b,p) if width==4 else u16(b,p);p+=width
   rs.append((src,obj,target,fl))
  return rs
 print(f"M378_HEADER pageCount={n} pageSize={size} dataOrigin=0x{dataorigin:x} fixupIndexMonotonic={int(all(x<=y for x,y in zip(ix,ix[1:])))}")
 for i,o in enumerate(objects,1):
  print(f"M378_OBJECT id={i} base=0x{o['base']:x} size=0x{o['size']:x} flags=0x{o['flags']:x} firstPage={o['first']} pages={o['pages']} linearEndExclusive=0x{o['base']+o['size']:x}")
 for name,jumprva,signature,table,logical,source in (
  ("object_state",0xf579,bytes.fromhex("2eff248534230100"),0x12334,19,0x334),
  ("global_state",0x106c1,bytes.fromhex("2eff248588340100"),0x13488,20,0x488)):
  j=resolve(1,jumprva);observed=raw(1,jumprva,len(signature))
  print(f"M378_JUMP name={name} object=1 rva=0x{jumprva:x} preferredLinear=0x{objects[0]['base']+jumprva:x} logicalPage={j[0] if j else 'none'} physical={hex(j[2]) if j else 'none'} exactSignature={int(observed==signature)}")
  entries=[]
  for i in range(7):
   src=source+4*i
   matches=[r for r in records(logical) if r[0]==src]
   if len(matches)!=1:raise ValueError("ambiguous dispatcher slot")
   _,obj,target,flags=matches[0]
   where=resolve(1,table+4*i);t=resolve(obj,target)
   entries.append((obj,target))
   print(f"M378_FIXUP name={name} slot={i} sourceObject=1 sourceRva=0x{table+4*i:x} sourcePreferredLinear=0x{objects[0]['base']+table+4*i:x} sourceFile={hex(where[2]) if where else 'none'} rawSource={raw(1,table+4*i,4).hex()} targetObject={obj} targetRva=0x{target:x} targetPreferredLinear=0x{objects[obj-1]['base']+target:x} targetFile={hex(t[2]) if t else 'none'} targetInObject={int(t is not None)}")
  for csbase in (0,objects[0]["base"],objects[1]["base"]):
   effective=csbase+table
   interpretations=[(i,effective-o["base"]) for i,o in enumerate(objects,1) if o["base"]<=effective<o["base"]+o["size"]]
   exact=any(i==1 and rva==table for i,rva in interpretations)
   print(f"M378_SELECTOR name={name} csBaseHypothesis=0x{csbase:x} effectiveLinear=0x{effective:x} mapped={','.join(f'obj{i}:rva0x{rva:x}' for i,rva in interpretations) or 'none'} matchesProposedTable={int(exact)} observedRuntimeCS=0")
  for obj,target in sorted(set(entries)):
   loc=resolve(obj,target);buf=raw(obj,target,12)
   print(f"M378_HANDLER name={name} targetObject={obj} targetRva=0x{target:x} preferredLinear=0x{objects[obj-1]['base']+target:x} file={hex(loc[2]) if loc else 'none'} raw={buf.hex() if buf else 'none'} entryBoundaryVerified=0")
 # Assess the *global* consistency of preferred object base ranges and
 # LE entry point object-relative interpretation, without inferring selector.
 entry_obj=u32(b,LE+0x18);entry_rva=u32(b,LE+0x1c)
 stack_obj=u32(b,LE+0x20);stack_rva=u32(b,LE+0x24)
 for name,obj,rva in (("entry",entry_obj,entry_rva),("stack",stack_obj,stack_rva)):
  valid=1<=obj<=cnt and 0<=rva<=objects[obj-1]["size"]
  loc=resolve(obj,rva) if valid else None
  print(f"M378_ENTRY name={name} object={obj} rva=0x{rva:x} validWithinObject={int(valid)} preferredLinear={hex(objects[obj-1]['base']+rva) if valid else 'none'} file={hex(loc[2]) if loc else 'none'}")
 print("M378_LIMITATIONS preferredBasesFromHeader=1 runtimeSelectorNotObserved=1 runtimeRelocationsNotObserved=1 handlerBoundariesNotProven=1 noSPRDecoder=1")
if __name__=="__main__":main()
