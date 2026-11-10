#!/usr/bin/env python3
"""M331: trace candidate SPR chunk state-field accesses and pointer advances.

Read-only, Capstone 5 required. All addresses are OVL physical offsets;
linear sweeps are NOT a complete control-flow graph or runtime trace.
"""
import argparse
import hashlib
from pathlib import Path

SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=[
 ("spr_state_setup",575250,575650),
 ("geometry_and_tag_processing",577491,580150),
 ("tag_processing_tail",580150,582250),
 ("other_geometry_caller",586300,586750),
 ("list_chunk_parser",919740,920160),
]
FIELDS={0x34:"record_header_pointer",0x38:"state_flags",0x44:"current_chunk_pointer",
        0x58:"record_payload_pointer_candidate",0x9c:"derived_pointer_or_value"}
def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument("ovl",type=Path)
 a=p.parse_args()
 try:
  from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_CALL,CS_GRP_JUMP
  from capstone.x86_const import X86_OP_MEM,X86_OP_IMM
 except ImportError:
  print("M331_ERROR install Capstone: python -m pip install capstone")
  return 2
 try: data=a.ovl.read_bytes()
 except OSError as e:
  print(f"M331_ERROR input={e}")
  return 2
 digest=hashlib.sha256(data).hexdigest()
 ok=len(data)==2008455 and digest==SHA
 print(f"M331_INPUT bytes={len(data)} sha256={digest} knownMatch={int(ok)}")
 if not ok:return 2
 md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
 for name,start,end in REGIONS:
  print(f"M331_REGION name={name} start={start} end={end} verifiedEntry=0")
  insns=list(md.disasm(data[start:end],start))
  boundaries={i.address for i in insns}
  matches=[]
  for index,ins in enumerate(insns):
   hits=[]
   for op in ins.operands:
    if op.type==X86_OP_MEM and op.mem.disp in FIELDS:
     hits.append(FIELDS[op.mem.disp])
   if hits:
    matches.append(index)
    print(f"M331_FIELD region={name} at={ins.address} fields={','.join(sorted(set(hits)))} "
          f"insn={ins.mnemonic} {ins.op_str}")
   if ins.group(CS_GRP_CALL):
    target=(ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None)
    print(f"M331_CALL region={name} at={ins.address} target={target if target is not None else 'indirect'} "
          f"insn={ins.mnemonic} {ins.op_str}")
   if ins.group(CS_GRP_JUMP) and ins.operands and ins.operands[0].type==X86_OP_IMM:
    target=ins.operands[0].imm
    if target<start or target>=end:
     print(f"M331_OUTBOUND_BRANCH region={name} at={ins.address} target={target}")
  for index in matches:
   ins=insns[index]
   print(f"M331_CONTEXT region={name} focus={ins.address}")
   for nearby in insns[max(0,index-3):min(len(insns),index+5)]:
    print(f"M331_INSN at={nearby.address} bytes={nearby.bytes.hex()} "
          f"mnemonic={nearby.mnemonic} operands={nearby.op_str}")
  print(f"M331_SUMMARY region={name} instructions={len(insns)} fieldAccesses={len(matches)}")
 print("M331_LIMITATION arbitrarySweepStarts=1 memoryFieldNamesHypotheses=1 "
       "noRuntimeValues=1 noLEFixups=1 noVerifiedPayloadConsumer=1")
 return 0
if __name__=="__main__":
 raise SystemExit(main())
