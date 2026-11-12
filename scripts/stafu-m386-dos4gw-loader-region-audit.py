#!/usr/bin/env python3
"""M386: read-only DOS/4GW loader regions and LE header audit.

No disassembly or execution. Reports raw bytes around loader markers, MZ
entry point and LE header; candidate x86 patterns are leads, not proof.
"""
import argparse,hashlib,struct
from pathlib import Path
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def hexdump(b,start,length=96):
 lo=max(0,start);hi=min(len(b),start+length)
 for p in range(lo,hi,16):
  x=b[p:min(p+16,hi)]
  print(f"M386_HEX offset=0x{p:x} bytes={x.hex()} ascii={''.join(chr(c) if 32<=c<=126 else '.' for c in x)!r}")
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();print(f"M386_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ":raise SystemExit("M386_ERROR missing MZ")
 mzbytes=(u16(b,4)-1)*512+(u16(b,2) or 512)
 entry=u16(b,8)*16+u16(b,22)*16+u16(b,20)
 print(f"M386_MZ declaredImageBytes={mzbytes} entryFile=0x{entry:x} initialCS=0x{u16(b,22):x} initialIP=0x{u16(b,20):x} e_lfanew=0x{u32(b,60):x} note=e_lfanew_may_not_point_to_LE")
 hexdump(b,entry,96)
 # LE header is located by an explicit, known signature rather than e_lfanew.
 le=0x38194
 if b[le:le+2]!=b"LE":raise SystemExit("M386_ERROR LE signature mismatch")
 fields=[("byteOrder",2,1),("wordOrder",3,1),("formatLevel",4,4),("cpu",8,2),("os",10,2),("moduleVersion",12,4),("moduleFlags",16,4),("pages",20,4),("entryObject",24,4),("entryRva",28,4),("stackObject",32,4),("stackRva",36,4),("pageSize",40,4),("lastPageBytes",44,4),("fixupSize",48,4),("fixupChecksum",52,4),("loaderSize",56,4),("loaderChecksum",60,4),("objectTableOffset",64,4),("objectCount",68,4),("pageMapOffset",72,4),("resourceTableOffset",80,4),("residentNameOffset",88,4),("entryTableOffset",92,4),("moduleDirectivesOffset",96,4),("fixupPageTableOffset",104,4),("fixupRecordTableOffset",108,4),("importModuleTableOffset",112,4),("importModuleCount",116,4),("importProcTableOffset",120,4),("pageChecksumOffset",124,4),("dataPagesOffset",128,4),("preloadPageCount",132,4),("nonresidentNameOffset",136,4),("nonresidentNameLength",140,4),("nonresidentNameChecksum",144,4),("autoDataObject",148,4),("debugInfoOffset",152,4),("debugInfoLength",156,4),("instancePreloadPages",160,4),("instanceDemandPages",164,4),("heapSize",168,4),("stackSize",172,4)]
 print(f"M386_LE offset=0x{le:x}")
 for name,off,size in fields:
  v=int.from_bytes(b[le+off:le+off+size],"little")
  print(f"M386_LE_FIELD name={name} offset=0x{off:x} value=0x{v:x} decimal={v}")
 for word in (b"DOS4GLOADER",b"DOS4GMAINLOADERVECT",b"LINEXE_LOADER",b"SEGEXE_LOADER",b"EXP_LOADER",b"[Enter loader",b"In loader]",b"Leave loader",b"invalid linear executable format"):
  positions=[];p=0
  while True:
   k=b.find(word,p)
   if k<0:break
   positions.append(k);p=k+1
  print(f"M386_MARKER name={word.decode('ascii','replace')!r} count={len(positions)} offsets={','.join(hex(x) for x in positions)}")
  for k in positions[:3]:
   hexdump(b,max(0,k-32),min(128,len(b)-max(0,k-32)))
 # Search only loader-bearing prefix before LE header for common immediate
 # references to LE header offsets and 0x07 fixup type. Context-only candidates.
 prefix=b[:le]
 for pattern,label in ((b"LE","le_literal"),(b"\x07\x10","fixup_07_10_literal"),(b"\x83\xe0\x0f","and_eax_0f"),(b"\x83\xe1\x0f","and_ecx_0f")):
  positions=[];p=0
  while len(positions)<40:
   k=prefix.find(pattern,p)
   if k<0:break
   positions.append(k);p=k+1
  print(f"M386_PATTERN label={label} matchesFirst40={len(positions)} offsets={','.join(hex(x) for x in positions)} interpretation=unverified")
 print("M386_LIMITATIONS stringsNotCodeReferences=1 patternsNotControlFlow=1 runtimeLoaderNotObserved=1 relocationRoutineNotIdentified=1")
if __name__=="__main__":main()
