#!/usr/bin/env python3
"""STAFU M202: read-only survey of direct callers and copy arguments.

A raw E8 match is a candidate, not proof of an instruction boundary.
The script does not execute the game or infer runtime palette provenance.
"""
import argparse
import hashlib
import pathlib
import struct
import sys

SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
HEADER = 229780
ENTRIES = {'rm-copy': 510528, 'resource-wrapper': 562864,
           'palette-helper': 510232, 'inner-loader': 513376}

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('ovl', type=pathlib.Path)
    ap.add_argument('--out', type=pathlib.Path, default=pathlib.Path('m202-output'))
    args = ap.parse_args()
    b = args.ovl.read_bytes()
    sha = hashlib.sha256(b).hexdigest()
    valid = len(b) == 2008455 and sha == SHA and b[HEADER:HEADER+2] == b'LE'
    print(f'RM_M202_INPUT bytes={len(b)} sha256={sha} knownMatch={int(valid)}')
    if not valid:
        print('RM_M202_ERROR unexpectedExecutable=1 noFilesWritten=1')
        return 2
    h = HEADER
    page_size = struct.unpack_from('<I', b, h+0x28)[0]
    objrel, nobj, maprel = struct.unpack_from('<III', b, h+0x40)
    pages_start = struct.unpack_from('<I', b, h+0x80)[0]
    if (page_size, nobj, pages_start) != (4096, 2, 273920):
        print('RM_M202_ERROR unexpectedLELayout=1'); return 2
    objects=[]
    for i in range(nobj):
        size,base,flags,first,count=struct.unpack_from('<IIIII',b,h+objrel+i*24)
        objects.append((size,base,first,count))
        print(f'RM_M202_OBJECT index={i+1} size={size} base=0x{base:08x} firstPage={first} pages={count}')
    # Physical-file-offset -> (object, object offset). No runtime fixups applied.
    mapping={}
    for oi,(size,base,first,count) in enumerate(objects,1):
        for j in range(count):
            m=b[h+maprel+(first+j-1)*4:h+maprel+(first+j)*4]
            if len(m)!=4: continue
            phys_page=int.from_bytes(m[:3], 'big')
            if not phys_page: continue
            physical=pages_start+(phys_page-1)*page_size
            usable=min(page_size,size-j*page_size,len(b)-physical)
            if usable<=0: continue
            mapping[physical]=(oi,j*page_size,usable)
    def locate(p):
        page=pages_start+((p-pages_start)//page_size)*page_size
        entry=mapping.get(page)
        if entry is None or p-page>=entry[2]: return None
        return entry[0],entry[1]+p-page
    args.out.mkdir(parents=True,exist_ok=True)
    manifest=[]
    for name,target in ENTRIES.items():
        print(f'RM_M202_TARGET name={name} physical={target} mapped={locate(target)}')
        start=max(0,target-256); end=min(len(b),target+1280)
        fn=f'm202-target-{name}-{start}.bin'
        (args.out/fn).write_bytes(b[start:end])
        manifest.append(f'{fn} physicalStart={start} entryPhysical={target}')
        # Search each mapped object page for direct relative E8 calls whose
        # 32-bit displacement resolves to the target physical file offset.
        candidates=[]
        for physical,(obj,offset,usable) in mapping.items():
            chunk=b[physical:physical+usable]
            i=chunk.find(b'\xe8')
            while i>=0:
                if i+5<=len(chunk):
                    disp=struct.unpack_from('<i',chunk,i+1)[0]
                    site=physical+i
                    resolved=site+5+disp
                    if resolved==target:
                        candidates.append((site,obj,offset+i,'physical-relative'))
                    # LE objects may be non-contiguous in physical file order.
                    # Compare object-relative addresses as an independent test.
                    here=locate(site); dest=locate(target)
                    if here and dest and here[0]==dest[0] and here[1]+5+disp==dest[1] and resolved!=target:
                        candidates.append((site,obj,offset+i,'object-relative'))
                i=chunk.find(b'\xe8',i+1)
        candidates.sort()
        print(f'RM_M202_CALL_COUNT name={name} candidates={len(candidates)}')
        for ix,(site,obj,off,kind) in enumerate(candidates):
            print(f'RM_M202_CALL name={name} site={site} object={obj} objectOffset=0x{off:x} kind={kind} bytes={b[site:site+5].hex()}')
            if ix<60:
                s=max(0,site-192); e=min(len(b),site+256)
                fn=f'm202-call-{name}-{site}.bin'
                (args.out/fn).write_bytes(b[s:e]); manifest.append(f'{fn} physicalStart={s} callPhysical={site} target={target} kind={kind}')
    # Include entire known copy function, including its exit path.
    for name,start,end in [('copy-and-flags',510400,511800),
                           ('copy-predecessors',509600,510528),
                           ('wrapper',562700,563350)]:
        fn=f'm202-context-{name}-{start}.bin'
        (args.out/fn).write_bytes(b[start:end]);manifest.append(f'{fn} physicalStart={start} end={end}')
        print(f'RM_M202_CONTEXT name={name} start={start} end={end} bytes={end-start}')
    (args.out/'m202-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
    print(f'RM_M202_OUTPUT directory={args.out} files={len(manifest)+1}')
    print('RM_M202_LIMITATION directE8CandidatesOnly=1 instructionBoundariesUnverified=1 relocationsUnapplied=1 runtimeArgumentsUnverified=1 paletteProvenanceUnverified=1')
    return 0

if __name__ == '__main__':
    sys.exit(main())
