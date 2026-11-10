#!/usr/bin/env python3
"""STAFU M194: read-only extraction of processing dispatcher and direct call targets."""
import hashlib
import pathlib
import struct
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
MODE0 = 513995
MODE8 = 514071
DISPATCH = 513964


def emit_window(data, label, start, end):
    start = max(0, start)
    end = min(len(data), end)
    path = pathlib.Path(f'm194-{label}-{start}.bin')
    chunk = data[start:end]
    path.write_bytes(chunk)
    print(f'RM_M194_WINDOW name={label} from={start} to={end} bytes={len(chunk)} file={path} sha256={hashlib.sha256(chunk).hexdigest()}')


def calls(data, label, start, end):
    """Locate candidate E8 rel32 calls; report candidates, not a disassembly."""
    found = []
    for site in range(start, end - 4):
        if data[site] != 0xE8:
            continue
        delta = struct.unpack_from('<i', data, site + 1)[0]
        target = site + 5 + delta
        if 0 <= target < len(data):
            found.append((site, target))
    print(f'RM_M194_CALL_SCAN name={label} candidates={len(found)} disassemblyVerified=0')
    for site, target in found:
        print(f'RM_M194_CALL_CANDIDATE name={label} site={site} target={target} bytes={data[site:site+5].hex()}')
    return found


def main():
    if len(sys.argv) != 2:
        sys.exit('Usage: python stafu-m194-handler-trace.py games/STTNG/STTNG.OVL')
    source = pathlib.Path(sys.argv[1])
    data = source.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    valid = digest == EXPECTED_SHA
    print(f'RM_M194_INPUT path={source} bytes={len(data)} sha256={digest} knownShaMatch={int(valid)}')
    if not valid:
        sys.exit('ERROR: executable SHA-256 mismatch; no windows written')
    signatures = {
        'dispatcher': (DISPATCH, bytes.fromhex('535657558b4424148b5c241831ed83f8')),
        'mode0': (MODE0, bytes.fromhex('b6018b3d24970900')),
        'mode8': (MODE8, bytes.fromhex('b401891ddc960900')),
    }
    for name, (offset, signature) in signatures.items():
        matched = data[offset:offset + len(signature)] == signature
        print(f'RM_M194_SIGNATURE name={name} offset={offset} expected={signature.hex()} actual={data[offset:offset+len(signature)].hex()} match={int(matched)}')
        if not matched:
            sys.exit('ERROR: handler signature mismatch; no windows written')

    emit_window(data, 'dispatcher', 513928, 514160)
    emit_window(data, 'resource-loader', 513376, 513900)
    emit_window(data, 'mode0', 513995, 514071)
    emit_window(data, 'mode8', 514071, 514141)

    # These are known call sites from M189; validate their opcodes and targets.
    known_sites = [514047, 514085, 514117]
    targets = []
    for site in known_sites:
        if data[site] != 0xE8:
            print(f'RM_M194_CALL site={site} opcode={data[site]:02x} isCall=0')
            continue
        delta = struct.unpack_from('<i', data, site + 1)[0]
        target = site + 5 + delta
        print(f'RM_M194_CALL site={site} bytes={data[site:site+5].hex()} target={target} inFile={int(0 <= target < len(data))}')
        if 0 <= target < len(data):
            targets.append(target)
    for i, target in enumerate(sorted(set(targets))):
        emit_window(data, f'helper{i+1}', target - 64, target + 2048)
    print('RM_M194_LIMITATION staticOnly=1 helperBoundariesUnverified=1 modeForBRIDGE_RMUnverified=1')

if __name__ == '__main__':
    main()
