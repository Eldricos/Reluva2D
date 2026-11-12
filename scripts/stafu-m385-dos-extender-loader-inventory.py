#!/usr/bin/env python3
"""M385: read-only DOS extender and LE loader entry-path inventory.

Searches original STTNG.EXE and STTNG.OVL for MZ/LE signatures, DOS
extender strings, executable overlays and LE entry metadata. Does not
execute game code or assume a particular loader.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def strings(b,minlen=5):
 out=[];start=None
 for i,c in enumerate(b):
  if 32<=c<=126:
   if start is None:start=i
  else:
   if start is not None and i-start>=minlen:out.append((start,b[start:i].decode("ascii")))
   start=None
 if start is not None and len(b)-start>=minlen:out.append((start,b[start:].decode("ascii")))
 return out
def inspect(path,ovl=False):
 b=path.read_bytes()
 print(f"M385_FILE name={path.name} bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()} mz={int(b[:2]==b'MZ')}")
 if b[:2]==b"MZ" and len(b)>=64:
  fields={"lastPageBytes":u16(b,2),"pages":u16(b,4),"relocations":u16(b,6),"headerParagraphs":u16(b,8),"minAlloc":u16(b,10),"maxAlloc":u16(b,12),"initialSS":u16(b,14),"initialSP":u16(b,16),"initialIP":u16(b,20),"initialCS":u16(b,22),"relocationTable":u16(b,24),"overlayNumber":u16(b,26),"newHeaderPointer":u32(b,60)}
  print("M385_MZ "+' '.join(f"{k}=0x{v:x}" for k,v in fields.items()))
  size=(fields["pages"]-1)*512+(fields["lastPageBytes"] or 512) if fields["pages"] else 0
  print(f"M385_MZ_IMAGE declaredBytes={size} headerBytes={fields['headerParagraphs']*16} entryFileOffset=0x{fields['headerParagraphs']*16+fields['initialCS']*16+fields['initialIP']:x} declaredWithinFile={int(size<=len(b))}")
  if size<len(b):print(f"M385_TRAILING bytes={len(b)-size} first64={b[size:size+64].hex()}")
  for i in range(min(fields["relocations"],12)):
   p=fields["relocationTable"]+i*4
   if p+4<=len(b):print(f"M385_MZ_RELOCATION index={i} offset=0x{u16(b,p):x} segment=0x{u16(b,p+2):x}")
 for magic in (b"LE",b"LX",b"BW",b"PMW",b"DOS/4G",b"DOS4GW",b"RATIONAL",b"CauseWay",b"CWSDPMI",b"DPMI",b"Phar Lap",b"GO32",b"VCPI"):
  hits=[];pos=0
  while len(hits)<24:
   k=b.find(magic,pos)
   if k<0:break
   hits.append(k);pos=k+1
  if hits:print(f"M385_SIGNATURE needle={magic.decode('ascii','replace')} countAtLeast={len(hits)} offsets={','.join(hex(x) for x in hits)}")
 for p in (u32(b,60) if b[:2]==b"MZ" and len(b)>=64 else -1,229780 if ovl else -1):
  if 0<=p<len(b)-0xc0 and b[p:p+2] in (b"LE",b"LX"):
   print(f"M385_LE_HEADER offset=0x{p:x} magic={b[p:p+2].decode()} cpu=0x{u16(b,p+8):x} os=0x{u16(b,p+10):x} moduleFlags=0x{u32(b,p+0x10):x} pages={u32(b,p+0x14)} eipObject={u32(b,p+0x18)} eip=0x{u32(b,p+0x1c):x} espObject={u32(b,p+0x20)} esp=0x{u32(b,p+0x24):x} autoDataObject={u32(b,p+0x5c)}")
 words=("dos/4g","dos4gw","rational","dos extender","dpmi","vcpi","causeway","phar lap","linear executable","protected mode","dos/32","dos32","stub","loader","copyright")
 ss=strings(b)
 for word in words:
  hits=[(p,t[:160]) for p,t in ss if word in t.lower()]
  for p,t in hits[:12]:print(f"M385_TEXT keyword={word!r} offset=0x{p:x} text={t!r}")
  if len(hits)>12:print(f"M385_TEXT_MORE keyword={word!r} remaining={len(hits)-12}")
 print(f"M385_STRINGS totalPrintableRuns5={len(ss)}")
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path);ap.add_argument("--exe",type=Path,default=None)
 a=ap.parse_args()
 inspect(a.ovl,True)
 if a.exe:
  if a.exe.exists():inspect(a.exe)
  else:print(f"M385_EXE_NOT_FOUND path={a.exe}")
 print("M385_LIMITATIONS staticIdentificationOnly=1 loaderNotExecuted=1 runtimeSelectorsUnobserved=1 signatureHitsNotProof=1")
if __name__=="__main__":main()
