#!/usr/bin/env python3
"""STAFU M418: DOS16PKG export resolver argument and result flow.

Read-only byte-anchored 16-bit DOS4GW static diagnostic.
"""
import argparse,hashlib,struct
from pathlib import Path
ENTRY=0x2e9d1
DSBASE=0x30b74
def word(b,p):return struct.unpack_from("<H",b,p)[0]
def signed(w):return w-65536 if w>=32768 else w
def ascii_at(b,p):
 if not 0<=p<len(b):return None
 q=b.find(b"\0",p,min(len(b),p+128))
 if q<0:return None
 raw=b[p:q]
 return raw.decode("ascii") if raw and all(32<=c<127 for c in raw) else None
def emit(b,p,h,meaning):
 expected=bytes.fromhex(h);actual=b[p:p+len(expected)]
 print("M418_INSN file=0x%x bytes=%s expected=%s matched=%d text=%s"%(p,actual.hex(),h,int(actual==expected),meaning))
 return actual==expected
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
 b=ap.parse_args().ovl.read_bytes()
 print("M418_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[0x38194:0x38196]!=b"LE":raise SystemExit("M418_ERROR unexpected OVL MZ/LE layout")
 print("M418_CALLER_RAW start=0x2851e bytes=%s"%b[0x2851e:0x28542].hex())
 checks=[
 (0x28520,"1e","push ds: segment of module name"),
 (0x28521,"68481c","push 1c48h: DOS16PKG offset"),
 (0x28524,"1e","push ds: segment of export name"),
 (0x28525,"683a1c","push 1c3ah: int21_call_16 offset"),
 (0x28528,"e8a464","call near 2e9d1h"),
 (0x2852b,"a30c1a","mov [1a0ch],ax"),
 (0x2852e,"89160e1a","mov [1a0eh],dx"),
 ]
 # Avoid relying on guessed offsets: scan actual known complete sequence.
 sequence=bytes.fromhex("1e68481c1e683a1ce8a464a30c1a89160e1a")
 found=b.find(sequence,0x28500,0x28550)
 print("M418_CALLER_SEQUENCE matched=%d file=%s"%(int(found>=0),hex(found) if found>=0 else "none"))
 if found>=0:
  p=found
  for h,meaning in [
   ("1e","push ds (module pointer segment)"),
   ("68481c","push module offset 1c48h"),
   ("1e","push ds (export pointer segment)"),
   ("683a1c","push export offset 1c3ah"),
   ("e8a464","near resolver call"),
   ("a30c1a","save returned AX to slot low"),
   ("89160e1a","save returned DX to slot high")]:
   emit(b,p,h,meaning);p+=len(bytes.fromhex(h))
  call=found+8
  print("M418_CALLER_TARGET file=0x%x computed=0x%x expected=0x%x"%(call,call+3+signed(word(b,call+1)),ENTRY))
 for off in (0x1c48,0x1c3a):
  print("M418_STRING dsOffset=0x%x candidateFile=0x%x value=%r"%(off,DSBASE+off,ascii_at(b,DSBASE+off)))
 print("M418_RESOLVER_RAW start=0x%x bytes=%s"%(ENTRY,b[ENTRY:ENTRY+112].hex()))
 # Parse resolver sequentially, verifying exact byte boundaries and stack
 # roles rather than treating raw pattern hits as reachable instructions.
 # These byte strings are derived from M417's resolver target window.
 pattern=[
 ("c8080000","enter 8,0","frame"),
 ("1e","push ds","save"),
 ("b8a800","mov ax,00a8h","segment"),
 ("8ed8","mov ds,ax","segment"),
 ("2bc0","sub ax,ax","zero"),
 ("8946fe","mov [bp-2],ax","result-local"),
 ("8946fc","mov [bp-4],ax","result-local"),
 ("8946fa","mov [bp-6],ax","result-local"),
 ("8946f8","mov [bp-8],ax","result-local"),
 ("ff760a","push [bp+10]","incoming-word"),
 ("ff7608","push [bp+8]","incoming-word"),
 ("ff7606","push [bp+6]","incoming-word"),
 ("ff7604","push [bp+4]","incoming-word"),
 ("8d46fc","lea ax,[bp-4]","output-pointer"),
 ("1650","push ss; push ax","output-far-pointer"),
 ("8d46f8","lea ax,[bp-8]","output-pointer"),
 ("1650","push ss; push ax","output-far-pointer"),
 ("e856ff","call near relative -170","internal-call"),
 ("8b46f8","mov ax,[bp-8]","return-low"),
 ("8b56fa","mov dx,[bp-6]","return-high"),
 ("1f","pop ds","restore"),
 ("c9","leave","frame"),
 ("c20800","ret 8","callee-cleanup"),
 ]
 p=ENTRY;bad=0;call_at=None
 for h,meaning,role in pattern:
  if not emit(b,p,h,meaning):bad+=1
  if role=="internal-call":call_at=p
  p+=len(bytes.fromhex(h))
 print("M418_RESOLVER_CHECK instructions=%d mismatches=%d end=0x%x"%(len(pattern),bad,p))
 if call_at is not None:
  target=call_at+3+signed(word(b,call_at+1))
  print("M418_INTERNAL_CALL file=0x%x bytes=%s target=0x%x preceding=%s following=%s"%(call_at,b[call_at:call_at+3].hex(),target,b[call_at-28:call_at].hex(),b[call_at+3:call_at+20].hex()))
  print("M418_INTERNAL_TARGET_RAW start=0x%x bytes=%s"%(target,b[target:target+112].hex()))
  for name,pat in (("direct_getproc",bytes.fromhex("9a205a8000")),("direct_getmodule",bytes.fromhex("9aa05b8000"))):
   hits=[];q=target
   while True:
    q=b.find(pat,q,min(len(b),target+512))
    if q<0:break
    hits.append(q);q+=1
   print("M418_TARGET_SIGNATURE name=%s within512=%s"%(name,",".join(hex(x) for x in hits) or "none"))
 print("M418_ARGUMENT_MODEL callerPushWords=4 resolverRetCleanup=8 internalPushWords=8 internalCleanup=unknown")
 print("M418_RESULT_MODEL outputSlot=[bp-8:bp-6] returnAX=[bp-8] returnDX=[bp-6] destinationDS=[1a0c:1a0e]")
 print("M418_LIMITATIONS staticOnly=1 runtimeSegmentsUnverified=1 internalHelperSemanticsUnknown=1 fullCFGUnverified=1")
 if bad:raise SystemExit("M418_ERROR resolver byte anchors mismatch; do not trust argument model")
if __name__=="__main__":main()
