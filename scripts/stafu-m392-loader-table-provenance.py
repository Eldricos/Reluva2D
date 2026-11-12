#!/usr/bin/env python3
"""STAFU M392: DOS/4GW loader table provenance, static pointer and string audit.

Read-only; requires only Python standard library. No runtime semantics assumed.
"""
import argparse,collections,hashlib,struct
from pathlib import Path
LE=0x38194; TABLE=0x329f0; N=17
NAMES=("INIT","DEINIT","LOADER","UNLOAD","FREEMAP","REL","UNREL","CANLOAD","GETLOADTABLE","GETLOADNAME","GETERRSTR","LINEXE_LOADMODULE","LINEXE_GETPROCADDR","LINEXE_GETFLATREGS","LINEXE_FREEMODULE","LINEXE_GETMODHANDLE","XXXGET_OBJECT_MAP")
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def occurrences(b,needle,start=0,end=None,limit=20):
 out=[];p=start;end=len(b) if end is None else end
 while len(out)<limit:
  p=b.find(needle,p,end)
  if p<0:break
  out.append(p);p+=1
 return out
def fmt(xs):return ",".join(f"0x{x:x}" for x in xs)
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes()
 print(f"M392_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M392_ERROR expected MZ and LE signatures")
 print(f"M392_REGIONS mzDeclaredEnd=0x{((struct.unpack_from('<H',b,4)[0]-1)*512+(struct.unpack_from('<H',b,2)[0] or 512)):x} leStart=0x{LE:x} table=0x{TABLE:x}")
 print(f"M392_TABLE_CONTEXT before={b[TABLE-64:TABLE].hex()} after={b[TABLE+N*8:TABLE+N*8+96].hex()}")
 for i,name in enumerate(NAMES):
  np=u32(b,TABLE+8*i);cp=u32(b,TABLE+8*i+4)
  print(f"M392_ENTRY index={i} name={name} namePointer=0x{np:08x} codePointer=0x{cp:08x} nameHigh16=0x{np>>16:04x} codeHigh16=0x{cp>>16:04x} nameLow16=0x{np&65535:04x} codeLow16=0x{cp&65535:04x}")
 # The independently established name affine mapping is a *file* mapping,
 # not necessarily a runtime descriptor base.
 name_base=0xA4F48C
 for i,name in enumerate(NAMES):
  v=u32(b,TABLE+8*i);p=v-name_base
  q=b.find(b"\0",p,min(p+80,LE)) if 0<=p<LE else -1
  actual=b[p:q] if q>p else b""
  print(f"M392_NAME_CHECK index={i} expected={name!r} file=0x{p:x} atStringStart={int(p==0 or (p>0 and b[p-1]==0))} actual={actual.decode('ascii','backslashreplace')!r} exact={int(actual==name.encode())}")
 # Look for independent uses of pointer values, pointer low words and
 # table addresses; classify MZ, loader prefix and LE image separately.
 regions=(("mz",0,62580),("loader",62580,LE),("le",LE,len(b)))
 targets=[("tableFile",TABLE),("nameBase",name_base),("codeBase",0x880000)]
 for i in (0,5,6,11,16):
  targets.extend(((f"codePtr_{NAMES[i]}",u32(b,TABLE+8*i+4)),(f"namePtr_{NAMES[i]}",u32(b,TABLE+8*i))))
 for label,v in targets:
  for width in (2,4):
   if width==2 and v>0xffff:val=v&65535
   else:val=v
   if val>=1<<(8*width):continue
   needle=val.to_bytes(width,"little")
   for region,start,end in regions:
    hits=occurrences(b,needle,start,end,20)
    print(f"M392_REFERENCE target={label} width={width} value=0x{val:x} region={region} countFirst20={len(hits)} offsets={fmt(hits)}")
 # Inspect pair and pointer neighborhoods, and compare the same 8-byte
 # shape elsewhere in loader prefix. Similar layout is not proof of linkage.
 for i in range(N):
  p=TABLE+i*8
  print(f"M392_PAIR_WINDOW index={i} file=0x{p:x} preceding8={b[p-8:p].hex()} pair={b[p:p+8].hex()} following8={b[p+8:p+16].hex()}")
 for label,needle in (("LINEXE_LOADER",b"LINEXE_LOADER\0"),("LINEXE_GETFLATREGS",b"LINEXE_GETFLATREGS\0"),("DOS4GLOADER",b"DOS4GLOADER\0"),("LINEXE_MOD_INFO_SEG",b"LINEXE_MOD_INFO_SEG\0")):
  hits=occurrences(b,needle,0,LE,30)
  print(f"M392_MARKER name={label} countFirst30={len(hits)} offsets={fmt(hits)}")
  for p in hits[:8]:print(f"M392_MARKER_WINDOW name={label} file=0x{p:x} preceding32={b[max(0,p-32):p].hex()} following32={b[p+len(needle):p+len(needle)+32].hex()}")
 # Relative differences are preserved under a common affine base and can
 # expose whether the two columns behave as distinct address spaces.
 for i in range(1,N):
  n0,c0=u32(b,TABLE+8*(i-1)),u32(b,TABLE+8*(i-1)+4)
  n1,c1=u32(b,TABLE+8*i),u32(b,TABLE+8*i+4)
  print(f"M392_DELTA index={i} nameDelta={n1-n0:+d} codeDelta={c1-c0:+d}")
 print("M392_LIMITATIONS rawOccurrencesNotReferences=1 pointerBasesNotRuntimeVerified=1 tableConsumersUnobserved=1 noRuntimeExecution=1")
if __name__=="__main__":main()
