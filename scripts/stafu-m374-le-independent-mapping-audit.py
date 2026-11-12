#!/usr/bin/env python3
"""STAFU M374: independent LE page-map/fixup-index alignment audit.

Read-only, strict original-image fingerprint. This diagnostic does not execute
original code, modify game resources, or assume reconstructed handlers execute.
"""
import argparse, hashlib, struct
from pathlib import Path
from collections import Counter

LE=229780
EXPECTED_SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
def word(b,p): return struct.unpack_from("<H",b,p)[0]
def dword(b,p): return struct.unpack_from("<I",b,p)[0]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument("ovl",type=Path);a=p.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print(f"M374_INPUT bytes={len(b)} sha256={sha}")
 if len(b)!=2008455 or sha!=EXPECTED_SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M374_ERROR input identity mismatch")
 n=dword(b,LE+0x14);size=dword(b,LE+0x28)
 objoff=LE+dword(b,LE+0x40);count=dword(b,LE+0x44)
 mapoff=LE+dword(b,LE+0x48);fixindex=LE+dword(b,LE+0x68)
 fixrecords=LE+dword(b,LE+0x6c);dataorigin=LE+dword(b,LE+0x80)
 objs=[dict(size=dword(b,objoff+24*i),base=dword(b,objoff+24*i+4),first=dword(b,objoff+24*i+12),pages=dword(b,objoff+24*i+16)) for i in range(count)]
 if n!=371 or size!=4096 or count!=2:raise SystemExit("M374_ERROR unexpected layout")
 print(f"M374_HEADER pages={n} pageSize={size} objectCount={count} mapPhysical=0x{mapoff:x} fixIndexPhysical=0x{fixindex:x} fixRecordsPhysical=0x{fixrecords:x} dataOriginPhysical=0x{dataorigin:x}")
 for i,o in enumerate(objs,1):print(f"M374_OBJECT id={i} base=0x{o['base']:x} size=0x{o['size']:x} first={o['first']} pages={o['pages']}")
 entries=[(int.from_bytes(b[mapoff+4*i:mapoff+4*i+3],"big"),b[mapoff+4*i+3]) for i in range(n)]
 indices=[dword(b,fixindex+4*i) for i in range(n+1)]
 print(f"M374_MAP total={len(entries)} uniquePageNumbers={len(set(x for x,f in entries))} zeroPageNumbers={sum(x==0 for x,f in entries)} nonzeroFlags={sum(f!=0 for x,f in entries)} firstFive={entries[:5]} lastFive={entries[-5:]}")
 print(f"M374_INDEX entries={len(indices)} monotonic={int(all(x<=y for x,y in zip(indices,indices[1:])))} first={indices[0]} last={indices[-1]}")
 def physical(logical,variant):
  if not 1<=logical<=n:return None
  if variant=="map_big":
   page,flag=entries[logical-1]
   if not page or flag:return None
   return dataorigin+(page-1)*size
  if variant=="map_little":
   i=logical-1;page=int.from_bytes(b[mapoff+4*i:mapoff+4*i+3],"little")
   return dataorigin+(page-1)*size if page else None
  if variant=="linear":
   return dataorigin+(logical-1)*size
  if variant=="linear_plus_one":
   return dataorigin+logical*size
  if variant=="linear_minus_one":
   return dataorigin+(logical-2)*size
 def parse(stream):
  records=[];at=0
  while at<len(stream):
   start=at
   if at+7>len(stream):return None,"truncated"
   typ,flags=stream[at],stream[at+1];at+=2
   if typ!=7 or flags not in (0,16):return None,f"unsupported_{typ:02x}_{flags:02x}"
   off=word(stream,at);at+=2
   obj=stream[at];at+=1
   width=4 if flags==16 else 2
   if at+width>len(stream):return None,"truncated_target"
   target=dword(stream,at) if width==4 else word(stream,at);at+=width
   if not 1<=obj<=len(objs) or target>=objs[obj-1]["size"]:return None,"invalid_target"
   records.append((start,off,obj,target,flags))
  return records,"complete"
 for logical,label,offsets in ((18,"preceding",()),(19,"object_state",tuple(0x334+4*i for i in range(7))),(20,"global_state",tuple(0x488+4*i for i in range(7))),(21,"following",())):
  print(f"M374_PAGE logical={logical} label={label} mappedNumber={entries[logical-1][0]} mapFlag={entries[logical-1][1]} indexPrev={indices[logical-1]} indexNext={indices[logical]}")
  for shift in (-2,-1,0,1,2):
   slot=logical-1+shift
   if slot<0 or slot+1>=len(indices):continue
   start,end=indices[slot:slot+2]
   stream=b[fixrecords+start:fixrecords+end]
   records,status=parse(stream)
   hits=sum(sum(r[1]==x for r in records) == 1 for x in offsets) if records is not None else 0
   overlaps=sum(r[1]+4>size for r in records) if records is not None else -1
   print(f"M374_FIXINDEX logical={logical} shift={shift:+d} slot={slot} length={len(stream)} status={status} records={len(records) if records is not None else -1} tableHits={hits}/{len(offsets)} sourceCrossesPage={overlaps}")
  for variant in ("map_big","map_little","linear","linear_plus_one","linear_minus_one"):
   phys=physical(logical,variant)
   valid=phys is not None and 0<=phys and phys+size<=len(b)
   chunk=b[phys:phys+size] if valid else b""
   samples=",".join(f"{off:03x}:{chunk[off:off+8].hex()}" for off in offsets) if chunk else "unavailable"
   print(f"M374_DATAMAP logical={logical} variant={variant} physical={f'0x{phys:x}' if phys is not None else 'none'} inFile={int(valid)} pageSha256={hashlib.sha256(chunk).hexdigest()[:16] if chunk else 'none'} tableRaw={samples}")
  if offsets:
   expected=(b"\x2e\xff\x24\x85\x34\x23\x01\x00" if logical==19 else b"\x2e\xff\x24\x85\x88\x34\x01\x00")
   # Check only the known jump instruction location by searching the
   # mapped page; this does not imply its target table is valid.
   for variant in ("map_big","linear","linear_plus_one","linear_minus_one"):
    phys=physical(logical,variant)
    chunk=b[phys:phys+size] if phys is not None and 0<=phys and phys+size<=len(b) else b""
    occurrences=[i for i in range(len(chunk)) if chunk.startswith(expected,i)]
    print(f"M374_SIGNATURE logical={logical} variant={variant} jumpOccurrences={len(occurrences)} offsets={','.join(hex(i) for i in occurrences[:8]) or 'none'}")
 print("M374_SCOPE fixedImageOnly=1 independentMapAndIndexVariants=1 noRuntimeLoader=1 noVerifiedHandlerBoundaries=1 noSPRDecoder=1")
if __name__=="__main__":main()
