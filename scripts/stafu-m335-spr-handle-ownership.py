#!/usr/bin/env python3
"""M335: inspect candidate SPR handle construction, ownership, and release.

Read-only static survey of a known STTNG.OVL. Addresses are physical file
offsets, NOT resolved LE runtime addresses. Capstone required.
"""
import argparse
import hashlib
from pathlib import Path

SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
SIZE = 2008455
REGIONS = [
    ("caller_handle_lifecycle", 580492, 581095),
    ("constructor_572028", 572028, 572550),
    ("alternate_constructor_567212", 567212, 567800),
    ("release_565852", 565852, 566150),
    ("release_566224", 566224, 566552),
    ("use_566260", 566260, 566552),
    ("use_566552", 566552, 566875),
    ("use_566876", 566876, 567212),
]
TARGETS = {572028, 567212, 565852, 566224, 566260, 566552, 566876}
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP
        from capstone.x86_const import X86_OP_IMM, X86_OP_MEM
    except ImportError:
        print("M335_ERROR install Capstone: python -m pip install capstone")
        return 2
    try:
        data = args.ovl.read_bytes()
    except OSError as exc:
        print(f"M335_ERROR read={exc}")
        return 2
    sha = hashlib.sha256(data).hexdigest()
    valid = len(data) == SIZE and sha == SHA
    print(f"M335_INPUT bytes={len(data)} sha256={sha} knownMatch={int(valid)}")
    if not valid:
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    def target(ins):
        if ins.operands and ins.operands[0].type == X86_OP_IMM:
            return ins.operands[0].imm
        return None
    for name, start, end in REGIONS:
        insns = list(md.disasm(data[start:end], start))
        print(f"M335_REGION name={name} start={start} end={end} instructions={len(insns)} verifiedEntry=0")
        for i, ins in enumerate(insns):
            t = target(ins) if (ins.group(CS_GRP_CALL) or ins.group(CS_GRP_JUMP)) else None
            mem = []
            for op in ins.operands:
                if op.type == X86_OP_MEM:
                    m = op.mem
                    mem.append(f"{ins.reg_name(m.base) if m.base else 'none'}:{ins.reg_name(m.index) if m.index else 'none'}:{m.disp:+#x}")
            print(f"M335_INSN region={name} at={ins.address} bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str} target={t if t is not None else 'none'} memory={','.join(mem) if mem else 'none'}")
            focus = ins.group(CS_GRP_CALL) or ins.mnemonic.startswith("ret") or any(op.type == X86_OP_MEM and op.mem.disp in (0xa0, 0x48, 0x54, 0x58, 0x70, 0x74) for op in ins.operands)
            if focus:
                print(f"M335_FOCUS region={name} at={ins.address} kind={'call' if ins.group(CS_GRP_CALL) else 'field_or_return'}")
                for neighbor in insns[max(0, i-5):min(len(insns), i+6)]:
                    print(f"M335_CONTEXT focus={ins.address} at={neighbor.address} instruction={neighbor.mnemonic} {neighbor.op_str}")
        print(f"M335_REGION_END name={name}")
    # Raw E8 reference inventory: possible calls, not proof of instruction alignment.
    for destination in sorted(TARGETS):
        matches = []
        for pos in range(0, len(data)-4):
            if data[pos] != 0xe8:
                continue
            displacement = int.from_bytes(data[pos+1:pos+5], "little", signed=True)
            if pos + 5 + displacement == destination:
                matches.append(pos)
        print(f"M335_RAW_CALLERS target={destination} count={len(matches)} offsets={','.join(map(str,matches[:100]))} truncated={int(len(matches)>100)}")
    print("M335_LIMITATION staticOnly=1 linearSweepNotCFG=1 arbitraryRegionStarts=1 LEFixupsUnapplied=1 indirectCallsUnresolved=1 handleSemanticsUnverified=1 decoderUnverified=1")
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
