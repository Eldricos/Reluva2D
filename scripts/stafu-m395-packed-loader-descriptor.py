#!/usr/bin/env python3
"""STAFU M395: read-only packed loader descriptor field validation."""
import argparse
import hashlib
import struct
from pathlib import Path

LE = 0x38194
BASE = 0xA4F48C
REGIONS = (
    ("linexe", 0x32A70, 0x32AB0),
    ("candidate_31ed4", 0x31EB4, 0x31F14),
    ("candidate_323f4", 0x323D4, 0x32434),
    ("candidate_3811e", 0x380FE, 0x3813E),
    ("candidate_38144", 0x38124, 0x38164),
)

def u16(data, offset):
    return struct.unpack_from("<H", data, offset)[0]

def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]

def string_at(data, offset):
    if offset < 0 or offset >= LE:
        return None
    end = data.find(b"\x00", offset, min(LE, offset + 128))
    if end <= offset:
        return None
    raw = data[offset:end]
    if not all(32 <= c <= 126 for c in raw):
        return None
    return raw.decode("ascii")

def describe_pointer(data, value):
    target = value - BASE
    text = string_at(data, target)
    at_start = bool(text and (target == 0 or data[target - 1] == 0))
    return target, text, at_start

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    print(f"M395_INPUT bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}")
    if data[:2] != b"MZ" or data[LE:LE+2] != b"LE":
        raise SystemExit("M395_ERROR expected MZ and LE signatures")
    for name, begin, end in REGIONS:
        print(f"M395_REGION name={name} start=0x{begin:x} end=0x{end:x} bytes={data[begin:end].hex()}")
        for offset in range(begin, end - 3):
            value = u32(data, offset)
            target, text, at_start = describe_pointer(data, value)
            if text:
                print(f"M395_POINTER_CANDIDATE region={name} source=0x{offset:x} aligned4={int(offset%4==0)} value=0x{value:08x} target=0x{target:x} atStringStart={int(at_start)} text={text!r}")
    # Deliberately test every byte alignment of the descriptor's first 32 bytes.
    for offset in range(0x32A78, 0x32A98):
        value = u32(data, offset)
        target, text, at_start = describe_pointer(data, value)
        print(f"M395_DESCRIPTOR_BYTE file=0x{offset:x} u32=0x{value:08x} stringTarget={('0x%x'%target) if text else '-'} string={text!r} stringStart={int(at_start)}")
    for offset in range(0x32A78, 0x32AA0, 2):
        print(f"M395_DESCRIPTOR_WORD file=0x{offset:x} u16=0x{u16(data,offset):04x}")
    # Compare context around each repeated 0x00020001 occurrence without
    # assuming they are identical structures.
    signature = struct.pack("<I", 0x00020001)
    found = []
    start = 0
    while True:
        pos = data.find(signature, start, LE)
        if pos < 0:
            break
        found.append(pos)
        start = pos + 1
    print(f"M395_VERSIONLIKE count={len(found)} offsets={','.join(hex(x) for x in found)}")
    for pos in found:
        begin = max(0, pos - 24)
        end = min(LE, pos + 40)
        print(f"M395_VERSION_CONTEXT file=0x{pos:x} before24={data[begin:pos].hex()} after40={data[pos:end].hex()}")
        for delta in (-12,-8,-4,4,8,12):
            source = pos + delta
            if source < 0 or source + 4 > LE:
                continue
            value = u32(data,source)
            target,text,at_start = describe_pointer(data,value)
            print(f"M395_VERSION_FIELD versionFile=0x{pos:x} relative={delta:+d} source=0x{source:x} value=0x{value:08x} string={text!r} atStringStart={int(at_start)}")
    # Search for exact pointer values of candidate descriptor strings to see
    # whether their references are unique. This does not imply code usage.
    for offset in (0x32A7C, 0x32A84, 0x32A8A):
        value = u32(data,offset)
        target,text,at_start = describe_pointer(data,value)
        pattern = struct.pack("<I",value)
        refs = []
        cursor = 0
        while True:
            p = data.find(pattern,cursor,LE)
            if p < 0:break
            refs.append(p)
            cursor = p+1
        print(f"M395_POINTER_REFERENCE source=0x{offset:x} value=0x{value:08x} target=0x{target:x} string={text!r} stringStart={int(at_start)} refs={','.join(hex(x) for x in refs)}")
    print("M395_LIMITATIONS packedLayoutUnverified=1 pointerMappingNotRuntimeValidated=1 noExecution=1")
if __name__ == "__main__":
    main()
