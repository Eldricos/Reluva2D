#!/usr/bin/env python3
"""STAFU M401: bounded segment setup and table-consumer trace (read-only).

This is a pattern-based 16-bit x86 investigation, not a full disassembler.
All file-relative call targets and inferred DS values are hypotheses.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LE_FILE = 0x38194
AFFINE_BASE = 0xA4F48C
REGIONS = (
    ("process_manager", 0x24BF0, 0x24CA0, 0x31D64, 5),
    ("loader_names", 0x2D660, 0x2D710, 0x338FE, 4),
)
def u16(data, at):
    return struct.unpack_from("<H", data, at)[0]
def s16(value):
    return value - 65536 if value >= 32768 else value
def segment_setup(data, start, stop):
    """Recognize exact mov ax,imm16; mov ds,ax, including optional nearby push ds."""
    for at in range(start, stop - 4):
        if data[at] == 0xB8 and data[at + 3:at + 5] == b"\x8e\xd8":
            yield at, u16(data, at + 1), data[max(start,at-2):at+7].hex()
def indexed_pairs(data, start, stop, low):
    """Identify push high word then low word, indexed by BX."""
    high = (low + 2) & 0xffff
    needle = b"\xff\xb7" + struct.pack("<H", high) + b"\xff\xb7" + struct.pack("<H", low)
    at = start
    while True:
        at = data.find(needle, at, stop)
        if at < 0:
            break
        yield at
        at += 1
def calls(data, start, stop):
    """Report plausible E8 rel16 and 9A far immediate call encodings.
    A match is not proof of an instruction boundary or execution.
    """
    for at in range(start, stop - 4):
        op = data[at]
        if op == 0xE8:
            disp = s16(u16(data, at + 1))
            yield at, "near_rel16", f"fileCandidate=0x{at + 3 + disp:x}", 3
        elif op == 0x9A:
            yield at, "far_imm16_16", f"offset=0x{u16(data,at+1):04x} selector=0x{u16(data,at+3):04x}", 5
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    print(f"M401_INPUT bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}")
    if len(data) <= LE_FILE + 2 or data[:2] != b"MZ" or data[LE_FILE:LE_FILE+2] != b"LE":
        raise SystemExit("M401_ERROR expected known MZ and embedded LE header")
    for name, start, stop, table, count in REGIONS:
        low = (AFFINE_BASE + table) & 0xffff
        print(f"M401_REGION name={name} start=0x{start:x} stop=0x{stop:x} tableFile=0x{table:x} tableAffine=0x{AFFINE_BASE+table:08x} count={count} indexedDisp=0x{low:04x}")
        print(f"M401_REGION_BYTES name={name} hex={data[start:stop].hex()}")
        setups = list(segment_setup(data, start, stop))
        for at, imm, context in setups:
            print(f"M401_DS_SETUP name={name} file=0x{at:x} immediate=0x{imm:04x} context={context} interpretation=mov_ax_imm16_then_mov_ds_ax")
        pairs = list(indexed_pairs(data, start, stop, low))
        for at in pairs:
            print(f"M401_INDEXED_PAIR name={name} file=0x{at:x} bytes={data[at:at+8].hex()} order=high_then_low")
            for delta in (-12, -9, -6, -3):
                p = at + delta
                if p >= start and data[p:p+3] == b"\xc1\xe3\x02":
                    print(f"M401_INDEX_SCALE name={name} file=0x{p:x} bytes=c1e302 operation=shl_bx_2")
        # Use a small forward neighborhood to distinguish calls immediately
        # after a table push from unrelated call-looking bytes elsewhere.
        for at, kind, dest, width in calls(data, start, stop):
            distance = min((at-(p+8) for p in pairs if 0 <= at-(p+8) <= 28), default=None)
            if distance is not None or kind == "far_imm16_16":
                near = "none" if distance is None else str(distance)
                print(f"M401_CALL_CANDIDATE name={name} file=0x{at:x} type={kind} {dest} distanceAfterPair={near} bytes={data[at:at+width].hex()}")
                if kind == "near_rel16":
                    target = at + 3 + s16(u16(data,at+1))
                    if 0 <= target < LE_FILE:
                        lo = max(0,target-16)
                        hi = min(LE_FILE,target+48)
                        print(f"M401_CALLEE_WINDOW name={name} call=0x{at:x} target=0x{target:x} start=0x{lo:x} stop=0x{hi:x} bytes={data[lo:hi].hex()}")
        # Confirm static table entries, without assuming their runtime DS mapping.
        for i in range(count):
            at = table + 4*i
            value = struct.unpack_from("<I",data,at)[0]
            target = value - AFFINE_BASE
            terminator = data.find(b"\x00",target,min(target+80,LE_FILE)) if 0 <= target < LE_FILE else -1
            text = data[target:terminator].decode("ascii","replace") if terminator >= target >= 0 else "?"
            print(f"M401_TABLE_ENTRY name={name} index={i} file=0x{at:x} pointer=0x{value:08x} string={text!r}")
        print(f"M401_REGION_SUMMARY name={name} dsSetupCount={len(setups)} indexedPairCount={len(pairs)}")
    print("M401_LIMITATIONS noControlFlowProof=1 patternRecognitionOnly=1 noRuntimeDSObservation=1 fileRelativeCallsHeuristic=1 noCodeExecution=1")
if __name__ == "__main__":
    main()
