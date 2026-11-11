#!/usr/bin/env python3
"""M341: read-only bounded CFG survey of resource-object consumers.

Physical file offsets, no LE fixups; static evidence only. Requires capstone.
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = [
    ("consumer_566260", 566260, 566552),
    ("consumer_566552", 566552, 566876),
    ("consumer_566876", 566876, 567212),
]
FIELDS = {0x44, 0x48, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x70, 0x74, 0x78, 0x88}

def describe(i):
    return f"{i.address} {i.mnemonic} {i.op_str}".rstrip()

def references(i):
    result = []
    for index, op in enumerate(i.operands):
        if op.type == X86_OP_MEM:
            base = i.reg_name(op.mem.base) if op.mem.base else "-"
            idx = i.reg_name(op.mem.index) if op.mem.index else "-"
            result.append((index, base, idx, op.mem.disp, op.size))
    return result

def decode_cfg(md, data, entry, lo, hi):
    pending = deque([entry])
    instructions = {}
    edges = []
    invalid = []
    while pending:
        pc = pending.popleft()
        while lo <= pc < hi and pc not in instructions:
            ins = next(md.disasm(data[pc:min(hi, pc + 15)], pc, count=1), None)
            if ins is None:
                invalid.append(pc)
                break
            instructions[pc] = ins
            nxt = pc + ins.size
            m = ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                target = ins.operands[0].imm if ins.operands and ins.operands[0].type == X86_OP_IMM else None
                edges.append((pc, target, m))
                if target is not None and lo <= target < hi:
                    pending.append(target)
                if m == "jmp":
                    break
            if m.startswith("ret") or m in ("int3", "ud2", "hlt"):
                break
            pc = nxt
    return instructions, edges, invalid

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    print(f"M341_INPUT bytes={len(data)} sha256={sha}")
    if len(data) != EXPECTED_SIZE or sha != EXPECTED_SHA:
        raise SystemExit("M341_ERROR unexpected OVL size/hash")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    all_targets = {}
    for name, lo, hi in REGIONS:
        insns, edges, invalid = decode_cfg(md, data, lo, lo, hi)
        print(f"M341_REGION name={name} entry={lo} end={hi} reachableInstructions={len(insns)} edges={len(edges)} undecodable={len(invalid)}")
        ordered = sorted(insns)
        for pc in ordered:
            ins = insns[pc]
            refs = references(ins)
            fieldrefs = [(operand, base, idx, disp, size) for operand, base, idx, disp, size in refs if base != "-" and disp in FIELDS]
            if fieldrefs:
                for operand, base, idx, disp, size in fieldrefs:
                    role = "destination" if operand == 0 else "source_or_other"
                    print(f"M341_FIELD region={name} at={pc} base={base} index={idx} disp={disp:#x} bytes={size} role={role} instruction={describe(ins)}")
            if ins.mnemonic.startswith("call"):
                target = ins.operands[0].imm if ins.operands and ins.operands[0].type == X86_OP_IMM else None
                all_targets.setdefault(target, []).append((name, pc))
                print(f"M341_CALL region={name} site={pc} target={target if target is not None else 'indirect'} instruction={describe(ins)}")
            if refs and (ins.mnemonic.startswith(("mov", "stos", "lods", "scas", "cmps", "rep", "lea"))):
                formatted = ";".join(f"op{op}:{base}:{idx}:{disp:+#x}:size{size}" for op, base, idx, disp, size in refs)
                print(f"M341_MEMORY region={name} at={pc} refs={formatted} instruction={describe(ins)}")
        focus = [j for j, pc in enumerate(ordered) if any(base != "-" and disp in (0x70, 0x74, 0x88) for _, base, idx, disp, size in references(insns[pc]))]
        for j in focus:
            pc = ordered[j]
            print(f"M341_FOCUS region={name} site={pc}")
            for k in range(max(0, j - 8), min(len(ordered), j + 12)):
                item = insns[ordered[k]]
                print(f"M341_CONTEXT region={name} focus={pc} delta={k-j:+d} at={describe(item)}")
        for src, dst, kind in edges:
            print(f"M341_EDGE region={name} source={src} target={dst if dst is not None else 'indirect'} kind={kind} outsideRegion={int(dst is not None and not(lo <= dst < hi))}")
        for pc in invalid:
            print(f"M341_UNDECODABLE region={name} offset={pc}")
        print(f"M341_REGION_END name={name}")
    for target, callers in sorted(all_targets.items(), key=lambda x: (-len(x[1]), -1 if x[0] is None else x[0])):
        print(f"M341_CALLEE target={target if target is not None else 'indirect'} count={len(callers)} sites={','.join(str(site) for _, site in callers)}")
    print("M341_LIMITATIONS staticOnly=1 boundedDirectBranchCFG=1 physicalOffsets=1 LEFixupsUnapplied=1 indirectCallsUnresolved=1 objectPointerProvenanceNotTracked=1 fieldOffsetMayBelongToDifferentStruct=1 bufferWritesUnverified=1 decoderUnverified=1")

if __name__ == "__main__":
    main()
