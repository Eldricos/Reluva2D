#!/usr/bin/env python3
"""STAFU M198: read-only extraction for Huffman symbol and back-reference paths.
Reads the original STTNG.OVL; writes only separate diagnostic .bin files.
"""
import hashlib
import pathlib
import struct
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
EXPECTED_SIZE = 2008455
SIGNATURES = {
    'symbol-decode': (526312, bytes.fromhex('5356575583ec1ca1dca50900')),
    'fixed-block': (528260, bytes.fromhex('53565755833deca5090000')),
    'dynamic-block': (528584, bytes.fromhex('5356575583ec2c')),
    'output-helper': (514476, bytes.fromhex('5356578b54241485d2')),
}
CALLS = [
    (528512, 526264),
    (528560, 526312),
    (529333, 526264),
]
WINDOWS = [
    ('symbol-decode', 526312, 527800),
    ('symbol-context', 526200, 528200),
    ('fixed-block', 528260, 528584),
    ('dynamic-block', 528584, 530472),
    ('output-helper', 514476, 515500),
    ('huffman-builder', 524532, 526264),
]

def main():
    if len(sys.argv) != 2:
        sys.exit('Usage: python stafu-m198-symbol-paths.py games/STTNG/STTNG.OVL')
    path = pathlib.Path(sys.argv[1])
    data = path.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    ok = len(data) == EXPECTED_SIZE and sha == EXPECTED_SHA
    print(f'RM_M198_INPUT path={path} bytes={len(data)} sha256={sha} knownMatch={int(ok)}')
    if not ok:
        sys.exit('ERROR: unexpected executable; no windows written')
    for name, (at, expected) in SIGNATURES.items():
        actual = data[at:at+len(expected)]
        match = actual == expected
        print(f'RM_M198_SIGNATURE name={name} offset={at} actual={actual.hex()} match={int(match)}')
        if not match:
            sys.exit('ERROR: signature mismatch; no windows written')
    for site, target in CALLS:
        raw = data[site:site+5]
        computed = site + 5 + struct.unpack_from('<i', data, site+1)[0] if raw[0] == 0xE8 else None
        match = computed == target
        print(f'RM_M198_CALL site={site} bytes={raw.hex()} target={computed} expected={target} match={int(match)}')
        if not match:
            sys.exit('ERROR: call mismatch; no windows written')
    for name, start, end in WINDOWS:
        chunk = data[start:end]
        out = pathlib.Path(f'm198-{name}-{start}.bin')
        out.write_bytes(chunk)
        print(f'RM_M198_WINDOW name={name} from={start} to={end} bytes={len(chunk)} file={out} sha256={hashlib.sha256(chunk).hexdigest()}')
    print('RM_M198_LIMITATION staticOnly=1 lengthDistanceSemanticsUnverified=1 bridgeResourceModeUnverified=1')

if __name__ == '__main__':
    main()
