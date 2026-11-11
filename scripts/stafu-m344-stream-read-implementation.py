#!/usr/bin/env python3
"""M344: read-only underlying stream-read implementation survey.

Requires capstone. Uses physical STTNG.OVL offsets, without LE fixups.
Direct-branch CFG is bounded and does not resolve indirect control flow.
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = (
    ("stream_read_517764", 517764, 518400),
    ("stream_position_517688", 517688, 517764),
    ("read_wrapper_563062", 563062, 563123),
)
CALLERS = (563092, 519100, 519166, 570032)

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def cfg(md, data, entry, lo, hi, limit=2000):
    pending=deque([entry])
    found={}
    edges=[]
    bad=[]
    while pending and len(found)<limit:
        pc=pending.popleft()
        while lo<=pc<hi and pc not in found and len(found)<limit:
            ins=next(md.disasm(data[pc:min(pc+15,hi)],pc,count=1),None)
            if ins is None:
                bad.append(pc)
                break
            found[pc]=ins
            nxt=pc+ins.size
            m=ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                t=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                edges.append((pc,t,m))
                if t is not None and lo<=t<hi:
                    pending.append(t)
                if m=="jmp":
                    break
            if m.startswith("ret") or m in ("int3","ud2","hlt"):
                break
            pc=nxt
    return found,edges,bad

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    data=args.ovl.read_bytes()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M344_INPUT bytes={len(data)} sha256={sha}")
    if len(data)!=EXPECTED_SIZE or sha!=EXPECTED_SHA:
        raise SystemExit("M344_ERROR unexpected STTNG.OVL size/hash")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    allcalls={}
    for name,lo,hi in REGIONS:
        instructions,edges,bad=cfg(md,data,lo,lo,hi)
        print(f"M344_REGION name={name} entry={lo} boundary={hi} decoded={len(instructions)} edges={len(edges)} undecodable={len(bad)}")
        for pc in sorted(instructions):
            ins=instructions[pc]
            print(f"M344_INSN region={name} {fmt(ins)}")
            for n,op in enumerate(ins.operands):
                if op.type==X86_OP_MEM:
                    b=ins.reg_name(op.mem.base) if op.mem.base else "-"
                    x=ins.reg_name(op.mem.index) if op.mem.index else "-"
                    print(f"M344_MEM region={name} site={pc} operand={n} base={b} index={x} disp={op.mem.disp:+#x} size={op.size}")
            if ins.mnemonic.startswith("call"):
                t=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                print(f"M344_CALL region={name} site={pc} target={t if t is not None else 'indirect'}")
                if t is not None:
                    allcalls.setdefault(t,[]).append(pc)
        for site,target,kind in edges:
            print(f"M344_EDGE region={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(lo<=target<hi))}")
        for pc in bad:
            print(f"M344_UNDECODABLE region={name} offset={pc}")
        print(f"M344_REGION_END name={name}")
    for t,sites in sorted(allcalls.items()):
        print(f"M344_CALLEE target={t} sites={','.join(map(str,sites))}")
    for site in CALLERS:
        lo=max(0,site-48)
        hi=min(len(data),site+48)
        print(f"M344_CALLSITE site={site} note=precall-window-may-start-mid-instruction")
        # Anchor post-call decoding at known instruction boundary.
        for ins in md.disasm(data[site:hi],site):
            print(f"M344_AFTER_CALL site={site} {fmt(ins)}")
            if ins.address>=site+30 or ins.mnemonic.startswith("ret"):
                break
    for target in (517764,):
        sites=[]
        for p in range(len(data)-4):
            if data[p]==0xe8 and p+5+int.from_bytes(data[p+1:p+5],"little",signed=True)==target:
                sites.append(p)
        print(f"M344_RAW_CALLERS target={target} count={len(sites)} sites={','.join(map(str,sites)) or '-'}")
    print("M344_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 boundedDirectBranchCFG=1 indirectBranchesUnresolved=1 functionBoundaryHeuristic=1 objectProvenanceNotTracked=1 streamSemanticsUnverified=1 decoderUnverified=1")

if __name__=="__main__":
    main()
