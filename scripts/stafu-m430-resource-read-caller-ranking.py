#!/usr/bin/env python3
"""M430: rank direct callers of the candidate resource-read helper.

Read-only, static analysis of a user-supplied STTNG.OVL. Does not extract assets.
Capstone is required for instruction-boundary/CFG validation:
    python -m pip install capstone
"""
import argparse
import hashlib
import struct
from collections import deque

TARGET = 0x8A3F4
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
START = 0x7AF94
END = 0x15382E
PROLOGUES = (bytes.fromhex("5356575589e5"), bytes.fromhex("56575589e5"),
             bytes.fromhex("575589e5"), bytes.fromhex("5589e5"))

def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]

def locate_candidates(data):
    hits = []
    for off in range(START, END - 4):
        if data[off] == 0xE8 and off + 5 + struct.unpack_from("<i", data, off + 1)[0] == TARGET:
            hits.append(off)
    return hits

def find_starts(data, call):
    lo = max(START, call - 0x1000)
    found = []
    for pos in range(lo, call):
        for pattern in PROLOGUES:
            if data.startswith(pattern, pos):
                found.append((pos, len(pattern)))
                break
    return sorted(found, reverse=True)

def trace(data, md, entry, goal, budget=5000):
    """Follow decoded direct branches from an assumed function entry.

    Conservative: indirect transfers terminate a path; direct calls are not
    traversed. A reachable call proves alignment only conditional on entry.
    """
    pending = deque([entry])
    seen = set()
    decoded = {}
    while pending and len(seen) < budget:
        pc = pending.popleft()
        while START <= pc < END and pc not in seen and len(seen) < budget:
            insns = list(md.disasm(data[pc:min(pc + 15, END)], pc, count=1))
            if not insns or insns[0].address != pc:
                break
            ins = insns[0]
            seen.add(pc)
            decoded[pc] = ins
            if pc == goal:
                return True, decoded
            m = ins.mnemonic
            nxt = pc + ins.size
            if m.startswith("ret") or m in ("int", "int3", "ud2", "hlt", "iret"):
                break
            if m == "jmp":
                if ins.operands and ins.operands[0].type == 2:
                    pending.append(ins.operands[0].imm)
                break
            if m.startswith("j") or m.startswith("loop"):
                if ins.operands and ins.operands[0].type == 2:
                    pending.append(ins.operands[0].imm)
                pc = nxt
                continue
            pc = nxt
    return False, decoded

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", help="path to original STTNG.OVL")
    args = parser.parse_args()
    with open(args.ovl, "rb") as handle:
        data = handle.read()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M430_INPUT bytes={len(data)} sha256={digest}")
    print(f"M430_HASH_MATCH expected={EXPECTED_SHA} match={digest == EXPECTED_SHA}")
    print(f"M430_OBJECT1 fileStart={START:#x} fileEnd={END:#x} assumedLinear=1")
    if len(data) < END or data[:2] != b"MZ":
        raise SystemExit("Unexpected input: missing MZ or truncated Object 1")
    hits = locate_candidates(data)
    print(f"M430_RAW_CALL_CANDIDATES target={TARGET:#x} count={len(hits)}")
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_OP_IMM
    except ImportError:
        print("M430_CAPSTONE unavailable: no CFG validation or ranking; install with python -m pip install capstone")
        for hit in hits:
            print(f"M430_UNVERIFIED_CALL file={hit:#x}")
        return
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    results = []
    for hit in hits:
        starts = find_starts(data, hit)
        verified = None
        graph = None
        # Try nearest candidate entries first; do not assume any is genuine.
        for start, plen in starts[:24]:
            reachable, decoded = trace(data, md, start, hit)
            if reachable:
                verified = start
                graph = decoded
                break
        before = []
        after = []
        if verified is not None:
            ordered = sorted(k for k in graph if hit - 64 <= k < hit)
            before = [graph[k] for k in ordered[-14:]]
            # Disassemble sequentially immediately following the call.
            after = list(md.disasm(data[hit + 5:hit + 53], hit + 5, count=12))
        score = 0
        features = []
        if verified is not None:
            score += 20
            features.append("reachable_from_prologue")
            recent = before[-10:]
            pushes = sum(i.mnemonic == "push" for i in recent)
            score += min(pushes, 4) * 2
            if pushes:
                features.append(f"pushes={pushes}")
            for ins in recent + after:
                s = (ins.mnemonic + " " + ins.op_str).lower()
                if any(x in s for x in ("cmp ", "test ")):
                    score += 2
                    features.append("check")
                    break
            for ins in after:
                if ins.mnemonic == "call":
                    score += 2
                    features.append("subsequent_call")
                    break
            for ins in recent:
                if ins.mnemonic == "call":
                    score += 2
                    features.append("preceding_call")
                    break
        results.append((score, hit, verified, features, before, after, len(starts)))
    verified_count = sum(entry is not None for _, _, entry, *_ in results)
    print(f"M430_CFG_REACHABLE count={verified_count} of={len(hits)}")
    print("M430_CAUTION CFG reachability is conditional on heuristic prologue identification; not runtime proof.")
    print("M430_CAUTION Scores prioritize inspection, not proof of SPR decoding or data flow.")
    for score, hit, entry, features, before, after, start_count in sorted(results, key=lambda x: (-x[0], x[1])):
        print(f"M430_RANK callFile={hit:#x} score={score} entry={hex(entry) if entry is not None else 'UNVERIFIED'} prologueCandidates={start_count} features={','.join(features) or 'none'}")
        if entry is None:
            print(f"M430_RAW_CONTEXT callFile={hit:#x} hex={data[max(START,hit-24):min(END,hit+20)].hex()}")
            continue
        for ins in before:
            print(f"M430_BEFORE file={ins.address:#x} bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")
        call = next(md.disasm(data[hit:hit+5], hit, count=1))
        print(f"M430_CALL file={hit:#x} mnemonic={call.mnemonic} operands={call.op_str}")
        for ins in after:
            print(f"M430_AFTER file={ins.address:#x} bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")
    print("M430_DONE readOnly=1")

if __name__ == "__main__":
    main()
