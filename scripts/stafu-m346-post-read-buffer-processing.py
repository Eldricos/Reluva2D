#!/usr/bin/env python3
"""M346: post-read buffer processing survey (read-only, static).

Disassembles verified call-target entries and anchored caller instructions.
Physical STTNG.OVL offsets; LE fixups and indirect branches are unresolved.
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
FUNCTIONS=(
    ("post_read_645730",645730,646100),
    ("stream_close_563166",563166,563330),
    ("related_646108",646108,646450),
    ("related_646841",646841,647160),
)
# Known instruction boundaries in M345. Each window is decoded from its anchor.
ANCHORS=(
    ("read_destination",568440,568540),
    ("read_embedded",568284,568345),
    ("post_read_570032",570032,570064),
    ("embedded_object_init",567408,567450),
    ("external_object_init",572353,572395),
)
FIELDS={0x44,0x48,0x50,0x54,0x58,0x5c,0x60,0x64,0x70,0x74,0x78,0x88,0xa0}

def insfmt(ins):
    return f"{ins.address} {ins.mnemonic} {ins.op_str}".rstrip()

def walk(md,data,start,end,limit=1400):
    pending=deque([start])
    found={}
    edges=[]
    failures=[]
    while pending and len(found)<limit:
        pc=pending.popleft()
        while start<=pc<end and pc not in found and len(found)<limit:
            ins=next(md.disasm(data[pc:min(pc+15,end)],pc,count=1),None)
            if ins is None:
                failures.append(pc)
                break
            found[pc]=ins
            m=ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                t=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                edges.append((pc,t,m))
                if t is not None and start<=t<end:
                    pending.append(t)
                if m=="jmp":
                    break
            if m.startswith("ret") or m in ("int3","hlt","ud2"):
                break
            pc+=ins.size
    return found,edges,failures

def emit(prefix,ins):
    print(f"{prefix} {insfmt(ins)}")
    if ins.mnemonic.startswith("call"):
        t=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
        print(f"M346_CALL site={ins.address} target={t if t is not None else 'indirect'}")
    for n,op in enumerate(ins.operands):
        if op.type==X86_OP_MEM:
            base=ins.reg_name(op.mem.base) if op.mem.base else "-"
            index=ins.reg_name(op.mem.index) if op.mem.index else "-"
            print(f"M346_MEM site={ins.address} operand={n} base={base} index={index} disp={op.mem.disp:+#x} size={op.size} tracked={int(op.mem.disp in FIELDS)}")

def main():
    p=argparse.ArgumentParser()
    p.add_argument("ovl",type=Path)
    a=p.parse_args()
    data=a.ovl.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    print(f"M346_INPUT bytes={len(data)} sha256={digest}")
    if len(data)!=SIZE or digest!=SHA:
        raise SystemExit("M346_ERROR unexpected STTNG.OVL size/hash")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    for name,start,end in FUNCTIONS:
        insns,edges,failures=walk(md,data,start,end)
        print(f"M346_FUNCTION name={name} start={start} end={end} instructions={len(insns)} edges={len(edges)} failures={len(failures)} boundaryHeuristic=1")
        for pc in sorted(insns):
            emit(f"M346_INSN function={name}",insns[pc])
        for site,target,kind in edges:
            print(f"M346_EDGE function={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(start<=target<end))}")
        for site in failures:
            print(f"M346_FAILURE function={name} site={site}")
        print(f"M346_FUNCTION_END name={name}")
    for name,start,end in ANCHORS:
        print(f"M346_ANCHOR name={name} start={start} end={end}")
        for ins in md.disasm(data[start:end],start):
            emit(f"M346_ANCHOR_INSN name={name}",ins)
        print(f"M346_ANCHOR_END name={name}")
    for target in (645730,563166,646108,646841):
        sites=[]
        for site in range(len(data)-4):
            if data[site]==0xe8 and site+5+int.from_bytes(data[site+1:site+5],"little",signed=True)==target:
                sites.append(site)
        print(f"M346_RAW_CALLERS target={target} count={len(sites)} sites={','.join(map(str,sites)) or '-'}")
    print("M346_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 indirectControlFlowUnresolved=1 functionBoundsHeuristic=1 rawCallersMayBeData=1 pointerProvenanceUnverified=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
