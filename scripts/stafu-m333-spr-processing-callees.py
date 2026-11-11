#!/usr/bin/env python3
"""M333: inspect candidate SPR processing callees and +0xA0 handle lifecycle.

Read-only. Physical OVL offsets are NOT LE runtime addresses. Static
linear disassembly is suggestive, not proof of execution or function extent.
Requires: pip install capstone
"""
import argparse
import hashlib
from pathlib import Path

SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=[
 ("candidate_567212",567212,568100),
 ("candidate_510232",510232,511100),
 ("producer_572028",572028,572650),
 ("a0_handle_helpers",565780,567000),
 ("a0_callers",580470,581090),
]
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 args=ap.parse_args()
 try:
  from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_CALL,CS_GRP_JUMP,CS_GRP_RET
  from capstone.x86_const import X86_OP_MEM,X86_OP_IMM
 except ImportError:
  print("M333_ERROR missing Capstone: python -m pip install capstone")
  return 2
 try: raw=args.ovl.read_bytes()
 except OSError as e:
  print(f"M333_ERROR read={e}");return 2
 digest=hashlib.sha256(raw).hexdigest()
 valid=len(raw)==2008455 and digest==SHA
 print(f"M333_INPUT bytes={len(raw)} sha256={digest} knownMatch={int(valid)}")
 if not valid:return 2
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 for name,start,end in REGIONS:
  instructions=list(md.disasm(raw[start:end],start))
  print(f"M333_REGION name={name} start={start} end={end} "
        f"decoded={len(instructions)} entryVerified=0")
  for idx,ins in enumerate(instructions):
   mem_a0=any(op.type==X86_OP_MEM and op.mem.disp==0xa0 for op in ins.operands)
   call=ins.group(CS_GRP_CALL)
   branch=ins.group(CS_GRP_JUMP)
   ret=ins.group(CS_GRP_RET)
   if not (mem_a0 or call or branch or ret):continue
   kind="FIELD_A0" if mem_a0 else ("CALL" if call else ("RETURN" if ret else "BRANCH"))
   target=None
   if (call or branch) and ins.operands and ins.operands[0].type==X86_OP_IMM:
    target=ins.operands[0].imm
   print(f"M333_EVENT region={name} kind={kind} at={ins.address} "
         f"target={target if target is not None else 'none'} "
         f"bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str}")
   if mem_a0 or call:
    for neighbor in instructions[max(0,idx-8):min(len(instructions),idx+6)]:
     print(f"M333_CONTEXT region={name} focus={ins.address} at={neighbor.address} "
           f"bytes={neighbor.bytes.hex()} instruction={neighbor.mnemonic} {neighbor.op_str}")
  print(f"M333_SUMMARY region={name} decodedBytes={sum(i.size for i in instructions)}")
 # E8 raw scan: call-site candidates; validate against disassembly before trusting.
 for target in (567212,510232,572028,565852,566224,566552,566260,566876):
  hits=[]
  for pos in range(0,len(raw)-4):
   if raw[pos]!=0xe8:continue
   disp=int.from_bytes(raw[pos+1:pos+5],"little",signed=True)
   if pos+5+disp==target:hits.append(pos)
  print(f"M333_RAW_CALL_XREF target={target} sites={','.join(map(str,hits)) if hits else 'none'} "
        "instructionBoundaryVerified=0")
 print("M333_LIMITATION arbitraryLinearSweeps=1 noCFG=1 noRuntimeTrace=1 "
       "LEFixupsUnapplied=1 rawE8MayBeData=1 imageDecoderUnverified=1")
 return 0
if __name__=="__main__":
 raise SystemExit(main())
