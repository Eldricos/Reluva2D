#!/usr/bin/env python3
"""STAFU M405: correlate paired loader-service arguments with DOS4GW strings.

Read-only byte analysis. Distinguishes direct file offsets, DS-relative offsets,
and affine pointers; does not assume runtime DS base or service semantics.
"""
import argparse,hashlib,struct
from pathlib import Path
PREFIX=0x38194
AFFINE=0xA4F48C
DS_FILE_BASE=0x30B74
SERVICES={0x5BA0:"module_candidate",0x5A20:"export_candidate"}
def word(b,p):return struct.unpack_from("<H",b,p)[0]
def dword(b,p):return struct.unpack_from("<I",b,p)[0]
def ascii_at(b,p):
 if p<0 or p>=PREFIX:return None
 end=b.find(b"\0",p,min(PREFIX,p+128))
 if end<0 or end==p:return None
 raw=b[p:end]
 if not all(32<=c<127 for c in raw):return None
 return raw.decode("ascii")
def findcalls(b,off):
 sig=b"\x9a"+struct.pack("<HH",off,0x80);pos=0
 while True:
  pos=b.find(sig,pos,PREFIX)
  if pos<0:return
  yield pos;pos+=1
def resolve(b,imm):
 for model,p in (("direct_file",imm),("ds_file_base_plus_offset",DS_FILE_BASE+imm),
                 ("affine_full_pointer",imm-AFFINE)):
  s=ascii_at(b,p)
  print(f"M405_RESOLVE value=0x{imm:x} model={model} file=0x{p:x} string={s!r}")
def pushes(b,p):
 cur=p;out=[]
 for _ in range(8):
  if cur>=3 and b[cur-3]==0x68:
   q=cur-3;out.append((q,"imm16",word(b,q+1)));cur=q
  elif cur>=4 and b[cur-4:cur-2]==b"\xff\xb7":
   q=cur-4;out.append((q,"bx_disp16",word(b,q+2)));cur=q
  elif cur>=3 and b[cur-3:cur-1]==b"\xff\x76":
   q=cur-3;out.append((q,"bp_disp8",b[q+2]-256 if b[q+2]>=128 else b[q+2]));cur=q
  elif cur>=1 and b[cur-1] in (0x1e,0x06,0x50,0x52):
   q=cur-1;out.append((q,{0x1e:"ds",0x06:"es",0x50:"ax",0x52:"dx"}[b[q]],None));cur=q
  else:break
 return list(reversed(out))
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print(f"M405_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[PREFIX:PREFIX+2]!=b"LE":raise SystemExit("M405_ERROR unexpected MZ/LE layout")
 print(f"M405_MODELS affineBase=0x{AFFINE:x} candidateDsFileBase=0x{DS_FILE_BASE:x} runtimeDsUnverified=1")
 for off,role in SERVICES.items():
  sites=list(findcalls(b,off))
  print(f"M405_SERVICE selector=0x0080 offset=0x{off:04x} roleHypothesis={role} count={len(sites)}")
  for p in sites:
   args=pushes(b,p)
   print(f"M405_CALL service=0x{off:04x} file=0x{p:x} args={';'.join(f'{q:x}:{kind}:{v}' for q,kind,v in args)}")
   for q,kind,v in args:
    if kind=="imm16":
     print(f"M405_IMMEDIATE service=0x{off:04x} call=0x{p:x} at=0x{q:x} value=0x{v:04x}")
     for model,fp in (("direct_file",v),("ds_file_base_plus_offset",DS_FILE_BASE+v)):
      s=ascii_at(b,fp)
      print(f"M405_STRING service=0x{off:04x} call=0x{p:x} model={model} file=0x{fp:x} value={s!r}")
    elif kind=="bx_disp16":
     for index in range(5):
      fp=DS_FILE_BASE+v+4*index
      if fp+4>PREFIX:break
      ptr=dword(b,fp)
      target=ptr-AFFINE
      print(f"M405_TABLE service=0x{off:04x} call=0x{p:x} displacement=0x{v:04x} index={index} pointer=0x{ptr:08x} stringFile=0x{target:x} string={ascii_at(b,target)!r}")
 # Cross-check names against both validated module export tables.
 for module,base,count in (("LINEXE_LOADER",0x329F0,17),("D32_INT31",0x323D4,3)):
  print(f"M405_EXPORT_TABLE module={module} file=0x{base:x} count={count}")
  for i in range(count):
   q=base+i*8
   np,cp=dword(b,q),dword(b,q+4)
   print(f"M405_EXPORT module={module} index={i} name={ascii_at(b,np-AFFINE)!r} nameFile=0x{np-AFFINE:x} codePointer=0x{cp:08x}")
 print("M405_LIMITATIONS runtimeDsBaseUnverified=1 callSemanticsUnverified=1 stringCorrelationNotExecutionProof=1")
if __name__=="__main__":main()
