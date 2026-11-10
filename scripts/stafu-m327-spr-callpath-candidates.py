#!/usr/bin/env python3
"""M327: conservative static search for SPR tag consumers in STTNG.OVL.

This is a *candidate finder*, not a disassembler or a call-graph proof.
It never writes extracted proprietary bytes and requires the known OVL hash.
"""
import argparse
import hashlib
import pathlib
import struct
import sys

SHA256 = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TAGS = (b"TRPS", b"TSIL", b"PMOC", b"MOCS", b"EMIT")
KNOWN_HELPERS = {
    "huffman_builder": (524532, 526264),
    "symbol_decode": (526312, 527800),
    "fixed_block": (528260, 528584),
    "dynamic_block": (528584, 530472),
}

def offsets(data, needle):
    start = 0
    while True:
        at = data.find(needle, start)
        if at < 0:
            return
        yield at
        start = at + 1

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=pathlib.Path)
    ap.add_argument("--radius", type=int, default=128,
                    help="Bytes on either side of tag occurrences to inspect (default 128)")
    ap.add_argument("--limit", type=int, default=50,
                    help="Maximum occurrences reported per tag (default 50)")
    args = ap.parse_args()
    if args.radius < 0 or args.radius > 4096 or args.limit < 1:
        ap.error("radius must be 0..4096 and limit must be positive")
    try:
        data = args.ovl.read_bytes()
    except OSError as exc:
        print(f"M327_ERROR input={exc}", file=sys.stderr)
        return 2
    digest = hashlib.sha256(data).hexdigest()
    valid = len(data) == 2008455 and digest == SHA256
    print(f"M327_INPUT bytes={len(data)} sha256={digest} knownMatch={int(valid)}")
    if not valid:
        print("M327_ERROR unsupported executable revision", file=sys.stderr)
        return 2

    # Raw byte matches can occur in compressed data, operands, or arbitrary data.
    for tag in TAGS:
        hits = list(offsets(data, tag))
        print(f"M327_TAG tag={tag.decode()} rawOccurrences={len(hits)}")
        for at in hits[:args.limit]:
            lo, hi = max(0, at - args.radius), min(len(data), at + 4 + args.radius)
            near = []
            for name, (start, end) in KNOWN_HELPERS.items():
                if lo < end and hi > start:
                    near.append(name)
            print(f"M327_TAG_HIT tag={tag.decode()} physical={at} "
                  f"window={lo}:{hi} helperWindowOverlap={','.join(near) or 'none'}")
        if len(hits) > args.limit:
            print(f"M327_TRUNCATED tag={tag.decode()} omitted={len(hits)-args.limit}")

    # E8 rel32 is a *candidate* direct call only when the byte is an instruction
    # boundary. Scanning every byte does NOT establish instruction boundaries.
    for name, (start, end) in KNOWN_HELPERS.items():
        hits = []
        for at in range(0, len(data) - 4):
            if data[at] != 0xE8:
                continue
            target = at + 5 + struct.unpack_from("<i", data, at + 1)[0]
            if start <= target < end:
                hits.append((at, target))
        print(f"M327_HELPER name={name} candidateE8Calls={len(hits)}")
        for at, target in hits[:args.limit]:
            print(f"M327_CALL_CANDIDATE helper={name} site={at} target={target}")
        if len(hits) > args.limit:
            print(f"M327_TRUNCATED helper={name} omitted={len(hits)-args.limit}")
    print("M327_LIMITATION staticOnly=1 rawTagsNotParsers=1 "
          "E8CandidatesNotVerifiedInstructions=1 runtimeResourceConnectionUnverified=1 "
          "codecUnverified=1")
    return 0

if __name__ == "__main__":
    sys.exit(main())
