#!/usr/bin/env python3
"""STAFU M419: static control-flow audit of DOS4GW export resolver 0x2E958.

Read-only byte-anchored inspection. All semantic labels remain hypotheses
unless supported by exact instruction bytes.
"""
import argparse,hashlib,struct
from pathlib import Path
ENTRY=0x2e958
END=0x2e9d1
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def s8(x):return x-256 if x>=128 else x
def s16(x):return x-65536 if x>=32768 else x
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print("M419_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[0x38194:0x38196]!=b"LE":raise SystemExit("M419_ERROR unexpected OVL layout")
 print("M419_SCOPE entry=0x%x end=0x%x length=%d"%(ENTRY,END,END-ENTRY))
 print("M419_RAW bytes=%s"%b[ENTRY:END].hex())
 # Sequential instruction boundaries established by direct inspection of
 # the M418 helper window. Stop at the first unknown instruction rather than
 # silently reinterpreting arbitrary bytes.
 instructions=[
 ("c8080000","enter 8,0"),
 ("1e","push ds"),
 ("b8a800","mov ax,00a8h"),
 ("8ed8","mov ds,ax"),
 ("c45e08","les bx,[bp+8]"),
 ("268b07","mov ax,es:[bx]"),
 ("268b5702","mov dx,es:[bx+2]"),
 ("8946fc","mov [bp-4],ax"),
 ("8956fe","mov [bp-2],dx"),
 ("0bd0","or dx,ax"),
 ("751d","jnz short existing-handle"),
 ("ff7612","push [bp+18]"),
 ("ff7610","push [bp+16]"),
 ("9aa05b8000","call far 0080:5ba0"),
 ("5b","pop bx"),
 ("5b","pop bx"),
 ("8946fc","mov [bp-4],ax"),
 ("8956fe","mov [bp-2],dx"),
 ("c45e08","les bx,[bp+8]"),
 ("268907","mov es:[bx],ax"),
 ("26895702","mov es:[bx+2],dx"),
 ("8b46fe","mov ax,[bp-2]"),
 ("0b46fc","or ax,[bp-4]"),
 ("742a","jz short no-handle"),
 ("8b4606","mov ax,[bp+6]"),
 ("0b4604","or ax,[bp+4]"),
 ("7427","jz short no-export-name"),
 ("ff760e","push [bp+14]"),
 ("ff760c","push [bp+12]"),
 ("ff76fe","push [bp-2]"),
 ("ff76fc","push [bp-4]"),
 ("9a205a8000","call far 0080:5a20"),
 ("83c408","add sp,8"),
 ("c45e04","les bx,[bp+4]"),
 ("268907","mov es:[bx],ax"),
 ("26895702","mov es:[bx+2],dx"),
 ("0bd0","or dx,ax"),
 ("7505","jnz short success"),
 ("b80100","mov ax,1"),
 ]
 p=ENTRY;errors=0;branches=[];far_calls=[]
 for raw,label in instructions:
  expected=bytes.fromhex(raw);actual=b[p:p+len(expected)];ok=actual==expected
  print("M419_INSN file=0x%x bytes=%s match=%d text=%s"%(p,actual.hex(),int(ok),label))
  if not ok:errors+=1
  if expected[0] in (0x74,0x75) and len(expected)==2:
   dest=p+2+s8(actual[1]);branches.append((p,dest,label))
  if expected[0]==0x9a and len(expected)==5:
   far_calls.append((p,u16(actual,1),u16(actual,3),label))
  p+=len(expected)
 print("M419_PREFIX_CHECK instructions=%d mismatches=%d parsedEnd=0x%x"%(len(instructions),errors,p))
 print("M419_TAIL_RAW start=0x%x end=0x%x bytes=%s"%(p,END,b[p:END].hex()))
 for at,target,label in branches:
  print("M419_BRANCH file=0x%x target=0x%x withinHelper=%d label=%s targetBytes=%s"%(
   at,target,int(ENTRY<=target<END),label,b[target:target+12].hex()))
 for at,offset,segment,label in far_calls:
  print("M419_FAR_CALL file=0x%x selector=0x%04x offset=0x%04x label=%s"%(at,segment,offset,label))
 # The 16-bit callee cleanup is visible at the helper epilogue if present.
 for pos in range(p,END-2):
  if b[pos:pos+3] in (bytes.fromhex("c21000"),bytes.fromhex("c20800"),bytes.fromhex("c20c00")):
   print("M419_RET_CANDIDATE file=0x%x bytes=%s cleanup=%d boundaryUnverified=1"%(
    pos,b[pos:pos+3].hex(),u16(b,pos+1)))
 print("M419_CALLER_CONTEXT start=0x2e9e9 bytes=%s"%b[0x2e9e9:0x2ea0d].hex())
 print("M419_MODEL inputOutputPointer=[bp+4] moduleHandlePointer=[bp+8] exportName=[bp+12:bp+14] moduleName=[bp+16:bp+18]")
 print("M419_LIMITATIONS staticOnly=1 branchTargetsNeedBoundaryValidation=1 runtimeModuleStateUnknown=1 farCallSemanticsInferred=1")
 if errors:raise SystemExit("M419_ERROR instruction mismatch; stop interpretation")
if __name__=="__main__":main()
