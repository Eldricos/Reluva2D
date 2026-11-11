#!/usr/bin/env python3
"""M332: bounded static trace of state +0x84 in the suspected SPR parser.

Read-only Capstone analysis; no claim of runtime provenance or decoder identity.
"""
import argparse
import hashlib
from pathlib import Path

EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=[
 ("state_origin",579680,580150),
 ("candidate_consumers",580450,581250),
 ("loop_tail",581250,582250),
 ("other_state_use",578550,579150),
]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 args=ap.parse_args()
 try:
  from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_CALL
  from capstone.x86_const import X86_OP_MEM,X86_OP_IMM,X86_REG_INVALID
 except ImportError:
  print("M332_ERROR capstone missing; python -m pip install capstone")
  return 2
 try: data=args.ovl.read_bytes()
 except OSError as exc:
  print(f"M332_ERROR {exc}")
  return 2
 digest=hashlib.sha256(data).hexdigest()
 valid=len(data)==2008455 and digest==EXPECTED
 print(f"M332_INPUT bytes={len(data)} sha256={digest} knownMatch={int(valid)}")
 if not valid:return 2
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 for label,start,end in REGIONS:
  ins=list(md.disasm(data[start:end],start))
  print(f"M332_REGION name={label} physicalStart={start} physicalEnd={end} sweepAnchorVerified={int(start==579680 and False)}")
  found=0
  for j,i in enumerate(ins):
   refs=[]
   for op in i.operands:
    if op.type==X86_OP_MEM and op.mem.disp in (0x44,0x58,0x84):
     refs.append(f"0x{op.mem.disp:x}")
   if not refs:continue
   found+=1
   print(f"M332_FIELD region={label} physical={i.address} offsets={','.join(refs)} mnemonic={i.mnemonic} operands={i.op_str}")
   for nearby in ins[max(0,j-7):min(len(ins),j+11)]:
    print(f"M332_CONTEXT focus={i.address} at={nearby.address} bytes={nearby.bytes.hex()} mnemonic={nearby.mnemonic} operands={nearby.op_str}")
  print(f"M332_REGION_SUMMARY name={label} matches={found}")
 # Inspect each call's argument preparation in the candidate consumer span.
 start,end=580450,581100
 ins=list(md.disasm(data[start:end],start))
 for j,i in enumerate(ins):
  if not i.group(CS_GRP_CALL):continue
  dest="indirect"
  if i.operands and i.operands[0].type==X86_OP_IMM:dest=str(i.operands[0].imm)
  print(f"M332_CALL at={i.address} target={dest} sweepVerified=0")
  for near in ins[max(0,j-10):min(len(ins),j+5)]:
   print(f"M332_CALL_CONTEXT call={i.address} at={near.address} mnemonic={near.mnemonic} operands={near.op_str}")
 print("M332_LIMITATION noCFG=1 noDataflowProof=1 noRuntimeTrace=1 "
       "physicalOffsetsNotRuntimeAddresses=1 LEFixupsUnapplied=1 decoderUnverified=1")
 return 0
if __name__=="__main__":raise SystemExit(main())
