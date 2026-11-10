#!/usr/bin/env python3
"""STAFU M186: read-only physical-offset helper extraction from installed STTNG.OVL."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import shutil
import sys

TARGET = 562864
CLEANUP = 562937
# The M183 cleanup window starts at 562905; its known entry begins 32 bytes later.
CLEANUP_PROLOGUE = bytes.fromhex('53 56 57 55 89 e5 81 ec 00 00 00 00')
# Loader call to the target at physical offset 510563: E8 48 CC 00 00.
CALL_SITE = 510563
CALL_BYTES = bytes.fromhex('e8 48 cc 00 00')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('ovl', type=Path, help='Installed game STTNG.OVL (not TITLE.FVF)')
    p.add_argument('--out', type=Path, default=Path('m186-helper-562864.bin'))
    p.add_argument('--before', type=int, default=32)
    p.add_argument('--after', type=int, default=1536)
    args = p.parse_args()
    if args.before < 0 or args.after < 256 or args.after > 65536:
        p.error('Require --before >= 0 and 256 <= --after <= 65536')
    data = args.ovl.read_bytes()
    print(f'RM_M186_INPUT path={args.ovl} bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}')
    if len(data) < TARGET + args.after:
        sys.exit('RM_M186_ERROR fileTooShort=1')
    observed_call = data[CALL_SITE:CALL_SITE+5]
    displacement = int.from_bytes(observed_call[1:5], 'little', signed=True)
    call_target = CALL_SITE + 5 + displacement if observed_call[:1] == b'\xe8' else -1
    print(f'RM_M186_CALL site={CALL_SITE} bytes={observed_call.hex()} expectedBytesMatch={int(observed_call==CALL_BYTES)} computedTarget={call_target} targetMatch={int(call_target==TARGET)}')
    prologue = data[CLEANUP:CLEANUP+len(CLEANUP_PROLOGUE)]
    print(f'RM_M186_CLEANUP offset={CLEANUP} prologue={prologue.hex()} expectedMatch={int(prologue==CLEANUP_PROLOGUE)}')
    if observed_call != CALL_BYTES or call_target != TARGET or prologue != CLEANUP_PROLOGUE:
        sys.exit('RM_M186_ERROR knownAnchorMismatch=1 (check game file and offsets; no output written)')
    start = TARGET-args.before
    end = TARGET+args.after
    chunk = data[start:end]
    args.out.write_bytes(chunk)
    print(f'RM_M186_WINDOW from={start} to={end} targetOffset={TARGET} targetRelative={args.before} bytes={len(chunk)} file={args.out} sha256={hashlib.sha256(chunk).hexdigest()}')
    for offset in range(TARGET, min(TARGET+256,end), 16):
        print(f'RM_M186_HEX file={offset} hex={data[offset:offset+16].hex(" ")}')
    tool = shutil.which('objdump')
    if tool:
        print('RM_M186_DISASSEMBLY_BEGIN tool=objdump architecture=i386 syntax=intel')
        cmd = [tool, '-D', '-b', 'binary', '-m', 'i386', '-M', 'intel',
               '--adjust-vma', str(start), str(args.out)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            lines = result.stdout.splitlines()
            # Show the target and a bounded excerpt; retain complete binary for follow-up.
            chosen = [line for line in lines if line.strip().startswith(f'{TARGET:x}:')]
            print(f'RM_M186_DISASM_TARGET found={int(bool(chosen))}')
            idx = next((i for i, line in enumerate(lines) if line.strip().startswith(f'{TARGET:x}:')), 0)
            print('\n'.join(lines[idx:idx+100]))
        else:
            print(f'RM_M186_DISASSEMBLY_ERROR returncode={result.returncode} stderr={result.stderr[:200]!r}')
        print('RM_M186_DISASSEMBLY_END')
    else:
        print('RM_M186_DISASSEMBLY unavailable=1 reason=objdumpNotInstalled')
    print('RM_M186_LIMITATION staticOnly=1 functionEndUnverified=1 indirectCallsUnresolved=1 stackWriteUnverified=1 returnValueUnverified=1')

if __name__ == '__main__':
    main()
