#!/usr/bin/env python3
"""M343: read-only resource I/O wrapper identification.

Physical offsets into verified STTNG.OVL; no LE fixups or runtime execution.
Requires capstone. Bounded direct-branch CFG; indirect edges unresolved.
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
    ("delegate_517688", 517688, 518400),
    ("candidate_read_563062", 563062, 563123),
    ("post_seek_570696", 570696, 571200),
    ("seek_wrapper_563123", 563123, 563166),
)
ANCHORS = ((519076, 519115), (519142, 519180), (566382, 566415))

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def refs(i):
    for j, op in enumerate(i.operands):
        if op.type == X86_OP_MEM:
            yield (j, i.reg_name(op.mem.base) if op.mem.base else "-", i.reg_name(op.mem.index) if op.mem.index else "-", op.mem.disp, op.size)

def cfg(md, data, entry, lo, hi, max_insns=1400):
    pending = deque([entry])
    found, edges, failures = {}, [], []
    while pending and len(found) < max_insns:
        pc = pending.popleft()
        while lo <= pc < hi and pc not in found and len(found) < max_insns:
            i = next(md.disasm(data[pc:min(pc+15,hi)], pc, count=1), None)
            if i is None:
                failures.append(pc)
                break
            found[pc] = i
            nxt = pc + i.size
            m = i.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                target = i.operands[0].imm if i.operands and i.operands[0].type == X86_OP_IMM else None
                edges.append((pc,target,m))
                if target is not None and lo <= target < hi:
                    pending.append(target)
                if m == "jmp":
                    break
            if m.startswith("ret") or m in ("int3","ud2","hlt"):
                break
            pc = nxt
    return found, edges, failures

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    print(f"M343_INPUT bytes={len(data)} sha256={sha}")
    if len(data) != EXPECTED_SIZE or sha != EXPECTED_SHA:
        raise SystemExit("M343_ERROR unexpected STTNG.OVL size/hash")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    for name, start, end in REGIONS:
        insns, edges, errors = cfg(md,data,start,start,end)
        print(f"M343_REGION name={name} entry={start} boundary={end} decoded={len(insns)} edges={len(edges)} undecodable={len(errors)}")
        for pc in sorted(insns):
            i = insns[pc]
            print(f"M343_INSN region={name} {fmt(i)}")
            mem = list(refs(i))
            if mem:
                print(f"M343_MEM region={name} site={pc} operands={';'.join(f'op{j}:{b}:{x}:{d:+#x}:size{size}' for j,b,x,d,size in mem)}")
            if i.mnemonic.startswith("call"):
                t = i.operands[0].imm if i.operands and i.operands[0].type == X86_OP_IMM else None
                print(f"M343_CALL region={name} site={pc} target={t if t is not None else 'indirect'}")
        for site,target,kind in edges:
            print(f"M343_EDGE region={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(start <= target < end))}")
        for pc in errors:
            print(f"M343_UNDECODABLE region={name} offset={pc}")
        print(f"M343_REGION_END name={name}")
    for lo, hi in ANCHORS:
        print(f"M343_ANCHOR start={lo} end={hi} note=known-instruction-boundary")
        for i in md.disasm(data[lo:hi],lo):
            print(f"M343_ANCHOR_INSN {fmt(i)}")
    for target in (517688,563062,570696):
        sites=[]
        for p in range(len(data)-4):
            if data[p] == 0xe8 and p+5+int.from_bytes(data[p+1:p+5],"little",signed=True) == target:
                sites.append(p)
        print(f"M343_RAW_CALLERS target={target} count={len(sites)} sites={','.join(map(str,sites)) or '-'}")
        for site in sites[:25]:
            print(f"M343_CALLER_CONTEXT target={target} site={site}")
            for i in md.disasm(data[site:min(site+45,len(data))],site):
                print(f"M343_AFTER_CALL at={fmt(i)}")
                if i.address >= site+30 or i.mnemonic.startswith("ret"):
                    break
    print("M343_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 functionBoundariesHeuristic=1 indirectBranchesUnresolved=1 rawCallersMayBeData=1 readSeekClassificationUnverified=1 decoderUnverified=1")

if __name__ == "__main__":
    main()
