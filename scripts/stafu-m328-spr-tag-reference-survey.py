#!/usr/bin/env python3
"""M328: read-only SPR tag-site mapping and conservative reference survey.

Maps physical OVL tag occurrences to LE object addresses, scans for exact
32-bit address references, and inventories candidate relative calls near
those occurrences. All hits are *candidates*, not decoded instructions.
No proprietary bytes are exported.
"""
import argparse
import hashlib
import pathlib
import struct
import sys

EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TAGS = (b"TRPS", b"TSIL", b"PMOC", b"MOCS", b"EMIT")
LE_OFFSET = 229780
HELPERS = {
    "huffman_builder": (524532, 526264),
    "symbol_decode": (526312, 527800),
    "fixed_block": (528260, 528584),
    "dynamic_block": (528584, 530472),
}

def u32(data, at):
    return struct.unpack_from("<I", data, at)[0]

def occurrences(data, needle):
    pos = 0
    while True:
        at = data.find(needle, pos)
        if at < 0:
            return
        yield at
        pos = at + 1

def load_le(data):
    h = LE_OFFSET
    if data[h:h+2] != b"LE":
        raise ValueError("missing LE header")
    page_size = u32(data, h+0x28)
    obj_rel, count, map_rel = (u32(data,h+0x40),u32(data,h+0x44),u32(data,h+0x48))
    data_offset = u32(data,h+0x80)
    if page_size != 4096 or count != 2 or data_offset != 273920:
        raise ValueError("unexpected LE layout")
    objects = []
    for i in range(count):
        size, base, flags, first, pages = struct.unpack_from("<IIIII", data, h+obj_rel+i*24)
        objects.append((size, base, first, pages))
    physical = {}
    for i, (size, base, first, pages) in enumerate(objects,1):
        for j in range(pages):
            p = h+map_rel+(first-1+j)*4
            page_num = int.from_bytes(data[p:p+3], "big")
            flags = data[p+3]
            if page_num and flags == 0:
                physical[page_num] = (i, base+j*page_size, size-j*page_size)
    return data_offset, page_size, physical

def address_at(pos, data_offset, page_size, physical):
    if pos < data_offset:
        return None
    page, off = divmod(pos-data_offset, page_size)
    entry = physical.get(page+1)
    if entry is None or off >= entry[2]:
        return None
    return (entry[0], entry[1]+off)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=pathlib.Path)
    ap.add_argument("--radius", type=int, default=96,
                    help="Search radius around each tag (default 96 bytes)")
    ap.add_argument("--limit", type=int, default=25,
                    help="Maximum printed hits per category (default 25)")
    args = ap.parse_args()
    if not 0 <= args.radius <= 4096 or args.limit < 1:
        ap.error("radius must be 0..4096 and limit positive")
    try:
        data = args.ovl.read_bytes()
    except OSError as exc:
        print(f"M328_ERROR {exc}", file=sys.stderr)
        return 2
    digest = hashlib.sha256(data).hexdigest()
    valid = len(data) == 2008455 and digest == EXPECTED_SHA
    print(f"M328_INPUT bytes={len(data)} sha256={digest} knownMatch={int(valid)}")
    if not valid:
        print("M328_ERROR unexpected OVL revision", file=sys.stderr)
        return 2
    try:
        data_offset, page_size, physical = load_le(data)
    except (ValueError, struct.error) as exc:
        print(f"M328_ERROR LE={exc}", file=sys.stderr)
        return 2
    print(f"M328_LE dataOffset={data_offset} pageSize={page_size} mappedPages={len(physical)}")
    tag_sites = []
    for tag in TAGS:
        sites = list(occurrences(data, tag))
        print(f"M328_TAG tag={tag.decode()} count={len(sites)}")
        for at in sites:
            addr = address_at(at, data_offset, page_size, physical)
            tag_sites.append((tag, at, addr))
            addr_text = f"object={addr[0]} linear=0x{addr[1]:08X}" if addr else "object=unmapped linear=unknown"
            print(f"M328_SITE tag={tag.decode()} physical={at} {addr_text}")
    # A raw address occurrence may be data, an immediate, a relocation target,
    # or part of unrelated bytes. LE fixups are not applied here.
    for tag, at, addr in tag_sites:
        if addr is None:
            continue
        needle = struct.pack("<I", addr[1])
        hits = list(occurrences(data, needle))
        print(f"M328_ADDRESS_REF tag={tag.decode()} targetPhysical={at} "
              f"linear=0x{addr[1]:08X} rawDwordHits={len(hits)}")
        for site in hits[:args.limit]:
            site_addr = address_at(site, data_offset, page_size, physical)
            source = f"0x{site_addr[1]:08X}" if site_addr else "unknown"
            print(f"M328_REF_CANDIDATE tag={tag.decode()} sourcePhysical={site} "
                  f"sourceLinear={source} targetPhysical={at}")
        if len(hits) > args.limit:
            print(f"M328_TRUNCATED kind=refs tag={tag.decode()} omitted={len(hits)-args.limit}")
    # An E8 byte is not necessarily an instruction boundary. Scan only bounded
    # windows around observed tag sites; report raw candidate rel32 targets.
    for tag, at, addr in tag_sites:
        lo = max(0, at-args.radius)
        hi = min(len(data), at+4+args.radius)
        calls = []
        for site in range(lo, max(lo,hi-4)):
            if data[site] != 0xE8:
                continue
            target = site+5+struct.unpack_from("<i",data,site+1)[0]
            if not 0 <= target < len(data):
                continue
            label = next((n for n,(a,b) in HELPERS.items() if a <= target < b), "other")
            calls.append((site,target,label))
        print(f"M328_NEAR_CALLS tag={tag.decode()} tagPhysical={at} "
              f"candidateE8={len(calls)}")
        for site,target,label in calls[:args.limit]:
            print(f"M328_CALL_CANDIDATE tag={tag.decode()} site={site} "
                  f"target={target} targetRegion={label}")
        if len(calls) > args.limit:
            print(f"M328_TRUNCATED kind=calls tag={tag.decode()} omitted={len(calls)-args.limit}")
    print("M328_LIMITATION rawTagSitesNotValidatedCode=1 referencesNotRelocationVerified=1 "
          "E8NotInstructionBoundaryVerified=1 runtimeCallPathUnverified=1 sprDecoderUnverified=1")
    return 0

if __name__ == "__main__":
    sys.exit(main())
