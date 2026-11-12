#!/usr/bin/env python3
"""M381: LE fixup application semantics and code-overlap audit.

Read-only; uses only Python standard library. A simulated relocation is
explicitly hypothetical: it is NOT a capture of DOS/4GW runtime memory.
"""
import argparse,hashlib,struct
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();digest=hashlib.sha256(b).hexdigest()
 print(f"M381_INPUT bytes={len(b)} sha256={digest}")
 if len(b)!=2008455 or digest!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M381_ERROR unexpected executable")
 count=u32(b,LE+0x14);ps=u32(b,LE+0x28);ot=LE+u32(b,LE+0x40)
 pm=LE+u32(b,LE+0x48);ft=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c)
 origin=LE+u32(b,LE+0x80);oc=u32(b,LE+0x44)
 objects=[dict(size=u32(b,ot+24*i),base=u32(b,ot+24*i+4),first=u32(b,ot+24*i+12),pages=u32(b,ot+24*i+16)) for i in range(oc)]
 if ps!=4096 or count!=371 or oc!=2:raise SystemExit("M381_ERROR layout mismatch")
 ix=[u32(b,ft+4*i) for i in range(count+1)]
 def locate(obj,rva):
  if not 1<=obj<=oc:return None
  o=objects[obj-1]
  if not 0<=rva<o["size"]:return None
  logical=o["first"]+rva//ps
  if not 1<=logical<=count:return None
  q=pm+4*(logical-1);num=int.from_bytes(b[q:q+3],"big")
  if not num or b[q+3]:return None
  physical=origin+(num-1)*ps+rva%ps
  return (logical,physical) if physical<len(b) else None
 def read(obj,rva,n):
  data=bytearray()
  for i in range(n):
   x=locate(obj,rva+i)
   if x is None:return None
   data.append(b[x[1]])
  return bytes(data)
 def parse(logical):
  p=fr+ix[logical-1];end=fr+ix[logical];rows=[]
  while p<end:
   start=p
   if p+7>end:raise ValueError("truncated")
   typ,flags=b[p],b[p+1];p+=2
   if typ!=7 or flags not in (0,16):raise ValueError("unexpected record type")
   src=u16(b,p);p+=2;obj=b[p];p+=1
   width=4 if flags&16 else 2
   if p+width>end:raise ValueError("truncated target")
   target=int.from_bytes(b[p:p+width],"little");p+=width
   rows.append(dict(at=start,source=src,target=target,object=obj,flags=flags,width=width))
  return rows
 print(f"M381_HEADER objects={oc} pages={count} pageSize={ps} fixupRecordBytes={ix[-1]}")
 for i,o in enumerate(objects,1):print(f"M381_OBJECT id={i} preferredBase=0x{o['base']:x} size=0x{o['size']:x} first={o['first']} pages={o['pages']}")
 # Fixup type 7 is a 32-bit offset fixup; target flag 0x10 selects
 # target offset field width, not the width of the source write.
 for logical in (18,19,20,21):
  records=parse(logical);source_overlaps=0;cross=0;bad=0
  occupied=set()
  for r in records:
   if r["source"]+4>ps:cross+=1
   if any(x in occupied for x in range(r["source"],r["source"]+4)):source_overlaps+=1
   occupied.update(range(r["source"],r["source"]+4))
   if locate(r["object"],r["target"]) is None:bad+=1
  print(f"M381_PAGE logical={logical} records={len(records)} target16={sum(r['flags']==0 for r in records)} target32={sum(r['flags']==16 for r in records)} sourceWriteWidth=4 overlaps={source_overlaps} crossPage={cross} invalidTarget={bad}")
 configs=(("object_state",19,0x12334,0x334,[0x1238e,0x1239b,0x123ca,0x123eb,0x123f8,0x12429],0x12314,0x12364),
          ("global_state",20,0x13488,0x488,[0x134ed,0x13561,0x1356e,0x13598],0x1346b,0x134bb))
 for name,logical,table,srcbase,targets,code_lo,code_hi in configs:
  records=parse(logical)
  slots=[]
  for j in range(7):
   hits=[r for r in records if r["source"]==srcbase+4*j]
   if len(hits)!=1:raise ValueError("missing/ambiguous slot")
   slots.append(hits[0])
  print(f"M381_TABLE name={name} page={logical} rva=0x{table:x} exactFixupSlots={len(slots)}")
  for j,r in enumerate(slots):
   source=table+4*j;original=read(1,source,4)
   if original is None:raise ValueError("source not mapped")
   target_obj=objects[r["object"]-1]
   offset32=r["target"]&0xffffffff
   linear32=(target_obj["base"]+r["target"])&0xffffffff
   print(f"M381_SLOT name={name} slot={j} raw={original.hex()} sourceRva=0x{source:x} targetObj={r['object']} targetRva=0x{r['target']:x} flags=0x{r['flags']:x} hypotheticalOffset32={offset32.to_bytes(4,'little').hex()} hypotheticalLinear32={linear32.to_bytes(4,'little').hex()} targetFile={hex(locate(r['object'],r['target'])[1]) if locate(r['object'],r['target']) else 'none'}")
  original=read(1,code_lo,code_hi-code_lo)
  if original is None:raise ValueError("code window unmapped")
  for model in ("object_offset","preferred_linear"):
   patched=bytearray(original)
   for j,r in enumerate(slots):
    source=table+4*j
    value=r["target"] if model=="object_offset" else objects[r["object"]-1]["base"]+r["target"]
    for k,c in enumerate((value&0xffffffff).to_bytes(4,"little")):
     if code_lo<=source+k<code_hi:patched[source+k-code_lo]=c
   diffs=sum(x!=y for x,y in zip(original,patched))
   print(f"M381_CODE_WINDOW name={name} model={model} begin=0x{code_lo:x} end=0x{code_hi:x} changedBytes={diffs} original={original.hex()} simulated={patched.hex()} runtimeObserved=0")
  for target in targets:
   # A relocation source near a target can corrupt raw instruction bytes;
   # enumerate rather than assume the target is an instruction boundary.
   overlap=[]
   for r in records:
    source=(logical-1)*ps+r["source"]
    if target-8<=source<=target+12:overlap.append(source)
   print(f"M381_TARGET_NEAR_SOURCE name={name} target=0x{target:x} nearbySourceRvas={','.join(hex(x) for x in sorted(overlap)) or 'none'} raw={read(1,target,12).hex() if read(1,target,12) else 'none'}")
 print("M381_INTERPRETATION type7_sourceWriteWidth32=1 targetFlag10_selects32bitTargetField=1")
 print("M381_LIMITATIONS LERecordParsingObserved=1 sourceTypeMeaningSpecAssumption=1 loaderBehaviorNotObserved=1 runtimeSelectorUnknown=1 relocationRuntimeBytesUnknown=1 noSPRDecoder=1")
if __name__=="__main__":main()
