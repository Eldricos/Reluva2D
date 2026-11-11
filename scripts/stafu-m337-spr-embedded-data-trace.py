#!/usr/bin/env python3
"""STAFU M337: trace constructor input and embedded-data initializer.

Read-only, bounded x86 disassembly. Addresses are physical OVL offsets;
LE fixups are not applied. No runtime execution or codec claim.
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path

EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = (
    ("helper_564528", 564528, 565450),
    ("helper_563005", 563005, 563650),
    ("constructor_567212", 567212, 567497),
    ("allocation_587664", 587664, 588100),
    ("caller_580700", 580700, 580787),
)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=Path)
    ns = ap.parse_args()
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET
        from capstone.x86_const import X86_OP_IMM, X86_OP_MEM
    except ImportError:
        print("M337_ERROR missing_capstone=1 install='python -m pip install capstone'")
        return 2
    try:
        blob = ns.ovl.read_bytes()
    except OSError as e:
        print(f"M337_ERROR read={e}")
        return 2
    sha = hashlib.sha256(blob).hexdigest()
    ok = len(blob) == EXPECTED_SIZE and sha == EXPECTED_SHA
    print(f"M337_INPUT bytes={len(blob)} sha256={sha} knownMatch={int(ok)}")
    if not ok:
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    for name, start, end in REGIONS:
        todo = deque([start])
        visited = set()
        instructions = {}
        while todo and len(visited) < 5000:
            pc = todo.popleft()
            if pc in visited or not start <= pc < end:
                continue
            visited.add(pc)
            ins = next(md.disasm(blob[pc:min(end, pc + 15)], pc, count=1), None)
            if ins is None or pc + ins.size > end:
                continue
            instructions[pc] = ins
            jump = ins.group(CS_GRP_JUMP)
            ret = ins.group(CS_GRP_RET)
            if jump:
                t = ins.operands[0].imm if ins.operands and ins.operands[0].type == X86_OP_IMM else None
                if t is not None and start <= t < end:
                    todo.append(t)
                if ins.mnemonic != "jmp":
                    todo.append(pc + ins.size)
            elif not ret:
                todo.append(pc + ins.size)
        print(f"M337_REGION name={name} start={start} end={end} reached={len(instructions)} boundedCFG=1")
        insns = sorted(instructions.items())
        for i, (pc, ins) in enumerate(insns):
            mem = []
            for op in ins.operands:
                if op.type == X86_OP_MEM:
                    m = op.mem
                    base = ins.reg_name(m.base) if m.base else "none"
                    index = ins.reg_name(m.index) if m.index else "none"
                    mem.append(f"{base}:{index}:{m.disp:+#x}")
            is_call = ins.group(CS_GRP_CALL)
            target = ins.operands[0].imm if is_call and ins.operands and ins.operands[0].type == X86_OP_IMM else None
            print(f"M337_INSN region={name} at={pc} bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str} memory={';'.join(mem) or 'none'}")
            if is_call:
                print(f"M337_CALL region={name} at={pc} target={target if target is not None else 'indirect'}")
                for neighbor_pc, neighbor in insns[max(0,i-8):min(len(insns),i+6)]:
                    print(f"M337_CALL_CONTEXT region={name} focus={pc} at={neighbor_pc} instruction={neighbor.mnemonic} {neighbor.op_str}")
            if ins.group(CS_GRP_RET):
                print(f"M337_RETURN region={name} at={pc}")
        print(f"M337_REGION_END name={name}")
    print("M337_QUESTIONS helper_564528_input_consumption=unknown embedded_data_at_0x88_writes=unknown transform_vs_copy=unknown")
    print("M337_LIMITATION staticOnly=1 LEFixupsUnapplied=1 physicalOffsets=1 indirectCallsUnresolved=1 boundedCFGMayMissOutOfRangeBranches=1 noRuntimeTrace=1 decoderUnverified=1")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
