#!/usr/bin/env python3
"""STAFU M396: read-only export group boundary validation across loader modules."""
import argparse,hashlib,struct
from pathlib import Path
BASE=0xA4F48C
LE=0x38194
MODULES=(("LINEXE_LOADER",0x32A78,0x329F0),("D32_INT31",0x323EC,0x323D4))
def word(b,p):return struct.unpack_from("<H",b,p)[0]
def dword(b,p):return struct.unpack_from("<I",b,p)[0]
def string(b,p):
 if p<0 or p>=LE:return None
 q=b.find(b"\0",p,min(p+100,LE))
 if q<=p:return None
 v=b[p:q]
 return v.decode("ascii") if all(32<=c<127 for c in v) else None
def ptr(b,p):
 v=dword(b,p);off=v-BASE
 return v,off,string(b,off)
def entries(b,start,end):
 if start<0 or end>LE or end<start or (end-start)%8:return []
 result=[]
 for p in range(start,end,8):
  nameval,nameoff,name=ptr(b,p)
  code=dword(b,p+4)
  result.append((p,nameval,nameoff,name,code))
 return result
def report(b,label,start,end):
 records=entries(b,start,end)
 print(f"M396_EXPORTS module={label} start=0x{start:x} end=0x{end:x} count={len(records)}")
 for i,(p,nv,no,n,code) in enumerate(records):
  print(f"M396_EXPORT module={label} index={i} file=0x{p:x} namePtr=0x{nv:08x} nameFile=0x{no:x} name={n!r} codePtr=0x{code:08x} codeHigh16=0x{code>>16:04x}")
 print(f"M396_EXPORT_VALIDATION module={label} validNames={sum(bool(n) for _,_,_,n,_ in records)}/{len(records)} codeHigh88={sum((c>>16)==0x88 for *_,c in records)}/{len(records)}")
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes()
 print(f"M396_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M396_ERROR expected MZ and LE signatures")
 for name,descriptor,table in MODULES:
  print(f"M396_MODULE name={name} descriptor=0x{descriptor:x} expectedTable=0x{table:x}")
  for p in range(descriptor,descriptor+36,2):
   print(f"M396_WORD module={name} file=0x{p:x} value=0x{word(b,p):04x}")
  for p in range(descriptor,descriptor+32):
   v,off,n=ptr(b,p)
   if 0<=off<LE:
    boundary=(off-table)%8 if off>=table else None
    print(f"M396_POINTER module={name} file=0x{p:x} value=0x{v:08x} target=0x{off:x} string={n!r} entryIndex={(off-table)//8 if off>=table and boundary==0 else '-'}")
  # Descriptor hypothesis: [reserved dword, module name dword,
  # version word, version word, group0 pointer dword,
  # group1 count word, group1 pointer dword].
  nameval,nameoff,nametext=ptr(b,descriptor+4)
  p0val,p0off,_=ptr(b,descriptor+12)
  count=word(b,descriptor+16)
  p1val,p1off,_=ptr(b,descriptor+18)
  gap=p1off-p0off
  first=gap//8 if gap>=0 and gap%8==0 else None
  inferred=first+count if first is not None else None
  print(f"M396_HYPOTHESIS module={name} name={nametext!r} namePointer=0x{nameval:08x} versions={word(b,descriptor+8)},{word(b,descriptor+10)} group0=0x{p0off:x} group1=0x{p1off:x} countField={count} gapBytes={gap} firstGroupCount={first} inferredTotal={inferred}")
  if first is not None and 0<inferred<512 and p0off>=0 and p0off+inferred*8<=LE:
   report(b,name,p0off,p0off+inferred*8)
   print(f"M396_GROUPS module={name} firstStart=0x{p0off:x} firstEnd=0x{p1off:x} secondStart=0x{p1off:x} secondEnd=0x{p1off+count*8:x} contiguous={int(p1off==p0off+first*8)}")
  else:print(f"M396_GROUPS module={name} unsupportedOrOutOfRange=1")
  print(f"M396_DESCRIPTOR_RAW module={name} bytes={b[descriptor:descriptor+48].hex()}")
 # Independently locate contiguous eight-byte export runs in the two
 # known neighborhoods; avoid presupposing descriptor interpretation.
 for label,lo,hi in (("linexe",0x329d8,0x32a90),("int31",0x323a0,0x32418)):
  for start in range(lo,hi-7,4):
   run=[]
   p=start
   while p+8<=hi:
    n=string(b,dword(b,p)-BASE)
    if not n or dword(b,p+4)>>16 not in (0x80,0x88):break
    run.append(n);p+=8
   if len(run)>=2 and (start<8 or not string(b,dword(b,start-8)-BASE)):
    print(f"M396_RUN region={label} start=0x{start:x} end=0x{p:x} count={len(run)} names={run!r}")
 print("M396_LIMITATIONS groupFieldSemanticsHypothetical=1 runtimeSelectorUnknown=1 staticOnly=1")
if __name__=="__main__":main()
