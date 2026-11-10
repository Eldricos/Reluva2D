#!/usr/bin/env python3
"""STAFU M204: read-only extraction for palette-source and wrapper analysis."""
import argparse
import hashlib
from pathlib import Path
import struct
import sys

EXPECTED_SIZE = 2008455
EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
WINDOWS = [
    ('palette-source-and-copy', 509800, 511400),
    ('palette-update-helper', 510200, 510388),
    ('resource-wrapper', 562700, 563450),
    ('resource-inner-loader', 513240, 514080),
    ('resource-lookup', 515980, 516800),
    ('palette-dac', 637700, 638650),
    ('palette-upper-caller-a', 1010400, 1010850),
    ('palette-upper-caller-b', 1012050, 1012450),
]
CALLS = [(510412, 562864), (510563, 562864),
         (510648, 510232), (562901, 513376),
         (510348, 638078), (510376, 638078)]

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('ovl', type=Path)
    ap.add_argument('--out', type=Path, default=Path('m204-output'))
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    ok = len(data) == EXPECTED_SIZE and digest == EXPECTED_SHA and data[229780:229782] == b'LE'
    print(f'RM_M204_INPUT bytes={len(data)} sha256={digest} knownMatch={int(ok)}')
    if not ok:
        print('RM_M204_ERROR unexpectedExecutable=1 noFilesWritten=1')
        return 2
    checks = []
    for site, expected in CALLS:
        op = data[site:site+5]
        actual = site+5+struct.unpack_from('<i', data, site+1)[0] if op[0] == 0xe8 else None
        passed = actual == expected
        checks.append(passed)
        print(f'RM_M204_CALL site={site} target={actual} expected={expected} match={int(passed)} bytes={op.hex()}')
    if not all(checks):
        print('RM_M204_ERROR callVerificationFailed=1 noFilesWritten=1')
        return 2
    args.out.mkdir(parents=True, exist_ok=True)
    manifest = []
    for name, start, end in WINDOWS:
        chunk = data[start:end]
        filename = f'm204-{name}-{start}.bin'
        (args.out/filename).write_bytes(chunk)
        digest = hashlib.sha256(chunk).hexdigest()
        manifest.append(f'{filename} start={start} end={end} sha256={digest}')
        print(f'RM_M204_WINDOW name={name} start={start} end={end} bytes={len(chunk)} sha256={digest} file={filename}')
    # This is a literal argument to the wrapper, not necessarily a numeric resource ID.
    print(f'RM_M204_ARGUMENT site=510402 value=0x00000c08 interpretation=unverified')
    (args.out/'m204-manifest.txt').write_text('\n'.join(manifest)+'\n', encoding='utf-8')
    print(f'RM_M204_OUTPUT directory={args.out} files={len(WINDOWS)+1}')
    print('RM_M204_LIMITATION staticOnly=1 wrapperArgumentMeaningUnverified=1 paletteResourceNameUnverified=1 lowerHalfSourceUnverified=1')
    return 0

if __name__ == '__main__':
    sys.exit(main())
