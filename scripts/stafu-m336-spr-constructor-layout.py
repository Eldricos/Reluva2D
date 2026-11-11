#!/usr/bin/env python3
"""M336: bounded constructor/cleanup object-layout and allocation trace.

Read-only static inspection. Offsets are physical OVL file offsets, not
runtime addresses. No original game code or resources are reproduced.
Requires: python -m pip install capstone
"""
import argparse
import hashlib
from collections import deque
from pathlib import Path

EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = (
    ("constructor_572028", 572028, 572411),
    ("constructor_567212", 567212, 568350),
    ("cleanup_565852", 565852, 566224),
    ("caller_580492", 580492, 580950),
)
def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("ovl", type=Path)
    args = p.parse_args()
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET
        from capstone.x86_const import X86_OP_IMM, X86_OP_MEM
    except ImportError:
        print("M336_ERROR missing_capstone=1 install='python -m pip install capstone'")
        return 2
    try:
        data = args.ovl.read_bytes()
    except OSError as e:
        print(f"M336_ERROR read={e}")
        return 2
    sha = hashlib.sha256(data).hexdigest()
    ok = len(data) == 2008455 and sha == EXPECTED_SHA
    print(f"M336_INPUT bytes={len(data)} sha256={sha} knownMatch={int(ok)}")
    if not ok:
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    for name, start, end in REGIONS:
        # Decode only addresses reached from the candidate entry by direct
        # branches/fallthrough. This avoids some linear-sweep data confusion.
        queue = deque([start])
        seen = set()
        decoded = {}
        edges = []
        while queue and len(seen) < 4000:
            at = queue.popleft()
            if at in seen or at < start or at >= end:
                continue
            seen.add(at)
            ins = next(md.disasm(data[at:min(at+15,end)], at, count=1), None)
            if ins is None or ins.address + ins.size > end:
                continue
            decoded[at] = ins
            is_call = ins.group(CS_GRP_CALL)
            is_jump = ins.group(CS_GRP_JUMP)
            is_ret = ins.group(CS_GRP_RET)
            target = (ins.operands[0].imm if ins.operands and ins.operands[0].type == X86_OP_IMM and (is_call or is_jump) else None)
            if is_call:
                edges.append(("call",at,target))
            if is_jump:
                edges.append(("jump",at,target))
                if target is not None and start <= target < end:
                    queue.append(target)
                if ins.mnemonic != "jmp":
                    queue.append(at+ins.size)
            elif not is_ret:
                queue.append(at+ins.size)
        print(f"M336_REGION name={name} start={start} end={end} reachableInstructions={len(decoded)} visited={len(seen)} entryProven=0")
        for at, ins in sorted(decoded.items()):
            is_call = ins.group(CS_GRP_CALL)
            is_jump = ins.group(CS_GRP_JUMP)
            is_ret = ins.group(CS_GRP_RET)
            mem = []
            for op in ins.operands:
                if op.type == X86_OP_MEM:
                    base = ins.reg_name(op.mem.base) if op.mem.base else "none"
                    index = ins.reg_name(op.mem.index) if op.mem.index else "none"
                    mem.append(f"{base}:{index}:{op.mem.disp:+#x}")
            # Full bounded listing, with memory operands, makes allocation,
            # field writes, pointer provenance and cleanup review reproducible.
            print(f"M336_INSN region={name} at={at} bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str} memory={';'.join(mem) or 'none'}")
            if is_call or is_ret:
                print(f"M336_EVENT region={name} kind={'call' if is_call else 'return'} at={at}")
        for kind, at, target in edges:
            if kind == "call":
                print(f"M336_CALL region={name} at={at} target={target if target is not None else 'indirect'}")
        print(f"M336_REGION_END name={name}")
    print("M336_HYPOTHESIS +0xa0 may be a managed object; compare constructors' returned pointers, object field stores and cleanup dereferences")
    print("M336_LIMITATION staticOnly=1 boundedCFG=1 branchTargetsPhysical=1 LEFixupsUnapplied=1 indirectCallsUnresolved=1 objectTypeUnverified=1 decoderUnverified=1")
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
