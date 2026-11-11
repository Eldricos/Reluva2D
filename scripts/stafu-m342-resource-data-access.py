#!/usr/bin/env python3
"""M342: static read-only data-access helper and caller survey.

All addresses are physical offsets into the verified STTNG.OVL. LE fixups
are not applied; indirect control flow is not resolved. Requires capstone.
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
    ("data_access_563123", 563123, 563650),
    ("caller_566260", 566260, 566552),
)

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def memrefs(i):
    for n, op in enumerate(i.operands):
        if op.type == X86_OP_MEM:
            yield n, i.reg_name(op.mem.base) if op.mem.base else "-", i.reg_name(op.mem.index) if op.mem.index else "-", op.mem.disp, op.size

def cfg(md, data, entry, lo, hi):
    queue = deque([entry])
    insns, edges, problems = {}, [], []
    while queue:
        pc = queue.popleft()
        while lo <= pc < hi and pc not in insns:
            i = next(md.disasm(data[pc:min(pc+15, hi)], pc, count=1), None)
            if i is None:
                problems.append(pc)
                break
            insns[pc] = i
            nxt = pc + i.size
            if i.mnemonic.startswith(("j", "loop")):
                target = i.operands[0].imm if i.operands and i.operands[0].type == X86_OP_IMM else None
                edges.append((pc, target, i.mnemonic))
                if target is not None and lo <= target < hi:
                    queue.append(target)
                if i.mnemonic == "jmp":
                    break
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int3", "ud2", "hlt"):
                break
            pc = nxt
    return insns, edges, problems

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M342_INPUT bytes={len(data)} sha256={digest}")
    if len(data) != EXPECTED_SIZE or digest != EXPECTED_SHA:
        raise SystemExit("M342_ERROR unexpected OVL size/hash")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    for name, lo, hi in REGIONS:
        insns, edges, problems = cfg(md, data, lo, lo, hi)
        print(f"M342_REGION name={name} start={lo} end={hi} decoded={len(insns)} edges={len(edges)} undecodable={len(problems)}")
        for pc in sorted(insns):
            i = insns[pc]
            refs = list(memrefs(i))
            print(f"M342_INSN region={name} at={fmt(i)}")
            if refs:
                detail = ";".join(f"op{n}:{base}:{index}:{disp:+#x}:size{size}" for n,base,index,disp,size in refs)
                print(f"M342_MEM region={name} site={pc} operands={detail}")
            if i.mnemonic.startswith("call"):
                target = i.operands[0].imm if i.operands and i.operands[0].type == X86_OP_IMM else None
                print(f"M342_CALL region={name} site={pc} target={target if target is not None else 'indirect'}")
        for site, target, kind in edges:
            print(f"M342_EDGE region={name} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not(lo <= target < hi))}")
        for pc in problems:
            print(f"M342_UNDECODABLE region={name} offset={pc}")
        print(f"M342_REGION_END name={name}")
    # Anchored disassembly immediately before the known call, from a known
    # reachable instruction stream rather than guessing an instruction boundary.
    caller, _, _ = cfg(md, data, 566260, 566260, 566552)
    pcs = sorted(caller)
    if 566397 in caller:
        idx = pcs.index(566397)
        print("M342_ARGUMENT_SETUP callsite=566397 target=563123")
        for pc in pcs[max(0, idx-25):min(len(pcs), idx+12)]:
            print(f"M342_CALL_CONTEXT at={fmt(caller[pc])}")
    else:
        print("M342_ARGUMENT_SETUP_NOT_REACHED callsite=566397")
    # Raw E8 call sites are leads only; show exact anchored post-call handling.
    sites = []
    for pc in range(len(data)-4):
        if data[pc] != 0xe8:
            continue
        delta = int.from_bytes(data[pc+1:pc+5], "little", signed=True)
        if pc + 5 + delta == 563123:
            sites.append(pc)
    print(f"M342_RAW_CALLERS target=563123 count={len(sites)} sites={','.join(map(str,sites)) or '-'}")
    for site in sites[:40]:
        print(f"M342_CALLER site={site}")
        for i in md.disasm(data[site:min(site+65, len(data))], site):
            print(f"M342_AFTER_CALL caller={site} at={fmt(i)}")
            if i.address >= site+48 or i.mnemonic.startswith("ret"):
                break
    print("M342_LIMITATIONS static=1 physicalOffsets=1 LEFixupsUnapplied=1 boundedCFG=1 indirectCallsUnresolved=1 rawCallSitesMayBeData=1 functionEndHeuristic=1 resourceDataAccessNotProven=1 decoderUnverified=1")

if __name__ == "__main__":
    main()
