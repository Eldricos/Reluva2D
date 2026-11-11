#!/usr/bin/env python3
"""M345: read-only stream-to-SPR dataflow evidence survey.

This is a static survey, not a proof of runtime reachability or SPR decoding.
Offsets are physical positions in the verified STTNG.OVL; LE fixups are not applied.
Requires capstone.
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

SIZE=2008455
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=(
    ("read_to_object_569800",569800,570120),
    ("resource_read_565300",565300,565760),
    ("resource_read_568150",568150,568540),
    ("spr_processing_580420",580420,581180),
    ("spr_processing_581350",581350,581850),
)
# Known, instruction-aligned anchors from previous surveys. Do not treat arbitrary
# region starts as verified function entrypoints.
ANCHORS=(
    ("read_call_570032",570032,570070),
    ("read_call_565466",565466,565515),
    ("read_call_568294",568294,568340),
    ("read_call_568454",568454,568500),
    ("spr_construct_external",580574,580605),
    ("spr_construct_embedded",580767,580830),
    ("object_embedded_buffer",567408,567445),
    ("object_external_buffer",572340,572380),
)
TARGETS=(563062,563123,517764,566260,566552,566876,567212,572028)
FIELDS={0x44,0x48,0x50,0x54,0x58,0x5c,0x60,0x64,0x70,0x74,0x78,0x84,0x94,0xa0}

def format_ins(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def walk(md,data,start,end,limit=1100):
    queue=deque([start])
    seen={}
    edges=[]
    errors=[]
    while queue and len(seen)<limit:
        pc=queue.popleft()
        while start<=pc<end and pc not in seen and len(seen)<limit:
            ins=next(md.disasm(data[pc:min(pc+15,end)],pc,count=1),None)
            if ins is None:
                errors.append(pc)
                break
            seen[pc]=ins
            m=ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                target=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                edges.append((pc,target,m))
                if target is not None and start<=target<end:
                    queue.append(target)
                if m=="jmp":
                    break
            if m.startswith("ret") or m in ("ud2","hlt","int3"):
                break
            pc+=ins.size
    return seen,edges,errors

def emit_ins(prefix,ins):
    print(f"{prefix} {format_ins(ins)}")
    if ins.mnemonic.startswith("call"):
        t=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
        print(f"M345_CALL site={ins.address} target={t if t is not None else 'indirect'}")
    for n,op in enumerate(ins.operands):
        if op.type==X86_OP_MEM:
            base=ins.reg_name(op.mem.base) if op.mem.base else "-"
            index=ins.reg_name(op.mem.index) if op.mem.index else "-"
            d=op.mem.disp
            if d in FIELDS or d in (0x409c,0x40a0,0x40a4,0x40a8,0x40ac):
                print(f"M345_FIELD site={ins.address} operand={n} base={base} index={index} displacement={d:+#x} size={op.size}")

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    data=args.ovl.read_bytes()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M345_INPUT bytes={len(data)} sha256={sha}")
    if len(data)!=SIZE or sha!=SHA:
        raise SystemExit("M345_ERROR unexpected STTNG.OVL size/hash")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    for name,lo,hi in REGIONS:
        insns,edges,errors=walk(md,data,lo,hi)
        print(f"M345_REGION name={name} start={lo} end={hi} decoded={len(insns)} edges={len(edges)} undecodable={len(errors)} startAlignmentUnverified=1")
        # Focus on relevant instructions and a short surrounding context.
        ordered=sorted(insns)
        selected=set()
        for idx,pc in enumerate(ordered):
            ins=insns[pc]
            hit=ins.mnemonic.startswith("call")
            for op in ins.operands:
                if op.type==X86_OP_MEM and op.mem.disp in FIELDS:
                    hit=True
            if hit:
                selected.update(ordered[max(0,idx-3):min(len(ordered),idx+5)])
        for pc in ordered:
            if pc in selected:
                emit_ins(f"M345_INSN region={name}",insns[pc])
        for site,target,kind in edges:
            print(f"M345_EDGE region={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(lo<=target<hi))}")
        for pc in errors:
            print(f"M345_UNDECODABLE region={name} at={pc}")
        print(f"M345_REGION_END name={name}")
    for name,lo,hi in ANCHORS:
        print(f"M345_ANCHOR name={name} start={lo} end={hi}")
        for ins in md.disasm(data[lo:hi],lo):
            emit_ins(f"M345_ANCHOR_INSN name={name}",ins)
        print(f"M345_ANCHOR_END name={name}")
    for target in TARGETS:
        sites=[]
        for p in range(len(data)-4):
            if data[p]==0xe8 and p+5+int.from_bytes(data[p+1:p+5],"little",signed=True)==target:
                sites.append(p)
        local=[s for s in sites if 565000<=s<=582500]
        print(f"M345_RAW_CALLERS target={target} total={len(sites)} local={','.join(map(str,local)) or '-'}")
    print("M345_LIMITATIONS static=1 physicalOffsets=1 LEFixupsUnapplied=1 rawCallersMayBeData=1 regionStartsMayBeMisaligned=1 directCFGIncomplete=1 indirectControlFlowUnresolved=1 fieldAliasesUnverified=1 runtimeDataflowUnproven=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
