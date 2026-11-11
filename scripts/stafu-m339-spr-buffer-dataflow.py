#!/usr/bin/env python3
"""M339: read-only SPR buffer field data-flow evidence survey.

Physical file offsets, static disassembly only; no LE fixups or runtime claims.
Requires: pip install capstone
"""
import argparse
import hashlib
import re
from collections import defaultdict
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_MEM, X86_OP_REG, X86_OP_IMM, X86_REG_INVALID

EXPECTED_SIZE = 2008455
EXPECTED_SHA256 = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = [
    ("record_field70", 577800, 578050),
    ("record_field70_second", 581420, 581810),
    ("constructor_embedded_buffer", 567330, 567495),
    ("constructor_external_buffer", 572220, 572410),
]
FIELDS = {0x44, 0x48, 0x50, 0x54, 0x60, 0x64, 0x68, 0x70, 0x74, 0x78, 0x88, 0xA0}

def fmt(ins):
    return f"{ins.address} {ins.mnemonic} {ins.op_str}".rstrip()

def regs(ins):
    return [ins.reg_name(op.reg) for op in ins.operands if op.type == X86_OP_REG]

def memrefs(ins):
    result = []
    for op in ins.operands:
        if op.type != X86_OP_MEM:
            continue
        mem = op.mem
        base = ins.reg_name(mem.base) if mem.base != X86_REG_INVALID else "-"
        index = ins.reg_name(mem.index) if mem.index != X86_REG_INVALID else "-"
        result.append((base, index, mem.disp, op.size))
    return result

def is_control(ins):
    return ins.mnemonic.startswith("j") or ins.mnemonic.startswith("call") or ins.mnemonic.startswith("ret") or ins.mnemonic in ("loop", "loope", "loopne")

def classify(ins):
    m = ins.mnemonic
    refs = memrefs(ins)
    if any(d in FIELDS and b != "-" for b, i, d, s in refs):
        return "FIELD"
    if refs and (m.startswith("mov") or m.startswith("stos") or m.startswith("rep") or m.startswith("cmp") or m.startswith("test")):
        return "MEMORY"
    if m.startswith(("push", "pop", "lea", "add", "sub", "shl", "shr", "sar", "imul", "and", "or", "xor")):
        return "FLOW"
    if is_control(ins):
        return "CONTROL"
    return "OTHER"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ovl", type=Path, help="Installed STTNG.OVL (read only)")
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M339_INPUT file={args.ovl} bytes={len(data)} sha256={digest}")
    if len(data) != EXPECTED_SIZE or digest != EXPECTED_SHA256:
        raise SystemExit("M339_ERROR original OVL size/hash mismatch")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    global_counts = defaultdict(int)
    for name, start, end in REGIONS:
        insns = list(md.disasm(data[start:end], start))
        print(f"M339_REGION name={name} start={start} end={end} linearInstructions={len(insns)}")
        field_hits = []
        for i, ins in enumerate(insns):
            kind = classify(ins)
            global_counts[kind] += 1
            if kind == "FIELD":
                field_hits.append(i)
            if kind in ("FIELD", "MEMORY", "CONTROL"):
                refs = ";".join(f"{b}+{d:#x}[{s}]" for b, ix, d, s in memrefs(ins)) or "-"
                print(f"M339_INSN region={name} kind={kind} at={fmt(ins)} mem={refs}")
        for i in field_hits:
            ins = insns[i]
            refs = memrefs(ins)
            targets = ",".join(f"{b}+{d:#x}" for b, ix, d, s in refs if d in FIELDS and b != "-")
            print(f"M339_FOCUS region={name} at={ins.address} fields={targets}")
            lo = max(0, i - 10)
            hi = min(len(insns), i + 15)
            for j in range(lo, hi):
                x = insns[j]
                print(f"M339_CONTEXT region={name} focus={ins.address} relation={j-i:+d} kind={classify(x)} instruction={fmt(x)}")
        print(f"M339_REGION_END name={name} fieldHits={len(field_hits)}")
    print("M339_COUNTS " + " ".join(f"{k}={v}" for k, v in sorted(global_counts.items())))
    print("M339_LIMITATION linearDisassemblyNotCFG=1 arbitraryWindowStarts=1 physicalOffsetsNotRuntimeAddresses=1 LEFixupsUnapplied=1 noRegisterSSA=1 pointerIdentityUnverified=1 writesNotProvenDecoder=1")

if __name__ == "__main__":
    main()
