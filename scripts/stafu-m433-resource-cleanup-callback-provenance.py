#!/usr/bin/env python3
"""STAFU M433: inspect resource cleanup helper and indirect callback provenance.

Static, read-only diagnostics for user-owned STTNG.OVL. File offsets below
are hypotheses from earlier experiments; do not interpret as runtime proof.
Requires: python -m pip install capstone
"""
import argparse
import hashlib
import struct
from collections import deque, defaultdict

START, END = 0x7AF94, 0x15382E
EXPECTED = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
HELPER, STATE, INDIRECT = 0x11BE64, 0x93569, 0x936D7
PROLOGUES = (bytes.fromhex(x) for x in ("5356575589e5", "56575589e5", "575589e5", "5589e5"))
PROLOGUES = tuple(PROLOGUES)

def decode(md, data, pc):
    if not START <= pc < END:
        return None
    ins = list(md.disasm(data[pc:min(pc+15, END)], pc, count=1))
    return ins[0] if ins and ins[0].address == pc else None

def walk(md, data, entry, max_instructions=3500):
    """Follow direct branches, never descend into called functions."""
    queue = deque([entry])
    visited, instructions, edges = set(), {}, []
    while queue and len(visited) < max_instructions:
        pc = queue.popleft()
        while START <= pc < END and pc not in visited and len(visited) < max_instructions:
            ins = decode(md, data, pc)
            if ins is None:
                break
            visited.add(pc)
            instructions[pc] = ins
            nxt = pc + ins.size
            m = ins.mnemonic
            if m.startswith("ret") or m in ("int", "int3", "ud2", "hlt", "iret"):
                break
            if m == "jmp":
                if ins.operands and ins.operands[0].type == 2:
                    queue.append(ins.operands[0].imm)
                    edges.append((pc, ins.operands[0].imm))
                break
            if m.startswith("j") or m.startswith("loop"):
                if ins.operands and ins.operands[0].type == 2:
                    queue.append(ins.operands[0].imm)
                    edges.append((pc, ins.operands[0].imm))
                pc = nxt
                continue
            pc = nxt
    return instructions, edges

def direct_calls(data, target):
    for pos in range(START, END-4):
        if data[pos] == 0xE8 and pos + 5 + struct.unpack_from("<i", data, pos+1)[0] == target:
            yield pos

def candidates(data, pos, span=0x800):
    lo = max(START, pos-span)
    return sorted((p for p in range(lo, pos) if any(data.startswith(q, p) for q in PROLOGUES)), reverse=True)

def print_ins(prefix, ins):
    print(f"{prefix} file={ins.address:#x} bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")

def trace_caller(md, data, call, target):
    """Find heuristic entry that reaches the specific direct-call opcode."""
    for entry in candidates(data, call)[:24]:
        insns, _ = walk(md, data, entry, 4500)
        if call in insns and insns[call].mnemonic == "call":
            return entry, insns
    return None, {}

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("ovl")
    args = p.parse_args()
    with open(args.ovl, "rb") as f:
        data = f.read()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M433_INPUT bytes={len(data)} sha256={digest} expectedMatch={digest==EXPECTED}")
    if len(data) < END or data[:2] != b"MZ":
        raise SystemExit("Unexpected OVL: MZ missing or Object1 truncated")
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        raise SystemExit("Capstone required: python -m pip install capstone")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    print("M433_CAUTION file offsets and function starts are hypotheses; direct opcode hits may be embedded data.")
    for label, entry in (("cleanup_candidate", HELPER), ("state_transition", STATE)):
        graph, edges = walk(md, data, entry)
        print(f"M433_FUNCTION name={label} entry={entry:#x} instructions={len(graph)} directEdges={len(edges)} lastDecoded={max(graph) if graph else -1:#x}")
        # Bound reporting to nearby function body: indirect jumps and suspicious distant branches remain visible.
        selected = [ins for pc, ins in sorted(graph.items()) if pc < entry+0x380]
        print(f"M433_FUNCTION_WINDOW name={label} instructions={len(selected)}")
        for ins in selected:
            print_ins("M433_INSN", ins)
        for ins in selected:
            if ins.mnemonic.startswith("ret"):
                print_ins("M433_RETURN", ins)
            elif ins.mnemonic == "call":
                print_ins("M433_CALLEE", ins)
            elif ins.mnemonic.startswith("mov") and "[" in ins.op_str:
                print_ins("M433_MEMORY", ins)
        if label == "state_transition":
            site = graph.get(INDIRECT)
            print(f"M433_INDIRECT_SITE file={INDIRECT:#x} aligned={site is not None} mnemonic={site.mnemonic if site else 'NONE'} operands={site.op_str if site else 'NONE'}")
            if site is not None:
                local = [graph[x] for x in sorted(graph) if INDIRECT-112 <= x <= INDIRECT+32]
                for ins in local:
                    print_ins("M433_INDIRECT_CONTEXT", ins)
    for label, target in (("cleanup_candidate", HELPER), ("state_transition", STATE)):
        hits = list(direct_calls(data, target))
        print(f"M433_CALLERS name={label} target={target:#x} rawCount={len(hits)}")
        for call in hits:
            entry, graph = trace_caller(md, data, call, target)
            print(f"M433_CALLER name={label} call={call:#x} entry={hex(entry) if entry is not None else 'UNVERIFIED'}")
            if entry is None:
                print(f"M433_CALLER_RAW call={call:#x} hex={data[max(START,call-24):call+12].hex()}")
                continue
            context = [graph[x] for x in sorted(graph) if call-96 <= x <= call+32]
            for ins in context:
                print_ins("M433_CALLER_CONTEXT", ins)
            # Stack argument at [ebp+0x18] is supplied by caller's pushes,
            # but register/stack state cannot be established from a local window.
            print("M433_CALLER_NOTE local push/call sequence only; no interprocedural dataflow proof")
    print("M433_DONE readOnly=1")

if __name__ == "__main__":
    main()
