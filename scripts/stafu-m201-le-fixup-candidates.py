#!/usr/bin/env python3
"""STAFU M201: conservative LE fixup-target survey (read-only game inputs).

Finds encoded target-offset candidates in the original LE fixup records.
Does NOT claim that byte matches are decoded fixup records or code xrefs.
"""
import hashlib
import pathlib
import struct
import sys

SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
HEADER = 229780
TARGETS = {
    'STANDARD.PAL': [1383596, 1405122, 1430922, 1431078],
    'BRIDGE.RM': [1407391, 1408226],
    'VIEWSCR.RM': [1408760],
}

def u32(b, off):
    return struct.unpack_from('<I', b, off)[0]

def main():
    if len(sys.argv) != 2:
        sys.exit('Usage: python stafu-m201-le-fixup-candidates.py games/STTNG/STTNG.OVL')
    path = pathlib.Path(sys.argv[1]); b = path.read_bytes()
    sha = hashlib.sha256(b).hexdigest()
    ok = len(b) == 2008455 and sha == SHA and b[HEADER:HEADER+2] == b'LE'
    print(f'RM_M201_INPUT bytes={len(b)} sha256={sha} knownMatch={int(ok)}')
    if not ok: sys.exit('ERROR: unexpected executable; no files written')
    h = HEADER
    size = u32(b,h+0x28); objrel=u32(b,h+0x40); count=u32(b,h+0x44)
    maprel=u32(b,h+0x48); fprel=u32(b,h+0x68); frrel=u32(b,h+0x6c)
    dataoff=u32(b,h+0x80)
    if (size,count,fprel,frrel) != (4096,2,1738,3226):
        sys.exit('ERROR: unexpected LE layout; no files written')
    objects=[]
    for i in range(count):
        objsize,base,flags,first,npage=struct.unpack_from('<IIIII',b,h+objrel+i*24)
        objects.append((objsize,base,first,npage))
        print(f'RM_M201_OBJECT index={i+1} size={objsize} base=0x{base:08x} firstPage={first} pages={npage}')
    pages=sum(o[3] for o in objects)
    fp=h+fprel; fr=h+frrel
    if fp+(pages+1)*4>len(b): sys.exit('ERROR: fixup page table truncated')
    ends=[u32(b,fp+i*4) for i in range(pages+1)]
    if ends!=sorted(ends) or fr+ends[-1]>len(b):
        sys.exit('ERROR: fixup page offsets invalid')
    print(f'RM_M201_FIXUP_TABLE logicalPages={pages} recordBytes={ends[-1]} nonemptyPages={sum(ends[i]<ends[i+1] for i in range(pages))}')
    out=pathlib.Path('m201-output');out.mkdir(exist_ok=True)
    (out/'m201-fixup-page-table.bin').write_bytes(b[fp:fp+(pages+1)*4])
    (out/'m201-fixup-records.bin').write_bytes(b[fr:fr+ends[-1]])
    manifest=[]
    for name,physical_list in TARGETS.items():
        for physical in physical_list:
            # Object 2's LE data is page-mapped; the physical page index is not
            # necessarily the logical page index. Invert the page map explicitly.
            physical_page=(physical-dataoff)//size+1
            within=(physical-dataoff)%size
            matches=[]
            for oi,(objsize,base,first,npage) in enumerate(objects):
                for j in range(npage):
                    raw=b[h+maprel+(first-1+j)*4:h+maprel+(first+j)*4]
                    if len(raw)!=4: continue
                    mapped=int.from_bytes(raw[:3],'big')
                    if mapped==physical_page and raw[3]==0:
                        offset=j*size+within
                        if offset<objsize: matches.append((oi+1,base,offset))
            print(f'RM_M201_TARGET name={name} physical={physical} physicalPage={physical_page} maps={len(matches)}')
            for obj,base,off in matches:
                print(f'RM_M201_TARGET_MAP name={name} physical={physical} object={obj} objectOffset=0x{off:08x} linear=0x{base+off:08x}')
                # A four-byte match is a candidate only. Target offsets can be
                # encoded differently, and arbitrary matches are possible.
                needle=struct.pack('<I',off)
                total=0
                for pg in range(pages):
                    a,z=ends[pg],ends[pg+1]
                    chunk=b[fr+a:fr+z]
                    pos=chunk.find(needle)
                    while pos>=0:
                        total+=1
                        rec_off=a+pos
                        context=b[fr+max(a,rec_off-12):fr+min(z,rec_off+12)]
                        logical_obj=1 if pg<objects[0][3] else 2
                        logical_local=pg if logical_obj==1 else pg-objects[0][3]
                        label=f'm201-{name.replace(".","_")}-{physical}-page{pg+1}-rec{rec_off}'
                        (out/(label+'.bin')).write_bytes(b[fr+a:fr+z])
                        manifest.append(f'{label} targetObject={obj} targetOffset=0x{off:08x} logicalPage={pg+1} sourceObject={logical_obj} sourceObjectPage={logical_local+1} recordOffset={rec_off} context={context.hex()}')
                        print(f'RM_M201_CANDIDATE name={name} physical={physical} targetObject={obj} sourceObject={logical_obj} sourcePage={logical_local+1} logicalPage={pg+1} recordOffset={rec_off} context={context.hex()}')
                        pos=chunk.find(needle,pos+1)
                print(f'RM_M201_CANDIDATE_COUNT name={name} physical={physical} targetObject={obj} exactOffset32Matches={total}')
    (out/'m201-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
    print(f'RM_M201_OUTPUT directory={out} candidateRecords={len(manifest)}')
    print('RM_M201_LIMITATION rawTargetMatchesOnly=1 fixupRecordsNotDecoded=1 relocationsNotApplied=1 runtimePalettePathUnverified=1')

if __name__=='__main__':
    main()
