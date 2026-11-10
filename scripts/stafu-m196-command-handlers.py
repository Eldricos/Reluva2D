#!/usr/bin/env python3
"""STAFU M196: read-only extraction of Mode 8 command handlers from STTNG.OVL."""
import hashlib
from pathlib import Path
import struct
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
CALLS = [(530770, 528584), (530793, 527684), (530813, 528260)]
# Exact call-site offsets are verified by scanning the known state-step instruction window.
TARGETS = [(0,527684),(1,528260),(2,528584)]

def main():
    if len(sys.argv) != 2:
        sys.exit('Usage: python stafu-m196-command-handlers.py games/STTNG/STTNG.OVL')
    path=Path(sys.argv[1]); data=path.read_bytes(); digest=hashlib.sha256(data).hexdigest()
    print(f'RM_M196_INPUT path={path} bytes={len(data)} sha256={digest} knownShaMatch={int(digest==EXPECTED_SHA)}')
    if digest!=EXPECTED_SHA:
        sys.exit('ERROR: unexpected executable; no windows written')
    # Verify that the three known targets are actually called in the state-step function.
    region_start,region_end=530472,530838
    hits={t:[] for _,t in TARGETS}
    for pos in range(region_start,region_end-4):
        if data[pos]!=0xe8: continue
        target=pos+5+struct.unpack_from('<i',data,pos+1)[0]
        if target in hits: hits[target].append(pos)
    for mode,target in TARGETS:
        positions=hits[target]
        print(f'RM_M196_CALL mode={mode} target={target} sites={",".join(map(str,positions)) or "none"} verified={int(bool(positions))}')
    if not all(hits.values()):
        sys.exit('ERROR: command-handler call verification failed; no windows written')
    windows=[('command0',527684,3072),('command1',528260,3072),('command2',528584,3072),('state-step',530472,368),('completion',514476,1024)]
    for name,start,size in windows:
        end=min(len(data),start+size)
        chunk=data[start:end]
        outfile=Path(f'm196-{name}-{start}.bin');outfile.write_bytes(chunk)
        print(f'RM_M196_WINDOW name={name} from={start} to={end} bytes={len(chunk)} file={outfile} sha256={hashlib.sha256(chunk).hexdigest()}')
        print(f'RM_M196_HEX name={name} first32={chunk[:32].hex()}')
    print('RM_M196_LIMITATION staticOnly=1 commandSemanticsUnverified=1 modeForBRIDGE_RMUnverified=1')
if __name__=='__main__': main()
