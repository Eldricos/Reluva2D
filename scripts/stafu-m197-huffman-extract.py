
#!/usr/bin/env python3
"""STAFU M197: read-only extraction of Huffman-related helpers from STTNG.OVL."""
import hashlib
import pathlib
import struct
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
EXPECTED_SIZE = 2008455
CALLS = [(528409,524532),(528488,524532),(528512,526264),(528560,526312),
         (529310,524532),(529333,526264),(530307,524532),(530372,524532)]
WINDOWS = [
 ('huffman-builder',524468,526300),
 ('symbol-helper',526200,527260),
 ('decode-helper',526248,527800),
 ('fixed-handler',528260,528584),
 ('dynamic-handler',528584,530472),
 ('output-copy',514476,515500),
]

def main():
    if len(sys.argv) != 2:
        sys.exit('Usage: python stafu-m197-huffman-extract.py games/STTNG/STTNG.OVL')
    path = pathlib.Path(sys.argv[1]); data = path.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    ok = sha == EXPECTED_SHA and len(data) == EXPECTED_SIZE
    print(f'RM_M197_INPUT path={path} bytes={len(data)} sha256={sha} knownMatch={int(ok)}')
    if not ok:
        sys.exit('ERROR: unexpected executable; no files written')
    for site, target in CALLS:
        raw = data[site:site+5]
        calc = site+5+struct.unpack_from('<i',data,site+1)[0] if raw[0] == 0xe8 else None
        match = calc == target
        print(f'RM_M197_CALL site={site} bytes={raw.hex()} target={calc} expected={target} match={int(match)}')
        if not match:
            sys.exit('ERROR: call verification failed; no files written')
    for name, start, end in WINDOWS:
        chunk = data[start:end]
        out = pathlib.Path(f'm197-{name}-{start}.bin')
        out.write_bytes(chunk)
        print(f'RM_M197_WINDOW name={name} from={start} to={end} bytes={len(chunk)} file={out} sha256={hashlib.sha256(chunk).hexdigest()}')
        print(f'RM_M197_HEX name={name} first32={chunk[:32].hex()}')
    print('RM_M197_LIMITATION staticOnly=1 algorithmsUnverified=1 bridgeResourceModeUnverified=1')

if __name__ == '__main__':
    main()
