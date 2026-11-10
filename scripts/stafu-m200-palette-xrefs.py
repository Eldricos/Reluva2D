#!/usr/bin/env python3
"""STAFU M200: read-only palette/resource string and static pointer-reference survey.

This is a discovery pass, not a reconstruction of palette initialization.
Writes only to --out, never modifies game resources.
"""
import argparse
import hashlib
import pathlib
import re
import struct
import sys

EXPECTED_SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
TERMS = (b'STANDARD.PAL', b'BRIDGE.RM', b'VIEWSCR.RM', b'bridge.cpp', b'viewscr.cpp', b'.PAL', b'palette')

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('ovl', type=pathlib.Path)
    p.add_argument('--out', type=pathlib.Path, default=pathlib.Path('m200-output'))
    args = p.parse_args()
    data = args.ovl.read_bytes()
    sha = hashlib.sha256(data).hexdigest()
    ok = len(data) == 2008455 and sha == EXPECTED_SHA
    print(f'RM_M200_INPUT bytes={len(data)} sha256={sha} knownMatch={int(ok)}')
    if not ok:
        print('RM_M200_ERROR unexpectedOVL=1 noFilesWritten=1')
        return 2
    hdr = 229780
    if data[hdr:hdr+2] != b'LE':
        print('RM_M200_ERROR invalidLEHeader=1'); return 2
    page_size = struct.unpack_from('<I',data,hdr+0x28)[0]
    obj_rel, count, map_rel = struct.unpack_from('<III',data,hdr+0x40)
    pages_start = struct.unpack_from('<I',data,hdr+0x80)[0]
    if page_size != 4096 or count != 2:
        print('RM_M200_ERROR unexpectedLELayout=1'); return 2
    objects=[]
    for i in range(count):
        pos=hdr+obj_rel+i*24
        size, base, flags, first, n=struct.unpack_from('<IIIII',data,pos)
        objects.append((size,base,first,n))
        print(f'RM_M200_OBJECT index={i+1} size={size} base=0x{base:08x} firstPage={first} pages={n}')
    # Build inverse map for raw physical bytes, with no fixups applied.
    phys_to_va={}
    for i,(size,base,first,n) in enumerate(objects):
        for j in range(n):
            mo=hdr+map_rel+(first-1+j)*4
            raw=data[mo:mo+4]
            if len(raw)!=4: continue
            page=int.from_bytes(raw[:3],'big')
            if not page: continue
            physical=pages_start+(page-1)*page_size
            if physical>=len(data): continue
            for k in range(0,min(page_size,size-j*page_size),4):
                phys_to_va[physical+k]=base+j*page_size+k
    # Find actual strings before attempting reference analysis.
    matches=[]
    for term in TERMS:
        pat=re.compile(re.escape(term),re.IGNORECASE)
        hits=[m.start() for m in pat.finditer(data)]
        print(f'RM_M200_STRING term={term.decode("ascii")} count={len(hits)} offsets={",".join(map(str,hits[:25])) or "none"} truncated={int(len(hits)>25)}')
        for off in hits[:50]:
            matches.append((term,off))
    args.out.mkdir(parents=True,exist_ok=True)
    manifest=[]
    # Search exact dword references to mapped string virtual addresses; these are
    # only candidate references because LE fixups may replace encoded operands.
    for term,off in matches:
        aligned=off&~3
        va=phys_to_va.get(aligned)
        if va is not None and off!=aligned: va+=off-aligned
        if va is None:
            print(f'RM_M200_LOCATION term={term.decode()} physical={off} mappedVA=none')
            continue
        print(f'RM_M200_LOCATION term={term.decode()} physical={off} mappedVA=0x{va:08x}')
        needle=struct.pack('<I',va)
        refs=[m.start() for m in re.finditer(re.escape(needle),data)]
        print(f'RM_M200_XREF term={term.decode()} stringPhysical={off} candidateDwordRefs={len(refs)} offsets={",".join(map(str,refs[:25])) or "none"} truncated={int(len(refs)>25)} fixupsUnapplied=1')
        for ref in refs[:8]:
            start=max(0,ref-128); end=min(len(data),ref+128)
            name=f'm200-ref-{off}-{ref}.bin'
            (args.out/name).write_bytes(data[start:end])
            manifest.append(f'{name} physicalStart={start} referencePhysical={ref} term={term.decode()}')
    # Preserve source-string context and established loader entry points.
    for term,off in matches[:30]:
        start=max(0,off-128);end=min(len(data),off+192)
        name=f'm200-string-{off}.bin'
        (args.out/name).write_bytes(data[start:end])
        manifest.append(f'{name} physicalStart={start} stringPhysical={off} term={term.decode()}')
    for name,offset in [('resource-lookup',516148),('inner-loader',513376),('dispatcher',513964),('rm-copy',510528)]:
        start=max(0,offset-128);end=min(len(data),offset+384)
        fn=f'm200-{name}-{start}.bin'
        (args.out/fn).write_bytes(data[start:end]); manifest.append(f'{fn} physicalStart={start} entryPhysical={offset}')
        print(f'RM_M200_ANCHOR name={name} entry={offset} file={fn}')
    (args.out/'m200-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
    print(f'RM_M200_OUTPUT directory={args.out} files={len(manifest)+1}')
    print('RM_M200_LIMITATION staticOnly=1 fixupsUnapplied=1 xrefsMayBeMissing=1 paletteAssemblyUnverified=1')
    return 0

if __name__=='__main__':
    sys.exit(main())
