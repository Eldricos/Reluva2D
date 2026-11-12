#!/usr/bin/env python3
"""STAFU M397: discover and validate packed DOS/4GW loader descriptors (read-only)."""
import argparse
import hashlib
import struct
from pathlib import Path

BASE = 0xA4F48C
LE = 0x38194
MAX_EXPORTS = 256
KNOWN = {"LINEXE_LOADER": 0x32A78, "D32_INT31": 0x323EC}

def u16(data, pos):
    return struct.unpack_from("<H", data, pos)[0]

def u32(data, pos):
    return struct.unpack_from("<I", data, pos)[0]

def name_at(data, pos):
    if not (0 <= pos < LE):
        return None
    end = data.find(b"\x00", pos, min(LE, pos + 96))
    if end <= pos:
        return None
    raw = data[pos:end]
    if not all(32 <= x <= 126 for x in raw):
        return None
    if pos and data[pos - 1] != 0:
        return None
    return raw.decode("ascii")

def resolved(data, pointer):
    pos = pointer - BASE
    return pos, name_at(data, pos)

def export(data, pos):
    if not (0 <= pos and pos + 8 <= LE):
        return None
    namepos, name = resolved(data, u32(data, pos))
    code = u32(data, pos + 4)
    if name is None or not (code >> 16):
        return None
    return name, code, namepos

def candidate(data, pos):
    if pos + 22 > LE:
        return None
    namepos, module = resolved(data, u32(data, pos + 4))
    if not module:
        return None
    firstpos = u32(data, pos + 12) - BASE
    secondpos = u32(data, pos + 18) - BASE
    count = u16(data, pos + 16)
    if not (0 <= firstpos < secondpos <= LE):
        return None
    gap = secondpos - firstpos
    if gap % 8 or gap > MAX_EXPORTS * 8 or count > MAX_EXPORTS:
        return None
    firstcount = gap // 8
    total = firstcount + count
    if firstcount != 2 or total > MAX_EXPORTS or total < 3:
        return None
    if secondpos + count * 8 > LE:
        return None
    records = []
    for i in range(total):
        entry = export(data, firstpos + i * 8)
        if entry is None:
            return None
        records.append(entry)
    if [x[0] for x in records[:2]] != ["INIT", "DEINIT"]:
        return None
    return {
        "file": pos, "name": module, "namepos": namepos,
        "reserved": u32(data, pos), "version0": u16(data, pos + 8),
        "version1": u16(data, pos + 10), "first": firstpos,
        "second": secondpos, "firstcount": firstcount,
        "secondcount": count, "records": records,
    }

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    print(f"M397_INPUT bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}")
    if data[:2] != b"MZ" or data[LE:LE + 2] != b"LE":
        raise SystemExit("M397_ERROR expected MZ and LE signatures")
    found = []
    for pos in range(0, LE - 21):
        item = candidate(data, pos)
        if item:
            found.append(item)
    print(f"M397_SCAN range=0x0..0x{LE:x} step=1 candidates={len(found)}")
    for item in found:
        print(
            f"M397_DESCRIPTOR file=0x{item['file']:x} name={item['name']!r} "
            f"nameFile=0x{item['namepos']:x} reserved=0x{item['reserved']:08x} "
            f"versions={item['version0']},{item['version1']} "
            f"first=0x{item['first']:x} second=0x{item['second']:x} "
            f"firstCount={item['firstcount']} secondCount={item['secondcount']} "
            f"total={len(item['records'])}"
        )
        for i, (name, code, namepos) in enumerate(item["records"]):
            group = "initial" if i < item["firstcount"] else "additional"
            print(
                f"M397_EXPORT module={item['name']!r} index={i} group={group} "
                f"name={name!r} nameFile=0x{namepos:x} "
                f"code=0x{code:08x} high16=0x{code >> 16:04x}"
            )
        selectors = sorted(set(code >> 16 for _, code, _ in item["records"]))
        print(f"M397_MODULE_SUMMARY name={item['name']!r} selectors={','.join(hex(s) for s in selectors)} allNamesValid=1 allGroupsContiguous=1")
    for name, offset in KNOWN.items():
        matches = [x for x in found if x["file"] == offset and x["name"] == name]
        print(f"M397_KNOWN name={name!r} file=0x{offset:x} matched={int(bool(matches))}")
    # Report weaker descriptor-shaped structures separately, without
    # classifying them as validated matches.
    weak = []
    for pos in range(0, LE - 21):
        namepos, module = resolved(data, u32(data, pos + 4))
        if not module or module in ("INIT", "DEINIT"):
            continue
        if (u16(data, pos + 8), u16(data, pos + 10)) != (1, 2):
            continue
        first = u32(data, pos + 12) - BASE
        second = u32(data, pos + 18) - BASE
        count = u16(data, pos + 16)
        if not (0 <= first < second <= LE and (second - first) == 16 and 0 < count <= MAX_EXPORTS):
            continue
        if any(x["file"] == pos for x in found):
            continue
        weak.append((pos, module, first, second, count))
    print(f"M397_WEAK_COUNT count={len(weak)}")
    for pos, module, first, second, count in weak[:80]:
        print(f"M397_WEAK file=0x{pos:x} name={module!r} first=0x{first:x} second=0x{second:x} secondCount={count}")
    print("M397_LIMITATIONS staticDescriptorInference=1 affinePointerModel=1 noRuntimeRegistrationEvidence=1")
if __name__ == "__main__":
    main()
