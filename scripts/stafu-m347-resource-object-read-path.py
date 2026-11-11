#!/usr/bin/env python3
"""M347: read-only resource-object stream positioning and state-flow survey.

All addresses are physical STTNG.OVL offsets. LE fixups are unapplied.
Instruction-aligned anchors are distinguished from heuristic function bounds.
Requires capstone.
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

EXPECTED_SIZE=2008455
EXPECTED_SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
FUNCTIONS=(
    ("object_position_566260",566260,566552),
    ("object_state_566552",566552,566876),
    ("object_transition_566876",566876,567212),
    ("stream_dispatch_570696",570696,570950),
    ("stream_position_wrapper_563123",563123,563166),
    ("stream_position_core_517688",517688,517764),
)
ANCHORS=(
    ("position_call",566370,566450),
    ("object_followup",580788,580835),
    ("other_position_call",569740,569795),
    ("state_call",570080,570135),
)
TRACKED={0x34,0x40,0x44,0x48,0x50,0x54,0x58,0x5c,0x60,0x64,0x70,0x74,0x78,0x84,0xa0,0x409c,0x40a0,0x40a4,0x40a8,0x40ac}

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def walk(md,data,lo,hi,max_ins=1600):
    pending=deque([lo])
    seen={}
    edges=[]
    failures=[]
    while pending and len(seen)<max_ins:
        pc=pending.popleft()
        while lo<=pc<hi and pc not in seen and len(seen)<max_ins:
            ins=next(md.disasm(data[pc:min(pc+15,hi)],pc,count=1),None)
            if ins is None:
                failures.append(pc)
                break
            seen[pc]=ins
            m=ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                dest=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                edges.append((pc,dest,m))
                if dest is not None and lo<=dest<hi:
                    pending.append(dest)
                if m=="jmp":
                    break
            if m.startswith("ret") or m in ("int3","ud2","hlt"):
                break
            pc+=ins.size
    return seen,edges,failures

def emit(label,ins):
    print(f"M347_INSN {label} {fmt(ins)}")
    if ins.mnemonic.startswith("call"):
        dest=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
        print(f"M347_CALL site={ins.address} target={dest if dest is not None else 'indirect'}")
    for n,op in enumerate(ins.operands):
        if op.type==X86_OP_MEM:
            base=ins.reg_name(op.mem.base) if op.mem.base else "-"
            idx=ins.reg_name(op.mem.index) if op.mem.index else "-"
            print(f"M347_MEM site={ins.address} operand={n} base={base} index={idx} disp={op.mem.disp:+#x} size={op.size} tracked={int(op.mem.disp in TRACKED)}")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("ovl",type=Path)
    a=ap.parse_args()
    data=a.ovl.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    print(f"M347_INPUT bytes={len(data)} sha256={digest}")
    if len(data)!=EXPECTED_SIZE or digest!=EXPECTED_SHA:
        raise SystemExit("M347_ERROR unexpected STTNG.OVL size/hash")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    for name,lo,hi in FUNCTIONS:
        insns,edges,failures=walk(md,data,lo,hi)
        print(f"M347_FUNCTION name={name} entry={lo} boundary={hi} decoded={len(insns)} edges={len(edges)} failures={len(failures)} boundaryHeuristic=1")
        for pc in sorted(insns):
            emit(f"function={name}",insns[pc])
        for site,target,kind in edges:
            print(f"M347_EDGE function={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(lo<=target<hi))}")
        for site in failures:
            print(f"M347_FAILURE function={name} site={site}")
        print(f"M347_FUNCTION_END name={name}")
    for name,lo,hi in ANCHORS:
        print(f"M347_ANCHOR name={name} start={lo} end={hi} startAlignmentUnverified=1")
        for ins in md.disasm(data[lo:hi],lo):
            emit(f"anchor={name}",ins)
        print(f"M347_ANCHOR_END name={name}")
    for target in (563123,570696,566260,566552,566876,517688):
        sites=[]
        for site in range(len(data)-4):
            if data[site]==0xe8 and site+5+int.from_bytes(data[site+1:site+5],"little",signed=True)==target:
                sites.append(site)
        nearby=[s for s in sites if 563000<=s<=582500]
        print(f"M347_RAW_CALLERS target={target} total={len(sites)} nearby={','.join(map(str,nearby)) or '-'}")
    print("M347_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 indirectBranchesUnresolved=1 functionBoundsHeuristic=1 rawCallersMayBeData=1 runtimeProvenanceUnverified=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
