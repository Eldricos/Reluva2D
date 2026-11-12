#!/usr/bin/env python3
"""M377 independent LE fixup grammar audit. Read-only; standard library only.

Checks raw record bytes against independently enumerated grammar choices,
including source-list and additive flag alternatives. Does not relocate code.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
LE=229780
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M377_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=EXPECTED or b[LE:LE+2]!=b"LE":raise SystemExit("M377_ERROR unexpected game executable")
 n=u32(b,LE+0x14);ps=u32(b,LE+0x28)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c)
 ot=LE+u32(b,LE+0x40);count=u32(b,LE+0x44)
 objects=[dict(size=u32(b,ot+i*24),base=u32(b,ot+i*24+4),first=u32(b,ot+i*24+12),pages=u32(b,ot+i*24+16)) for i in range(count)]
 if n!=371 or ps!=4096 or count!=2:raise SystemExit("M377_ERROR layout")
 ix=[u32(b,fi+4*i) for i in range(n+1)]
 print(f"M377_HEADER pages={n} pageSize={ps} fixupBytes={ix[-1]} objects={count} indexMonotonic={int(all(x<=y for x,y in zip(ix,ix[1:])))}")
 for i,o in enumerate(objects,1):print(f"M377_OBJECT id={i} base=0x{o['base']:x} size=0x{o['size']:x} first={o['first']} pages={o['pages']}")
 def parse(blob,mode):
  p=0;out=[];errors=[]
  while p<len(blob):
   start=p
   if p+2>len(blob):errors.append("short_header");break
   source,flags=blob[p:p+2];p+=2
   # This audit deliberately enumerates alternative grammars; only
   # type 7 / flags 0,16 are supported by the observed data.
   if source!=7 or flags not in (0,16):errors.append(f"unknown_{source:02x}_{flags:02x}");break
   if mode=="canonical":
    srcw=2;targetw=4 if flags&16 else 2
   elif mode=="source32":
    srcw=4;targetw=4 if flags&16 else 2
   elif mode=="target32_always":
    srcw=2;targetw=4
   elif mode=="target16_always":
    srcw=2;targetw=2
   elif mode=="flags_reverse":
    srcw=2;targetw=2 if flags&16 else 4
   elif mode=="source8":
    srcw=1;targetw=4 if flags&16 else 2
   elif mode=="target_is_additive":
    srcw=2;targetw=4 if flags&16 else 2
   else:raise ValueError(mode)
   needed=srcw+1+targetw+(4 if mode=="target_is_additive" else 0)
   if p+needed>len(blob):errors.append("truncated");break
   src=int.from_bytes(blob[p:p+srcw],"little");p+=srcw
   obj=blob[p];p+=1
   target=int.from_bytes(blob[p:p+targetw],"little");p+=targetw
   if mode=="target_is_additive":p+=4
   if not 1<=obj<=count:errors.append("invalid_object");break
   if src>=ps:errors.append("source_out_of_page");break
   if target>=objects[obj-1]["size"]:errors.append("target_out_of_object");break
   out.append((start,source,flags,src,obj,target,p-start))
  return out,p,errors
 modes=("canonical","source32","target32_always","target16_always","flags_reverse","source8","target_is_additive")
 for logical in (18,19,20,21):
  blob=b[fr+ix[logical-1]:fr+ix[logical]]
  print(f"M377_PAGE logical={logical} length={len(blob)} first32={blob[:32].hex()} last16={blob[-16:].hex()}")
  for mode in modes:
   rs,used,errors=parse(blob,mode)
   print(f"M377_GRAMMAR logical={logical} mode={mode} consumed={used}/{len(blob)} records={len(rs)} complete={int(used==len(blob) and not errors)} error={','.join(errors) or 'none'}")
  rs,used,errors=parse(blob,"canonical")
  if errors or used!=len(blob):raise SystemExit("M377_ERROR canonical parsing failed")
  print(f"M377_COUNTS logical={logical} sourceTypes={dict(Counter(r[1] for r in rs))} targetFlags={dict(Counter(r[2] for r in rs))} recordSizes={dict(Counter(r[6] for r in rs))} sourceUnique={len(set(r[3] for r in rs))} records={len(rs)}")
  # Independently test if any observed record exhibits the source-list
  # flag (0x20), additive flag (0x04), or chained fixup source encoding.
  print(f"M377_FEATURES logical={logical} sourceListFlag20={sum(bool(r[2]&0x20) for r in rs)} additiveFlag04={sum(bool(r[2]&0x04) for r in rs)} offset32Flag10={sum(bool(r[2]&0x10) for r in rs)}")
  for name,base in (("object_state",0x334),("global_state",0x488)):
   if (logical,name) not in ((19,"object_state"),(20,"global_state")):continue
   for i in range(7):
    off=base+i*4;found=[r for r in rs if r[3]==off]
    print(f"M377_TABLE name={name} slot={i} source=0x{off:x} records={len(found)} "+(" ".join(f"raw={blob[r[0]:r[0]+r[6]].hex()} flags=0x{r[2]:02x} targetObject={r[4]} targetOffset=0x{r[5]:x} recordBytes={r[6]}" for r in found) if found else "missing"))
  # Test actual contiguous source regions for overlapping writes.
  writes=Counter(k for r in rs for k in range(r[3],r[3]+4))
  overlaps=[x for x,c in writes.items() if c>1]
  print(f"M377_WRITE_AUDIT logical={logical} sourcesCrossPage={sum(r[3]+4>ps for r in rs)} overlappedBytes={len(overlaps)} maxWritesPerByte={max(writes.values(),default=0)}")
 print("M377_SCOPE grammarAlternativesHeuristic=1 rawFixupRecordsOnly=1 sourceType7AndFlagsOnly=1 noRuntimeSelector=1 noLoaderObservation=1 noSPRDecoder=1")
if __name__=="__main__":main()
