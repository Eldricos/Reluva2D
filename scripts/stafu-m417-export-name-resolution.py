#!/usr/bin/env python3
"""STAFU M417: resolve the export name behind DOS4GW far-pointer slot 1A0C.

Read-only static byte diagnostic; names and call semantics are hypotheses until
pointer translation and call argument order agree.
"""
import argparse,hashlib,struct
from pathlib import Path

CALL=0x2852a
STORE=0x2852d
DSBASE=0x30b74
PREFIX_END=0x38194
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def s16(x):return x-65536 if x>=32768 else x
def cstring(b,p,maxlen=96):
 if not 0<=p<len(b):return None
 end=b.find(b"\0",p,min(len(b),p+maxlen))
 if end<0:return None
 raw=b[p:end]
 if not raw or any(x<32 or x>126 for x in raw):return None
 return raw.decode("ascii")
def main():
 a=argparse.ArgumentParser(description=__doc__);a.add_argument("ovl",type=Path)
 b=a.parse_args().ovl.read_bytes()
 print("M417_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[0x38194:0x38196]!=b"LE":raise SystemExit("M417_ERROR unexpected MZ/LE layout")
 print("M417_WINDOW start=0x284e0 end=0x28550 bytes=%s"%b[0x284e0:0x28550].hex())
 for start,end in ((0x28500,0x28540),(0x284f0,0x28550),(0x28400,0x28550)):
  print("M417_CONTEXT start=0x%x end=0x%x bytes=%s"%(start,end,b[start:end].hex()))
 # Explicitly locate near calls around the store, computing signed displacement.
 for p in range(0x284e0,STORE):
  if b[p]==0xe8:
   target=p+3+s16(u16(b,p+1))
   print("M417_NEAR_CALL_CANDIDATE file=0x%x bytes=%s target=0x%x instructionBoundaryVerified=0"%(p,b[p:p+3].hex(),target))
 # Parse the specific call at 0x2852a only if it really is an E8.
 if b[CALL]==0xe8:
  target=CALL+3+s16(u16(b,CALL+1))
  print("M417_RESOLVER_CALL file=0x%x bytes=%s target=0x%x"%(CALL,b[CALL:CALL+3].hex(),target))
  print("M417_RESOLVER_TARGET_WINDOW start=0x%x bytes=%s"%(target,b[target:target+128].hex()))
 else:
  print("M417_RESOLVER_CALL_ASSUMPTION file=0x%x actual=%s expected=e8 status=invalid"%(CALL,b[CALL:CALL+3].hex()))
 # Locate the nearest preceding call, regardless of assumed exact location.
 calls=[p for p in range(STORE-48,STORE) if b[p]==0xe8]
 for p in calls:
  target=p+3+s16(u16(b,p+1))
  print("M417_PRESTORE_CALL file=0x%x target=0x%x gapToStore=%d preceding=%s following=%s"%(
   p,target,STORE-(p+3),b[max(0,p-32):p].hex(),b[p+3:STORE+12].hex()))
 # Decode the exact AX/DX storage and subsequent null check from bytes.
 expected=bytes.fromhex("a30c1a89160e1a8bc20b060c1a")
 print("M417_STORAGE file=0x%x expected=%s actual=%s matches=%d"%(
  STORE,expected.hex(),b[STORE:STORE+len(expected)].hex(),int(b[STORE:STORE+len(expected)]==expected)))
 # Inspect immediate operands for far string pointers and DS-offset strings.
 # Nearby push immediates are candidate argument values, not established strings.
 for p in range(0x284f0,STORE):
  op=b[p]
  if op==0x68 and p+3<=STORE:
   val=u16(b,p+1)
   print("M417_PUSH_IMM_CANDIDATE file=0x%x value=0x%04x dsCandidate=0x%x dsText=%r instructionBoundaryVerified=0"%(
    p,val,DSBASE+val,cstring(b,DSBASE+val)))
  if op==0x6a and p+2<=STORE:
   print("M417_PUSH_IMM8_CANDIDATE file=0x%x value=0x%02x instructionBoundaryVerified=0"%(p,b[p+1]))
 # A full four-byte far pointer may be encoded as two pushes. Search readable
 # strings in DS-mapped offsets explicitly present as push immediates.
 for val in (0x1c07,0x1c48,0x1c3a):
  at=DSBASE+val
  print("M417_KNOWN_OFFSET ds=0x%04x file=0x%x raw=%s text=%r"%(val,at,b[at:at+64].hex(),cstring(b,at)))
 # Compare with previously established 0080:5A20 and 0080:5BA0 call signatures.
 for name,pat in (("getproc",bytes.fromhex("9a205a8000")),("getmodule",bytes.fromhex("9aa05b8000"))):
  found=[]
  start=0
  while True:
   p=b.find(pat,start,PREFIX_END)
   if p<0:break
   found.append(p);start=p+1
  print("M417_RESOLVER_SIGNATURE name=%s total=%d nearby=%s"%(name,len(found),
   ",".join(hex(p) for p in found if 0x28300<=p<=0x28600) or "none"))
 print("M417_LIMITATIONS byteScanNotDisassembly=1 DSMappingCandidate=1 callArgumentsUnverified=1 nameNotProven=1")
if __name__=="__main__":main()
