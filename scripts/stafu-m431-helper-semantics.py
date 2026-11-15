#!/usr/bin/env python3
"""STAFU M431: read-only examination of candidate helper and two caller paths.

Requires: pip install capstone
Usage: python scripts/stafu-m431-helper-semantics.py games/STTNG/STTNG.OVL > m431-helper-semantics.txt
File offsets refer to the original OVL, not runtime addresses. No game data is modified.
"""
import hashlib
import struct
import sys

EXPECTED = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
START, END = 0x7AF94, 0x15382E
HELPER = 0x8A3F4
CALLERS = [(0xC7658, 0xC767B, "conditional-path"), (0x8701D, 0x870B6, "object-state")]
OTHER = [0x8A65C, 0x8B548, 0x8B9E4, 0x83386]

def decode(md, data, start, limit, label):
    print(f"M431_SECTION name={label} start={start:#x} end={limit:#x}")
    if not (START <= start < limit <= END):
        print("M431_INVALID_RANGE")
        return []
    instructions = list(md.disasm(data[start:limit], start))
    for i in instructions:
        print(f"M431_INSN file={i.address:#x} bytes={i.bytes.hex()} mnemonic={i.mnemonic} operands={i.op_str}")
    if instructions:
        last = instructions[-1]
        print(f"M431_DECODE_STOP file={last.address + last.size:#x} requestedEnd={limit:#x}")
    return instructions

def summarize(ins, label):
    print(f"M431_SUMMARY name={label} decoded={len(ins)}")
    for i in ins:
        if i.mnemonic.startswith("call"):
            print(f"M431_CALL name={label} site={i.address:#x} target={i.op_str}")
        if i.mnemonic.startswith("ret"):
            print(f"M431_RETURN name={label} site={i.address:#x} operand={i.op_str}")
        if i.mnemonic in ("push", "pop") and ("ebp" in i.op_str or "esp" in i.op_str):
            print(f"M431_FRAME name={label} site={i.address:#x} {i.mnemonic} {i.op_str}")
    print("M431_CAUTION linear disassembly does not prove instruction reachability; branches may cross window boundaries")

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        print("M431_ERROR capstone missing; install with: python -m pip install capstone")
        return 2
    with open(sys.argv[1], "rb") as f:
        data = f.read()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M431_INPUT bytes={len(data)} sha256={digest} hashMatch={digest == EXPECTED}")
    if digest != EXPECTED:
        print("M431_ERROR input differs from established OVL; refusing offset-based analysis")
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    print(f"M431_OBJECT1 fileStart={START:#x} fileEnd={END:#x} addressMode=file-offset")
    helper = decode(md, data, HELPER, HELPER + 0x240, "helper-8a3f4")
    summarize(helper, "helper-8a3f4")
    for entry, call, name in CALLERS:
        ins = decode(md, data, entry, min(call + 0xB0, END), "caller-" + name)
        summarize(ins, "caller-" + name)
        hits = [i for i in ins if i.address == call]
        print(f"M431_CALLSITE name={name} site={call:#x} boundaryVerified={bool(hits)}")
        if hits and hits[0].mnemonic == "call" and hits[0].size == 5:
            displacement = struct.unpack_from("<i", data, call + 1)[0]
            target = call + 5 + displacement
            print(f"M431_CALLSITE_TARGET name={name} target={target:#x} expected={HELPER:#x} match={target == HELPER}")
    for target in OTHER:
        ins = decode(md, data, target, min(target + 0x90, END), f"related-{target:x}")
        summarize(ins, f"related-{target:x}")
    print("M431_CAUTION Function labels are hypotheses, not established semantics. Object mapping and control flow are not runtime validated.")
    print("M431_DONE readOnly=1")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
