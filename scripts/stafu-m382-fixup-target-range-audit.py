#!/usr/bin/env python3
"""M382: classify LE fixup target object fields and address ranges.

Read-only; standard library only. Tests alternate interpretations as
hypotheses, not as verified LE loader semantics.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M382_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M382_ERROR unexpected executable")
 pages=u32(b,LE+0x14);ps=u32(b,LE+0x28);oc=u32(b,LE+0x44)
 ot=LE+u32(b,LE+0x40);ft=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c)
 objs=[(u32(b,ot+24*i),u32(b,ot+24*i+4),u32(b,ot+24*i+12),u32(b,ot+24*i+16)) for i in range(oc)]
 ix=[u32(b,ft+4*i) for i in range(pages+1)]
 print(f"M382_HEADER pages={pages} pageSize={ps} objects={oc} recordBytes={ix[-1]}")
 for j,(size,base,first,np) in enumerate(objs,1):
  print(f"M382_OBJECT number={j} size=0x{size:x} preferredBase=0x{base:x} firstPage={first} pageCount={np}")
 records=[]
 for logical in (18,19,20,21):
  p=fr+ix[logical-1];end=fr+ix[logical]
  while p<end:
   at=p
   if p+5>end:raise ValueError(f"truncated record page {logical}")
   typ,flags=b[p],b[p+1];p+=2
   if typ!=7 or flags not in (0,16):
    # Full-file records may contain other valid LE fixup forms.
    # Report the first unsupported record and stop rather than silently
    # reinterpret or fabricate record boundaries.
    print(f"M382_UNSUPPORTED page={logical} at=0x{at:x} type=0x{typ:x} flags=0x{flags:x} parsedSoFar={len(records)}")
    raise SystemExit("M382_STOP unsupported grammar outside audited pages")
   src=u16(b,p);p+=2;obj=b[p];p+=1
   width=4 if flags&16 else 2
   if p+width>end:raise ValueError("truncated target")
   target=int.from_bytes(b[p:p+width],"little");p+=width
   records.append((logical,at,src,obj,target,flags,width))
  if p!=end:raise ValueError("page parse incomplete")
 print(f"M382_PARSED auditedPages=18,19,20,21 records={len(records)}")
 for logical in (18,19,20,21):
  rs=[r for r in records if r[0]==logical]
  print(f"M382_PAGE page={logical} count={len(rs)}")
  byobj=collections.Counter(r[3] for r in rs)
  for obj,n in sorted(byobj.items()):
   xs=[r for r in rs if r[3]==obj]
   inbounds=sum(1 for r in xs if 1<=obj<=oc and r[4]<objs[obj-1][0])
   print(f"M382_PAGE_OBJECT page={logical} encodedObject={obj} count={n} inBounds={inbounds} targetMin=0x{min(r[4] for r in xs):x} targetMax=0x{max(r[4] for r in xs):x} width16={sum(r[6]==2 for r in xs)} width32={sum(r[6]==4 for r in xs)}")
   bad=[r for r in xs if not(1<=obj<=oc and r[4]<objs[obj-1][0])]
   for r in bad[:8]:
    print(f"M382_BAD_EXAMPLE page={logical} recordFile=0x{r[1]:x} source=0x{r[2]:x} obj={obj} target=0x{r[4]:x} flags=0x{r[5]:x} raw={b[r[1]:r[1]+(9 if r[6]==4 else 7)].hex()}")
  for mode in ("direct","one_based_object_shift","target_linear_absolute","target_linear_preferred_subtract"):
   ok=0
   for r in rs:
    obj,t=r[3],r[4]
    if mode=="direct":valid=1<=obj<=oc and t<objs[obj-1][0]
    elif mode=="one_based_object_shift":valid=0<=obj<oc and t<objs[obj][0]
    elif mode=="target_linear_absolute":valid=any(base<=t<base+size for size,base,_,_ in objs)
    else:valid=1<=obj<=oc and objs[obj-1][1]<=t<objs[obj-1][1]+objs[obj-1][0]
    ok+=bool(valid)
   print(f"M382_HYPOTHESIS page={logical} model={mode} valid={ok} invalid={len(rs)-ok}")
  for width in (2,4):
   xs=[r for r in rs if r[6]==width]
   if xs:
    print(f"M382_WIDTH page={logical} width={width} count={len(xs)} encodedObjects={dict(sorted(collections.Counter(r[3] for r in xs).items()))}")
 print("M382_LIMITATIONS auditedPagesOnly=1 noRuntimeLoader=1 noObservedSelector=1 alternativeModelsNotProof=1")
if __name__=="__main__":main()
