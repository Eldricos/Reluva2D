#!/usr/bin/env python3
"""STAFU M203: read-only palette-update routine and buffer-reference survey.

Reports raw call/reference candidates, not verified instruction boundaries.
No relocations are applied; no original game files are modified.
"""
import argparse
import hashlib
import pathlib
import struct
import sys

SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
SIZE = 2008455
HEADER = 229780
TARGET = 638078
BUFFERS = {'palette-base': 0x11F6E8, 'palette-upper': 0x11F868,
           'rm-destination': 0x1534C, 'palette-init-flag': 0x996B0}
WINDOWS = [
    ('palette-update', 637600, 640200),
    ('palette-init-and-copy', 509600, 511200),
    ('resource-wrapper', 562700, 563350),
    ('adjacent-palette-helper', 510000, 510528),
]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ovl', type=pathlib.Path)
    parser.add_argument('--out', type=pathlib.Path, default=pathlib.Path('m203-output'))
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    valid = len(data) == SIZE and digest == SHA and data[HEADER:HEADER+2] == b'LE'
    print(f'RM_M203_INPUT bytes={len(data)} sha256={digest} knownMatch={int(valid)}')
    if not valid:
        print('RM_M203_ERROR unexpectedExecutable=1 noFilesWritten=1')
        return 2
    h = HEADER
    page_size = struct.unpack_from('<I', data, h+0x28)[0]
    objrel, count, maprel = struct.unpack_from('<III', data, h+0x40)
    pages_start = struct.unpack_from('<I', data, h+0x80)[0]
    if (page_size, count, pages_start) != (4096, 2, 273920):
        print('RM_M203_ERROR unexpectedLELayout=1 noFilesWritten=1')
        return 2
    mapped = []
    for i in range(count):
        size, base, _, first, pages = struct.unpack_from('<IIIII', data, h+objrel+i*24)
        print(f'RM_M203_OBJECT index={i+1} size={size} base=0x{base:08x} firstPage={first} pages={pages}')
        for j in range(pages):
            raw = data[h+maprel+(first+j-1)*4:h+maprel+(first+j)*4]
            page = int.from_bytes(raw[:3], 'big')
            if not page: continue
            phys = pages_start + (page-1)*page_size
            usable = min(page_size, size-j*page_size, len(data)-phys)
            if usable > 0: mapped.append((phys, usable, i+1, j*page_size))
    def locate(physical):
        for p, length, obj, offset in mapped:
            if p <= physical < p+length: return (obj, offset+physical-p)
        return None
    print(f'RM_M203_TARGET name=palette-update physical={TARGET} mapped={locate(TARGET)}')
    args.out.mkdir(parents=True, exist_ok=True)
    manifest = []
    def export(name, start, end):
        chunk = data[start:end]
        filename = f'm203-{name}-{start}.bin'
        (args.out/filename).write_bytes(chunk)
        manifest.append(f'{filename} physicalStart={start} end={end} sha256={hashlib.sha256(chunk).hexdigest()}')
        return filename
    for name, start, end in WINDOWS:
        filename = export(name, start, end)
        print(f'RM_M203_WINDOW name={name} start={start} end={end} bytes={end-start} file={filename}')
    calls = []
    for phys, usable, obj, offset in mapped:
        chunk = data[phys:phys+usable]
        for pos in range(max(0,usable-4)):
            if chunk[pos] != 0xE8: continue
            displacement = struct.unpack_from('<i', chunk, pos+1)[0]
            site = phys+pos
            if site+5+displacement == TARGET:
                calls.append((site,obj,offset+pos))
    calls.sort()
    print(f'RM_M203_CALL_COUNT target={TARGET} candidates={len(calls)}')
    for site, obj, offset in calls:
        print(f'RM_M203_CALL site={site} object={obj} objectOffset=0x{offset:x} bytes={data[site:site+5].hex()}')
        export('call-palette-update', max(0,site-192), min(len(data),site+256))
    for name, value in BUFFERS.items():
        needle = struct.pack('<I',value)
        hits=[]
        for phys, usable, obj, offset in mapped:
            chunk=data[phys:phys+usable]
            pos=chunk.find(needle)
            while pos>=0:
                hits.append((phys+pos,obj,offset+pos))
                pos=chunk.find(needle,pos+1)
        print(f'RM_M203_BUFFER name={name} address=0x{value:x} rawDwordMatches={len(hits)}')
        for site,obj,offset in hits[:60]:
            print(f'RM_M203_REF name={name} site={site} object={obj} objectOffset=0x{offset:x}')
            export('ref-'+name,max(0,site-96),min(len(data),site+128))
        if len(hits)>60:
            print(f'RM_M203_REF_TRUNCATED name={name} omitted={len(hits)-60}')
    (args.out/'m203-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
    print(f'RM_M203_OUTPUT directory={args.out} files={len(manifest)+1}')
    print('RM_M203_LIMITATION staticOnly=1 rawDwordRefsOnly=1 instructionBoundariesUnverified=1 fixupsUnapplied=1 paletteSemanticsUnverified=1')
    return 0

if __name__ == '__main__':
    sys.exit(main())
