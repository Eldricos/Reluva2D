#!/usr/bin/env python3
"""Milestone 189: read-only investigation of the nonnegative resource processing path."""
import hashlib
import pathlib
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
CALL_SITE = 513786
CALL_BYTES = bytes.fromhex('e8 ad 00 00 00')
TARGET = 513964
WINDOWS = (
    ('processing', TARGET - 64, TARGET + 4096),
    ('nonnegative-path', 513632 - 32, 513869 + 32),
    ('negative-path', 513428 - 32, 513632 + 32),
)

def main():
    if len(sys.argv) not in (2, 3):
        print('Usage: python stafu-m189-processing-extract.py <STTNG.OVL> [output-folder]')
        return 2
    path = pathlib.Path(sys.argv[1]); out = pathlib.Path(sys.argv[2]) if len(sys.argv) == 3 else pathlib.Path('.')
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print(f'RM_M189_INPUT path={path} bytes={len(data)} sha256={digest} knownShaMatch={int(digest==EXPECTED_SHA)}')
    actual = data[CALL_SITE:CALL_SITE+5]
    target = CALL_SITE + 5 + int.from_bytes(actual[1:5], 'little', signed=True) if len(actual)==5 and actual[0]==0xe8 else -1
    valid = actual == CALL_BYTES and target == TARGET and digest == EXPECTED_SHA
    print(f'RM_M189_CALL site={CALL_SITE} bytes={actual.hex()} expectedBytesMatch={int(actual==CALL_BYTES)} computedTarget={target} targetMatch={int(target==TARGET)}')
    if not valid:
        print('RM_M189_ERROR verification failed; no files written')
        return 1
    out.mkdir(parents=True, exist_ok=True)
    for name, start, end in WINDOWS:
        if start < 0 or end > len(data):
            print(f'RM_M189_ERROR windowOutOfRange name={name}')
            return 1
        blob = data[start:end]
        filename = f'm189-{name}-{start}.bin'
        (out/filename).write_bytes(blob)
        print(f'RM_M189_WINDOW name={name} from={start} to={end} targetRelative={TARGET-start if name=="processing" else "n/a"} bytes={len(blob)} file={filename} sha256={hashlib.sha256(blob).hexdigest()}')
    for pos in range(TARGET, TARGET+160, 16):
        print(f'RM_M189_HEX file={pos} hex={data[pos:pos+16].hex(" ")}')
    print('RM_M189_LIMITATION staticOnly=1 processingFunctionEndUnverified=1 semanticsUnverified=1')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
