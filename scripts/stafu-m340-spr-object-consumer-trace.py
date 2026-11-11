#!/usr/bin/env python3
"""M340: read-only constructor caller and object-return consumer trace.

Uses physical OVL file offsets and bounded recursive-descent disassembly.
Results are candidates, not runtime call graph or proof of SPR decoding.
Requires capstone (pip install capstone).
"""
import argparse
import hashlib
from collections import defaultdict, deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

SIZE = 2008455
SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CONSTRUCTORS = (567212, 572028)
WINDOWS = (
    ("caller_spr_processing", 580420, 581200, 580420),
    ("second_processing", 581350, 581850, 581350),
    ("constructor_embedded", 567212, 567497, 567212),
    ("constructor_external", 572028, 572415, 572028),
)

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def mem(i):
    out = []
    for o in i.operands:
        if o.type == X86_OP_MEM:
            b = i.reg_name(o.mem.base) if o.mem.base else "-"
            x = i.reg_name(o.mem.index) if o.mem.index else "-"
            out.append(f"{b}:{x}:{o.mem.scale}:{o.mem.disp:+#x}")
    return ";".join(out) or "-"

def walk(md, data, start, lo, hi, budget=12000):
    """Bounded CFG candidate: direct branches only, no calls followed."""
    todo = deque([start])
    seen = set()
    edges = []
    while todo and len(seen) < budget:
        pc = todo.popleft()
        while lo <= pc < hi and pc not in seen and len(seen) < budget:
            ins = next(md.disasm(data[pc:min(pc + 15, hi)], pc, count=1), None)
            if not ins:
                break
            seen.add(pc)
            nxt = pc + ins.size
            m = ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                target = ins.operands[0].imm if ins.operands and ins.operands[0].type == X86_OP_IMM else None
                edges.append((pc, target, m))
                if target is not None and lo <= target < hi:
                    todo.append(target)
                if m == "jmp":
                    break
            if m.startswith("ret") or m in ("int3", "ud2", "hlt"):
                break
            pc = nxt
    return sorted(seen), edges

def main():
    p = argparse.ArgumentParser()
    p.add_argument("ovl", type=Path)
    a = p.parse_args()
    data = a.ovl.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    print(f"M340_INPUT bytes={len(data)} sha256={sha}")
    if len(data) != SIZE or sha != SHA:
        raise SystemExit("M340_ERROR unexpected OVL size/hash")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    # Raw byte signatures for direct relative calls. Not all occurrences are code.
    for target in CONSTRUCTORS:
        candidates = []
        for pos in range(0, len(data) - 4):
            if data[pos] != 0xe8:
                continue
            disp = int.from_bytes(data[pos + 1:pos + 5], "little", signed=True)
            if pos + 5 + disp == target:
                candidates.append(pos)
        print(f"M340_RAW_CALL_TARGET target={target} count={len(candidates)} sites={','.join(map(str,candidates)) or '-'}")
        for site in candidates[:80]:
            lo = max(0, site - 55)
            hi = min(len(data), site + 90)
            print(f"M340_CALLSITE site={site} target={target} linearContextMayBeMisaligned=1")
            # Anchor at call itself, then decode forward: avoids misleading preceding bytes.
            for ins in md.disasm(data[site:hi], site):
                if ins.address >= site + 75:
                    break
                print(f"M340_AFTER_CALL site={site} at={fmt(ins)} mem={mem(ins)}")
                if ins.mnemonic.startswith("ret"):
                    break
            # Show bytes before the call for later validated CFG reconstruction.
            print(f"M340_PRECALL_BYTES site={site} start={lo} hex={data[lo:site].hex()}")
    for name, lo, hi, entry in WINDOWS:
        pcs, edges = walk(md, data, entry, lo, hi)
        print(f"M340_CFG_REGION name={name} lo={lo} hi={hi} entry={entry} decoded={len(pcs)} branchEdges={len(edges)}")
        for pc in pcs:
            ins = next(md.disasm(data[pc:min(pc + 15, hi)], pc, count=1), None)
            if not ins:
                continue
            fields = [o.mem.disp for o in ins.operands if o.type == X86_OP_MEM and o.mem.base and o.mem.disp in (0x70, 0x74, 0x78, 0x88, 0xa0)]
            iscall = ins.mnemonic.startswith("call")
            if fields or iscall or ins.mnemonic.startswith("ret") or (name == "second_processing" and ins.mnemonic.startswith("j")):
                print(f"M340_CFG_INSN region={name} at={fmt(ins)} fields={','.join(hex(v) for v in fields) or '-'} mem={mem(ins)}")
        for source, target, kind in edges:
            print(f"M340_CFG_EDGE region={name} source={source} target={target if target is not None else 'indirect'} kind={kind}")
        print(f"M340_CFG_END name={name}")
    # Known call sites from prior M334/M335; print exact anchored forward evidence.
    for site in (580574, 580767):
        print(f"M340_KNOWN_RETURN_CONSUMER site={site}")
        for ins in md.disasm(data[site:min(len(data), site + 115)], site):
            print(f"M340_RETURN_TRACE site={site} at={fmt(ins)} mem={mem(ins)}")
            if ins.address > site + 85:
                break
    print("M340_LIMITATIONS physicalOffsets=1 LEFixupsUnapplied=1 rawE8MayBeData=1 directBranchCFGOnly=1 entryPointHeuristic=1 noIndirectCalls=1 objectIdentityNotProven=1 decoderNotVerified=1")

if __name__ == "__main__":
    main()
