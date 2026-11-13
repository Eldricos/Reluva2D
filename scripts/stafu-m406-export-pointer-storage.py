#!/usr/bin/env python3
"""STAFU M406: locate stores and indirect uses of 0080:5A20 export results.

Static, read-only diagnostics. Matches are byte patterns, not proven instruction
boundaries or runtime control flow. Original game resources remain untouched.
"""
import argparse, hashlib, struct
from pathlib import Path

END=0x38194
DS_FILE_BASE=0x30B74
CALL=b"\x9a\x20\x5a\x80\x00"
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def string_at(b,p):
 if not 0<=p<END:return None
 q=b.find(b"\0",p,min(END,p+100))
 if q<=p:return None
 raw=b[p:q]
 return raw.decode("ascii") if all(32<=x<127 for x in raw) else None
def prev_name(b,p):
 # Only immediately contiguous push sequences are treated as arguments.
 cur=p
 for _ in range(8):
  if cur>=3 and b[cur-3]==0x68:
   n=u16(b,cur-2);s=string_at(b,DS_FILE_BASE+n)
   if s:return n,s
   cur-=3
  elif cur>=3 and b[cur-3:cur-1]==b"\xff\x76":cur-=3
  elif cur>=4 and b[cur-4:cur-2]==b"\xff\xb7":cur-=4
  elif cur>=1 and b[cur-1] in (0x1e,0x06,0x50,0x52,0x53,0x56,0x57):cur-=1
  else:break
 return None,None
def store_candidates(b,p):
 # Recognize consecutive AX and DX stores, optionally after add sp,8.
 cur=p+5
 if b[cur:cur+3]==b"\x83\xc4\x08":cur+=3
 out=[]
 for _ in range(3):
  if b[cur:cur+3]==b"\xa3"+b[cur+1:cur+3]:
   out.append(("ax_abs",u16(b,cur+1),cur));cur+=3
  elif b[cur:cur+4]==b"\x89\x16"+b[cur+2:cur+4]:
   out.append(("dx_abs",u16(b,cur+2),cur));cur+=4
  elif b[cur:cur+3] in (b"\x89\x46\xfc",b"\x89\x46\xf8",b"\x89\x46\xf2"):
   out.append(("ax_bp",b[cur+2]-256,cur));cur+=3
  elif b[cur:cur+3] in (b"\x89\x56\xfe",b"\x89\x56\xfa",b"\x89\x56\xf4"):
   out.append(("dx_bp",b[cur+2]-256,cur));cur+=3
  else:break
 return out
def find_all(b,pattern):
 start=0
 while True:
  p=b.find(pattern,start,END)
  if p<0:break
  yield p
  start=p+1
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print(f"M406_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[END:END+2]!=b"LE":raise SystemExit("M406_ERROR unexpected MZ/LE layout")
 calls=list(find_all(b,CALL))
 print(f"M406_CALLS count={len(calls)} sites={','.join(hex(p) for p in calls)}")
 globals_found=[]
 for p in calls:
  name_offset,name=prev_name(b,p)
  stores=store_candidates(b,p)
  print(f"M406_CALL file=0x{p:x} exportName={name!r} nameDsOffset={f'0x{name_offset:04x}' if name_offset is not None else 'unknown'} stores={';'.join(f'{kind}:0x{disp&0xffff:04x}@0x{site:x}' for kind,disp,site in stores) or 'none'}")
  print(f"M406_CONTEXT file=0x{p:x} before={b[max(0,p-24):p].hex()} after={b[p+5:min(END,p+53)].hex()}")
  ax=next(((d,s) for k,d,s in stores if k=="ax_abs"),None)
  dx=next(((d,s) for k,d,s in stores if k=="dx_abs"),None)
  if ax and dx and dx[0]==ax[0]+2:
   globals_found.append((p,name,ax[0]))
   print(f"M406_GLOBAL_POINTER call=0x{p:x} exportName={name!r} dsOffset=0x{ax[0]:04x} candidateFile=0x{DS_FILE_BASE+ax[0]:x}")
 # Indirect far calls: FF /3, memory forms including DS:[disp16] and [bx+disp16].
 # Explicitly report byte patterns without claiming runtime execution.
 for call,name,disp in globals_found:
  print(f"M406_POINTER_USAGE exportName={name!r} producer=0x{call:x} dsOffset=0x{disp:04x}")
  patterns=[
   ("far_call_abs",b"\xff\x1e"+struct.pack("<H",disp)),
   ("far_jump_abs",b"\xff\x2e"+struct.pack("<H",disp)),
   ("load_ax_abs",b"\xa1"+struct.pack("<H",disp)),
   ("load_dx_abs",b"\x8b\x16"+struct.pack("<H",disp+2)),
   ("push_low_abs",b"\xff\x36"+struct.pack("<H",disp)),
   ("push_high_abs",b"\xff\x36"+struct.pack("<H",disp+2)),
  ]
  for label,pat in patterns:
   hits=list(find_all(b,pat))
   print(f"M406_XREF exportName={name!r} dsOffset=0x{disp:04x} kind={label} count={len(hits)} sites={','.join(hex(x) for x in hits[:30]) or 'none'}")
 print(f"M406_GLOBAL_COUNT count={len(globals_found)}")
 print("M406_LIMITATIONS bytePatternsOnly=1 noVerifiedRuntimeSelectorBase=1 noControlFlowValidation=1 noExportSemanticsProof=1")
if __name__=="__main__":main()
