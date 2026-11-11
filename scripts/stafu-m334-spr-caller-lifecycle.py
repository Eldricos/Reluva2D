#!/usr/bin/env python3
"""M334: read-only static survey of the candidate SPR processing caller.

Addresses are raw STTNG.OVL physical offsets. No LE relocation or CFG proof.
Requires: pip install capstone
"""
import argparse
import hashlib
from collections import defaultdict
from pathlib import Path

EXPECTED_SIZE = 2008455
EXPECTED_SHA256 = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = [
    ("caller_580470", 580470, 581090),
    ("producer_572028", 572028, 572440),
    ("initializer_567212", 567212, 567510),
    ("processor_510232", 510232, 510790),
    ("handle_566552", 566552, 566940),
]
TARGETS = {572028, 567212, 510232, 566552, 565852, 566260, 566876}
FOCI = [580574, 580767, 580797, 580999]
def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET
        from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_ESP, X86_REG_EBP
    except ImportError as e:
        ap.error("Install Capstone: python -m pip install capstone")
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    good = len(data) == EXPECTED_SIZE and digest == EXPECTED_SHA256
    print(f"M334_INPUT bytes={len(data)} sha256={digest} knownMatch={int(good)}")
    if not good:
        raise SystemExit("M334_ERROR unexpected STTNG.OVL version; refusing address-based analysis")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    decoded = {}
    for name, start, end in REGIONS:
        instructions = list(md.disasm(data[start:end], start))
        print(f"M334_REGION name={name} start={start} end={end} instructions={len(instructions)} entryVerified=0")
        decoded[name] = instructions
        for i, ins in enumerate(instructions):
            mem = []
            for op in ins.operands:
                if op.type == X86_OP_MEM:
                    m = op.mem
                    mem.append(f"base={ins.reg_name(m.base) or 'none'},index={ins.reg_name(m.index) or 'none'},disp={m.disp:+#x}")
            is_call = ins.group(CS_GRP_CALL)
            is_branch = ins.group(CS_GRP_JUMP)
            is_ret = ins.group(CS_GRP_RET)
            target = None
            if (is_call or is_branch) and ins.operands and ins.operands[0].type == X86_OP_IMM:
                target = ins.operands[0].imm
            important = is_call or is_ret or (is_branch and name == "caller_580470") or bool(mem)
            if important:
                print(f"M334_INSN region={name} at={ins.address} bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str} target={target if target is not None else 'none'} memory={'|'.join(mem) if mem else 'none'}")
            if is_call:
                before = instructions[max(0, i-9):i]
                after = instructions[i+1:i+9]
                print(f"M334_CALL region={name} at={ins.address} target={target if target is not None else 'indirect'}")
                for label, seq in (("BEFORE",before),("AFTER",after)):
                    for ctx in seq:
                        print(f"M334_CONTEXT region={name} focus={ins.address} position={label} at={ctx.address} bytes={ctx.bytes.hex()} instruction={ctx.mnemonic} {ctx.op_str}")
    caller = decoded["caller_580470"]
    starts = {ins.address:i for i,ins in enumerate(caller)}
    for focus in FOCI:
        print(f"M334_FOCUS at={focus} instructionBoundary={int(focus in starts)}")
        if focus in starts:
            i=starts[focus]
            for ins in caller[max(0,i-14):min(len(caller),i+15)]:
                print(f"M334_FOCUS_CONTEXT focus={focus} at={ins.address} bytes={ins.bytes.hex()} instruction={ins.mnemonic} {ins.op_str}")
    for target in sorted(TARGETS):
        sites=[]
        for pos in range(0,len(data)-4):
            if data[pos]!=0xE8: continue
            displacement=int.from_bytes(data[pos+1:pos+5],"little",signed=True)
            if pos+5+displacement==target: sites.append(pos)
        print(f"M334_RAW_XREF target={target} count={len(sites)} sites={','.join(map(str,sites))} boundariesUnverified=1")
    print("M334_LIMITATION linearSweepNotCFG=1 runtimeCallsUnverified=1 physicalOffsetsOnly=1 LEFixupsUnapplied=1 pointerIdentityUnverified=1 SPRInputUnverified=1 decoderUnverified=1")
if __name__ == "__main__":
    main()
