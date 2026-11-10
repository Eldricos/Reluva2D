#!/usr/bin/env python3
"""STAFU M195: read-only extraction of mode-8 processing helpers."""
import hashlib
import pathlib
import struct
import sys

EXPECTED = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
if len(sys.argv) != 2:
    sys.exit('Usage: python stafu-m195-stateful-helper-extract.py games/STTNG/STTNG.OVL')
p = pathlib.Path(sys.argv[1]); b = p.read_bytes(); sha = hashlib.sha256(b).hexdigest()
print(f'RM_M195_INPUT path={p} bytes={len(b)} sha256={sha} knownShaMatch={int(sha==EXPECTED)}')
if sha != EXPECTED: sys.exit('ERROR: unexpected executable; no output written')
checks = [(514085,530840),(514119,530944),(530879,530472)]
for site, target in checks:
    raw=b[site:site+5]
    computed=site+5+struct.unpack_from('<i',b,site+1)[0] if len(raw)==5 and raw[0]==0xe8 else None
    ok=computed==target
    print(f'RM_M195_CALL site={site} bytes={raw.hex()} target={computed} expected={target} match={int(ok)}')
    if not ok: sys.exit('ERROR: call target mismatch; no output written')
windows=[('state-step',530472-64,530472+3072),('state-init',530840-64,530840+512),('state-cleanup',530944-64,530944+512),('mode8',514071,514141)]
for name,start,end in windows:
    if start<0 or end>len(b): sys.exit('ERROR: window out of bounds')
    chunk=b[start:end]; out=pathlib.Path(f'm195-{name}-{start}.bin'); out.write_bytes(chunk)
    print(f'RM_M195_WINDOW name={name} from={start} to={end} bytes={len(chunk)} file={out} sha256={hashlib.sha256(chunk).hexdigest()}')
print('RM_M195_LIMITATION staticOnly=1 helperSemanticsUnverified=1 resourceModeForBRIDGE_RMUnverified=1')
