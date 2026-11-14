#!/usr/bin/env python3
"""STAFU M415: helper argument provenance and local-slot flow.

Read-only, byte-anchored static diagnostic of DOS4GW helper 0x2D05E.
Reports known instructions and raw bytes; does not assert runtime semantics.
"""
import argparse,hashlib,struct
from pathlib import Path
HELPER=0x2D05E
END=0x2D0B8
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def emit(b,at,length,text,role):
 print("M415_INSN file=0x%x bytes=%s text=%s role=%s"%(at,b[at:at+length].hex(),text,role))
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print("M415_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[0x38194:0x38196]!=b"LE":raise SystemExit("M415_ERROR unexpected MZ/LE layout")
 print("M415_HELPER start=0x%x end=0x%x raw=%s"%(HELPER,END,b[HELPER:END].hex()))
 # Decode exact linear helper body with manually verified 16-bit x86 instruction
 # widths. These offsets are checked against expected bytes before reporting.
 steps=[
 (0x2d05e,"c81a0000","enter 26,0","frame"),
 (0x2d062,"1e","push ds","saved-segment"),
 (0x2d063,"b8a800","mov ax,00a8h","segment-setup"),
 (0x2d066,"8ed8","mov ds,ax","segment-setup"),
 (0x2d068,"c746ee0050","mov word [bp-18],5000h","local-init"),
 (0x2d06d,"c45e04","les bx,[bp+4]","caller-argument"),
 (0x2d070,"895efc","mov [bp-4],bx","pointer-local"),
 (0x2d073,"8c46fe","mov [bp-2],es","pointer-local"),
 (0x2d076,"268b4720","mov ax,es:[bx+32]","field-read"),
 (0x2d07a,"8946f0","mov [bp-16],ax","local-store"),
 (0x2d07d,"2bc0","sub ax,ax","zero"),
 (0x2d07f,"8946ec","mov [bp-20],ax","local-store"),
 (0x2d082,"8946e6","mov [bp-26],ax","local-store"),
 (0x2d085,"8d46e6","lea ax,[bp-26]","address-local"),
 (0x2d088,"1650","push ss; push ax","arg-pointer"),
 (0x2d08a,"8d46ee","lea ax,[bp-18]","address-local"),
 (0x2d08d,"1650","push ss; push ax","arg-pointer"),
 (0x2d08f,"1650","push ss; push ax","arg-pointer"),
 (0x2d091,"ff1e0c1a","call far [1a0ch]","call-1"),
 (0x2d095,"83c40c","add sp,12","cleanup-1"),
 (0x2d098,"6a20","push 32","arg-2"),
 (0x2d09a,"6a18","push 24","arg-2"),
 (0x2d09c,"8e06c83f","mov es,[3fc8h]","segment-setup"),
 (0x2d0a0,"26ff367a3b","push es:[3b7ah]","arg-2"),
 (0x2d0a5,"ff76fc","push [bp-4]","arg-2"),
 (0x2d0a8,"ff76fe","push [bp-2]","arg-2"),
 (0x2d0ab,"9afa3e8000","call far 0080:3efah","call-2"),
 (0x2d0b0,"83c40a","add sp,10","cleanup-2"),
 (0x2d0b3,"1f","pop ds","restore"),
 (0x2d0b4,"c9","leave","frame"),
 (0x2d0b5,"c20400","ret 4","return"),
 ]
 mismatches=0
 for at,expected,text,role in steps:
  raw=bytes.fromhex(expected)
  ok=b[at:at+len(raw)]==raw
  if not ok:
   mismatches+=1
   print("M415_MISMATCH file=0x%x expected=%s actual=%s"%(at,expected,b[at:at+len(raw)].hex()))
  else:emit(b,at,len(raw),text,role)
 print("M415_CHECK checked=%d mismatches=%d"%(len(steps),mismatches))
 print("M415_POINTER_PROVENANCE callerStack=[bp+4] farPointer=ES:BX savedOffset=[bp-4] savedSegment=[bp-2] fieldOffset=0x20 fieldValueLocal=[bp-16]")
 print("M415_CALL1_ARGS order=push_ss_bpminus26;push_ss_bpminus18;push_ss_bpminus18 countWords=6 cleanupBytes=12 note=threeFarPointers")
 print("M415_CALL2_ARGS pushOrder=32;24;es:[3b7a];[bp-4];[bp-2] countWords=5 cleanupBytes=10")
 print("M415_LOCAL_MAP bpMinus26=zeroInitialized bpMinus20=zeroInitialized bpMinus18=0x5000 bpMinus16=fieldAtPlus0x20 bpMinus4=callerPointerOffset bpMinus2=callerPointerSegment")
 print("M415_CALLER_CONTEXT file=0x2ce4f raw=%s"%(b[0x2ce4f:0x2ce54].hex()))
 print("M415_LIMITATIONS linearStaticOnly=1 noRuntimeSegmentValues=1 calleeBehaviorUnknown=1 noResourceSemanticsClaim=1")
 if mismatches:raise SystemExit("M415_ERROR byte anchors differ; do not trust provenance summary")
if __name__=="__main__":main()
