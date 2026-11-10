#!/usr/bin/env python3
"""STAFU M187: read-only extraction of STTNG.OVL inner helper.

Usage: python stafu-m187-inner-helper.py path/to/STTNG.OVL
Writes m187-inner-helper-513376.bin to the current directory.
"""
import hashlib
import pathlib
import struct
import sys

CALL_SITE = 562901
TARGET = 513376
BEFORE = 64
AFTER = 4096
EXPECTED_CALL = bytes.fromhex('e8 86 3e ff ff')

def main():
    if len(sys.argv) != 2:
        print('Usage: python stafu-m187-inner-helper.py <STTNG.OVL>', file=sys.stderr)
        return 2
    path = pathlib.Path(sys.argv[1])
    try:
        data = path.read_bytes()
    except OSError as exc:
        print(f'RM_M187_ERROR read={exc}', file=sys.stderr)
        return 2
    sha = hashlib.sha256(data).hexdigest()
    print(f'RM_M187_INPUT path={path} bytes={len(data)} sha256={sha}')
    if len(data) < TARGET + AFTER or len(data) < CALL_SITE + 5:
        print('RM_M187_ERROR reason=fileTooShort')
        return 1
    observed = data[CALL_SITE:CALL_SITE+5]
    relative = struct.unpack_from('<i', data, CALL_SITE + 1)[0]
    actual_target = CALL_SITE + 5 + relative
    ok = observed == EXPECTED_CALL and actual_target == TARGET
    print(f'RM_M187_CALL site={CALL_SITE} bytes={observed.hex()} expectedBytesMatch={int(observed == EXPECTED_CALL)} computedTarget={actual_target} targetMatch={int(actual_target == TARGET)}')
    if not ok:
        print('RM_M187_ERROR reason=callSignatureMismatch extractionAborted=1')
        return 1
    start = TARGET - BEFORE
    end = TARGET + AFTER
    chunk = data[start:end]
    out = pathlib.Path(f'm187-inner-helper-{TARGET}.bin')
    out.write_bytes(chunk)
    print(f'RM_M187_WINDOW from={start} to={end} targetOffset={TARGET} targetRelative={BEFORE} bytes={len(chunk)} file={out} sha256={hashlib.sha256(chunk).hexdigest()}')
    for offset in range(TARGET, TARGET+256, 16):
        print(f'RM_M187_HEX file={offset} hex={data[offset:offset+16].hex(" ")}')
    print('RM_M187_LIMITATION staticBytesOnly=1 functionEndUnverified=1 outputPointerWritesUnverified=1 returnSemanticsUnverified=1')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
