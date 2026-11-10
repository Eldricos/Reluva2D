#!/usr/bin/env python3
"""STAFU M188: read-only resource lookup and call-site extraction."""
import argparse
import hashlib
from pathlib import Path

LOOKUP = 516148
CALL = 513402
INNER = 513376

def hx(data, offset, length=16):
    return ' '.join(f'{b:02x}' for b in data[offset:offset+length])

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('overlay',type=Path,help='Installed STTNG.OVL')
    p.add_argument('--out-dir',type=Path,default=Path('.'))
    args=p.parse_args()
    data=args.overlay.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    print(f'RM_M188_INPUT path={args.overlay} bytes={len(data)} sha256={digest}')
    if len(data)<LOOKUP+4096:
        raise SystemExit('ERROR: file too small for expected lookup window')
    actual=data[CALL:CALL+5]
    expected=bytes.fromhex('e8 b5 0a 00 00')
    target=CALL+5+int.from_bytes(actual[1:5],'little',signed=True) if actual[:1]==b'\xe8' else -1
    ok=(actual==expected and target==LOOKUP)
    print(f'RM_M188_CALL site={CALL} bytes={actual.hex()} expectedBytesMatch={int(actual==expected)} computedTarget={target} targetMatch={int(target==LOOKUP)}')
    if not ok:
        raise SystemExit('ERROR: lookup call mismatch; no windows written')
    args.out_dir.mkdir(parents=True,exist_ok=True)
    windows=[('lookup',LOOKUP-64,LOOKUP+4096),('inner-callsite',INNER,INNER+640),('inner-output',513720,513960),('loader-callsite',510528,510744)]
    for name,start,end in windows:
        block=data[start:end]
        path=args.out_dir/f'm188-{name}-{start}.bin'
        path.write_bytes(block)
        print(f'RM_M188_WINDOW name={name} from={start} to={end} bytes={len(block)} file={path} sha256={hashlib.sha256(block).hexdigest()}')
    for offset in range(LOOKUP,LOOKUP+256,16):
        print(f'RM_M188_LOOKUP_HEX file={offset} hex={hx(data,offset)}')
    print('RM_M188_LIMITATION staticOnly=1 lookupEndUnverified=1 recordFieldSemanticsUnverified=1')

if __name__=='__main__':
    main()
