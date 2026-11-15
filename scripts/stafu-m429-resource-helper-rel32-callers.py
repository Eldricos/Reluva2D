#!/usr/bin/env python3
"""STAFU M429: direct rel32 caller candidates for five resource helpers.

Read-only. Scan every E8 byte in Object 1 and calculate its signed rel32
target; verify possible instruction starts with local Capstone decoding.
Local alignment is heuristic, not a validated control-flow graph.
"""
import argparse, hashlib, struct
from pathlib import Path

LE = 0x38194
TARGETS = {
    "stream_position_wrapper": 0x897b3,
    "position_manager": 0x7e638,
    "resource_read_state": 0x8a3f4,
    "resource_lifecycle_a": 0x8a518,
    "resource_lifecycle_b": 0x8a65c,
}
def u32(data, pos):
    return struct.unpack_from("<I", data, pos)[0]

def analyze_alignment(md, blob, start, candidate, lo):
    """Find nearby decode starts that land exactly on candidate."""
    accepted = []
    for anchor in range(max(lo, candidate - 24), candidate + 1):
        seq = list(md.disasm(blob[anchor:min(len(blob), candidate + 5)], anchor))
        if not seq:
            continue
        for idx, ins in enumerate(seq):
            if ins.address == candidate and ins.mnemonic == "call" and ins.size == 5:
                if idx > 0 and seq[idx - 1].address + seq[idx - 1].size != candidate:
                    continue
                accepted.append((anchor, idx, seq))
                break
    return accepted

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    parser.add_argument("--context", type=int, default=48)
    args = parser.parse_args()
    b = args.ovl.read_bytes()
    print("M429_INPUT bytes=%d sha256=%s" % (len(b), hashlib.sha256(b).hexdigest()))
    if b[:2] != b"MZ" or b[LE:LE+2] != b"LE":
        raise SystemExit("M429_ERROR unexpected MZ/LE headers")
    tab = LE + u32(b, LE + 0x40)
    data_origin = LE + u32(b, LE + 0x80)
    page_size = u32(b, LE + 0x28)
    if page_size != 4096:
        raise SystemExit("M429_ERROR unexpected LE page size")
    size, base, flags, first_page, page_count = struct.unpack_from("<IIIII", b, tab)
    lo = data_origin + (first_page - 1) * page_size
    hi = min(len(b), lo + size)
    print("M429_OBJECT1 fileStart=0x%x fileEnd=0x%x base=0x%x assumedLinear=1" % (lo, hi, base))
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        raise SystemExit("M429_ERROR capstone required: python -m pip install capstone")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    matches = {name: [] for name in TARGETS}
    all_e8 = 0
    for pos in range(lo, hi - 4):
        if b[pos] != 0xE8:
            continue
        all_e8 += 1
        displacement = struct.unpack_from("<i", b, pos + 1)[0]
        dest = pos + 5 + displacement
        for name, target in TARGETS.items():
            if dest == target:
                matches[name].append((pos, displacement))
    print("M429_SCAN e8Bytes=%d object1Bytes=%d" % (all_e8, hi - lo))
    for name, target in TARGETS.items():
        print("M429_TARGET name=%s file=0x%x linear=0x%x" % (name, target, base + target - lo))
        hits = matches[name]
        print("M429_CANDIDATES name=%s count=%d" % (name, len(hits)))
        for pos, disp in hits:
            a = max(lo, pos - args.context)
            z = min(hi, pos + 5 + args.context)
            print("M429_HIT name=%s callFile=0x%x callLinear=0x%x rel32=%d contextStart=0x%x hex=%s" %
                  (name, pos, base + pos - lo, disp, a, b[a:z].hex()))
            aligned = analyze_alignment(md, b, lo, pos, lo)
            print("M429_LOCAL_ALIGNMENT name=%s callFile=0x%x possibleStarts=%d examples=%s" %
                  (name, pos, len(aligned), repr([hex(t[0]) for t in aligned[:12]])))
            if aligned:
                anchor, idx, seq = sorted(aligned, key=lambda x: (-x[1], x[0]))[0]
                print("M429_ALIGNMENT_EXAMPLE name=%s callFile=0x%x anchor=0x%x preceding=%d" %
                      (name, pos, anchor, idx))
                for ins in seq[max(0, idx-10):idx+1]:
                    print("M429_INSN file=0x%x bytes=%s mnemonic=%s operands=%s" %
                          (ins.address, ins.bytes.hex(), ins.mnemonic, ins.op_str))
            else:
                print("M429_ALIGNMENT_WARNING name=%s callFile=0x%x noLocalDecode=1" % (name, pos))
    print("M429_LIMITATIONS rawE8Candidates=1 localAlignmentNotCFG=1 indirectCallsNotScanned=1 fixupsNotParsed=1 noRuntimeTrace=1 noDecoderIdentified=1")

if __name__ == "__main__":
    main()
