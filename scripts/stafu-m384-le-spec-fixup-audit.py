#!/usr/bin/env python3
"""M384 specification-based audit of LE internal fixup records.

References: IBM OS/2 16/32-bit Linear Executable Module Format,
section 2.12 (fixup records); Open Watcom exeflat.h definitions.
Read-only. This checks record grammar, not observed loader behavior.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def s16(b,p):return struct.unpack_from("<h",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M384_INPUT bytes={len(b)} sha256={sha}")
 if sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M384_ERROR unexpected OVL")
 n=u32(b,LE+0x14);ps=u32(b,LE+0x28);oc=u32(b,LE+0x44)
 ot=LE+u32(b,LE+0x40);ft=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c)
 idx=[u32(b,ft+4*i) for i in range(n+1)]
 objs=[dict(size=u32(b,ot+24*i),base=u32(b,ot+24*i+4),flags=u32(b,ot+24*i+8),first=u32(b,ot+24*i+12),pages=u32(b,ot+24*i+16)) for i in range(oc)]
 print(f"M384_HEADER objects={oc} pages={n} pageSize={ps}")
 for i,o in enumerate(objs,1):
  print(f"M384_OBJECT id={i} size=0x{o['size']:x} base=0x{o['base']:x} flags=0x{o['flags']:x} executable={int(bool(o['flags']&4))} writable={int(bool(o['flags']&2))} big32={int(bool(o['flags']&0x2000))}")
 total=0;counts=collections.Counter()
 for logical in (18,19,20,21):
  p=fr+idx[logical-1];end=fr+idx[logical];rows=[]
  while p<end:
   start=p
   if p+4>end:raise ValueError("truncated header")
   src_type=b[p];flags=b[p+1];p+=2
   src_kind=src_type&15
   has_list=bool(src_type&0x20)
   alias=bool(src_type&0x10)
   target_kind=flags&3
   additive=bool(flags&4);chain=bool(flags&8)
   target32=bool(flags&0x10);additive32=bool(flags&0x20)
   object16=bool(flags&0x40)
   # Only the observed internal reference type is expected here.
   if src_kind!=7 or target_kind!=0 or has_list or alias or additive or chain or object16 or additive32 or flags&0x80 or src_type&0xc0:
    print(f"M384_UNEXPECTED page={logical} file=0x{start:x} sourceType=0x{src_type:02x} targetFlags=0x{flags:02x}")
    raise SystemExit("M384_STOP unsupported/uncertain record form")
   source=s16(b,p);p+=2
   target_obj=b[p];p+=1
   width=4 if target32 else 2
   if p+width>end:raise ValueError("truncated target")
   target=int.from_bytes(b[p:p+width],"little");p+=width
   rows.append(dict(start=start,source=source,target_obj=target_obj,target=target,flags=flags,width=width))
  if p!=end:raise ValueError("record boundary mismatch")
  total+=len(rows)
  stats=collections.Counter()
  for r in rows:
   stats["target16" if r["width"]==2 else "target32"]+=1
   stats["negativeSource" if r["source"]<0 else "nonnegativeSource"]+=1
   stats["validSource"]+=int(0<=r["source"]<=ps-4)
   stats["validTarget"]+=int(1<=r["target_obj"]<=oc and r["target"]<objs[r["target_obj"]-1]["size"])
   stats["object1" if r["target_obj"]==1 else "object2" if r["target_obj"]==2 else "otherObject"]+=1
  counts.update(stats)
  print(f"M384_PAGE logical={logical} records={len(rows)} "+' '.join(f"{k}={v}" for k,v in sorted(stats.items())))
  if logical in (19,20):
   table=0x12334 if logical==19 else 0x13488
   pagebase=(logical-1)*ps
   for slot in range(7):
    off=table-pagebase+slot*4
    hits=[r for r in rows if r["source"]==off]
    if len(hits)!=1:raise ValueError("table slot not uniquely backed")
    r=hits[0]
    print(f"M384_TABLE_SLOT page={logical} slot={slot} source=0x{off:x} recordFile=0x{r['start']:x} sourceType=07 targetFlags=0x{r['flags']:02x} targetObject={r['target_obj']} targetOffset=0x{r['target']:x} specSourceWriteBits=32 specTargetFieldBits={r['width']*8}")
 print(f"M384_TOTAL records={total} "+' '.join(f"{k}={v}" for k,v in sorted(counts.items())))
 print("M384_SPEC source07=32bit_offset target10=32bit_target_offset objectNumber=one_based sourceOffset=signed16 pageRelative")
 print("M384_LIMITATIONS grammarOnly=1 noLoaderExecution=1 runtimeSelectorsUnobserved=1 actualRelocatedBytesUnobserved=1 codeTableOverlapUnresolved=1")
if __name__=="__main__":main()
