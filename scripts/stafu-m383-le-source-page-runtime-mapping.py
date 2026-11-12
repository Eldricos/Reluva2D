#!/usr/bin/env python3
"""M383 independent LE source-page, physical, and dispatcher address audit.

Read-only Python 3 standard library. All runtime addresses are hypotheses.
"""
import argparse, hashlib, struct, collections
from pathlib import Path
LE=229780
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p): return struct.unpack_from("<H",b,p)[0]
def u32(b,p): return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes();digest=hashlib.sha256(b).hexdigest()
 print(f"M383_INPUT size={len(b)} sha256={digest}")
 if digest!=EXPECTED or b[LE:LE+2]!=b"LE":raise SystemExit("M383_ERROR unexpected OVL")
 ps=u32(b,LE+0x28);n=u32(b,LE+0x14);oc=u32(b,LE+0x44)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 ft=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objects=[dict(size=u32(b,ot+24*i),base=u32(b,ot+24*i+4),first=u32(b,ot+24*i+12),count=u32(b,ot+24*i+16)) for i in range(oc)]
 offsets=[u32(b,ft+4*i) for i in range(n+1)]
 print(f"M383_HEADER pages={n} pageSize={ps} objects={oc} dataOrigin=0x{origin:x} pageMap=0x{pm:x} fixupIndex=0x{ft:x} fixupRecords=0x{fr:x}")
 def physical(logical,within):
  if not 1<=logical<=n or not 0<=within<ps:return None
  q=pm+(logical-1)*4;num=int.from_bytes(b[q:q+3],"big")
  if not num or b[q+3]!=0:return None
  p=origin+(num-1)*ps+within
  return p if p<len(b) else None
 def object_page(logical):
  for i,o in enumerate(objects,1):
   if o["first"]<=logical<o["first"]+o["count"]:
    return i,(logical-o["first"])*ps
  return None,None
 def object_at(obj,rva):
  if not 1<=obj<=oc:return None
  o=objects[obj-1]
  if not 0<=rva<o["size"]:return None
  return physical(o["first"]+rva//ps,rva%ps)
 def records(logical):
  p=fr+offsets[logical-1];end=fr+offsets[logical];out=[]
  while p<end:
   start=p
   if p+7>end:raise ValueError("truncated fixup")
   typ,flags=b[p],b[p+1];src=u16(b,p+2);obj=b[p+4];p+=5
   if typ!=7 or flags not in (0,16):raise ValueError(f"unexpected fixup at 0x{start:x}")
   width=4 if flags==16 else 2
   if p+width>end:raise ValueError("truncated target")
   target=int.from_bytes(b[p:p+width],"little");p+=width
   out.append(dict(file=start,typ=typ,flags=flags,src=src,obj=obj,target=target,width=width))
  return out
 total=0
 for logical in (18,19,20,21):
  obj,pagebase=object_page(logical);rs=records(logical);total+=len(rs)
  src_out=0;invalid_target=0;physbad=0;overlap=0;occupied=set()
  print(f"M383_PAGE logical={logical} sourceObject={obj} objectPageRva=0x{pagebase:x} sourcePagePhysical={hex(physical(logical,0))} records={len(rs)}")
  for r in rs:
   src=r["src"];targetobj=r["obj"];target=r["target"]
   if src+4>ps:src_out+=1
   if any(k in occupied for k in range(src,src+4)):overlap+=1
   occupied.update(range(src,src+4))
   if not 1<=targetobj<=oc or target>=objects[targetobj-1]["size"]:invalid_target+=1
   if physical(logical,src) is None or object_at(obj,pagebase+src)!=physical(logical,src):physbad+=1
  print(f"M383_PAGE_SUMMARY logical={logical} sourceOutOfPage={src_out} overlappingWrites={overlap} invalidTarget={invalid_target} sourcePhysicalMismatch={physbad}")
  if logical in (19,20):
   # For object1, logical page19 starts at 0x12000, page20 at 0x13000.
   table=0x12334 if logical==19 else 0x13488
   jump=0xf579 if logical==19 else 0x106c1
   operand=table
   for j in range(7):
    source=table+4*j;off=source-pagebase
    matches=[r for r in rs if r["src"]==off]
    if len(matches)!=1:raise ValueError("missing unique table source")
    r=matches[0];p=physical(logical,off);p2=object_at(obj,source)
    orig=b[p:p+4] if p is not None else b""
    targetphysical=object_at(r["obj"],r["target"])
    print(f"M383_SLOT logical={logical} slot={j} sourceObject={obj} sourceRva=0x{source:x} sourceLinearPreferred=0x{objects[obj-1]['base']+source:x} sourcePhysical={hex(p)} objectMappingPhysical={hex(p2)} recordFile=0x{r['file']:x} sourceOffset=0x{off:x} raw={orig.hex()} targetObject={r['obj']} targetRva=0x{r['target']:x} targetLinearPreferred=0x{objects[r['obj']-1]['base']+r['target']:x} targetPhysical={hex(targetphysical) if targetphysical is not None else 'none'}")
   instr=b[object_at(1,jump):object_at(1,jump)+8]
   disp=u32(instr,4)
   print(f"M383_JUMP logical={logical} jumpRva=0x{jump:x} jumpPhysical={hex(object_at(1,jump))} instruction={instr.hex()} displacement=0x{disp:x} matchesTableRva={int(disp==table)}")
   for csbase in (0,objects[0]["base"],objects[1]["base"]):
    linear=csbase+disp
    mapped=object_at(1,linear-objects[0]["base"]) if objects[0]["base"]<=linear<objects[0]["base"]+objects[0]["size"] else None
    print(f"M383_CS_HYPOTHESIS logical={logical} csBase=0x{csbase:x} effectiveLinear=0x{linear:x} mapsObject1Rva={hex(linear-objects[0]['base']) if mapped is not None else 'none'} physical={hex(mapped) if mapped is not None else 'none'} matchesFixupSource={int(linear==objects[0]['base']+table)} runtimeObserved=0")
  # Audit unusual targets with complete separation of source and target.
  bytarget=collections.Counter(r["obj"] for r in rs)
  print(f"M383_TARGET_DISTRIBUTION logical={logical} byObject={dict(sorted(bytarget.items()))}")
 print(f"M383_TOTAL auditedPages=4 records={total}")
 print("M383_LIMITATIONS runtimeSelectorUnobserved=1 fixupApplicationUnobserved=1 sourceType7WriteWidthAssumed4=1 codeVsDataUnresolved=1")
if __name__=="__main__":main()
