#!/usr/bin/env python3
"""STAFU M190: read-only LE object/page-map and processing jump-table investigation.
No modification to STTNG.OVL. All mappings are candidates until object/segment identity is established.
"""
import hashlib, pathlib, struct, sys
SHA='65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
LE=229780
DISPATCH=513964
TABLE_OFFSET=0x56e4

def u16(b,p): return struct.unpack_from('<H',b,p)[0]
def u32(b,p): return struct.unpack_from('<I',b,p)[0]
def safe(b,p,n): return 0 <= p <= len(b)-n

def main():
    if len(sys.argv) not in (2,3):
        print('Usage: python stafu-m190-jumptable.py <STTNG.OVL> [output-folder]');return 2
    src=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]) if len(sys.argv)==3 else pathlib.Path('.')
    b=src.read_bytes(); sha=hashlib.sha256(b).hexdigest()
    print(f'RM_M190_INPUT path={src} bytes={len(b)} sha256={sha} knownShaMatch={int(sha==SHA)}')
    expected=bytes.fromhex('83 f8 08 0f 87 90 00 00 00 2e ff 24 85 e4 56 00 00')
    actual=b[513978:513978+len(expected)]
    print(f'RM_M190_DISPATCH offset=513978 bytes={actual.hex()} expectedMatch={int(actual==expected)} tableOperand=0x{TABLE_OFFSET:x}')
    if sha!=SHA or not safe(b,LE,0xb0) or b[LE:LE+2]!=b'LE':
        print('RM_M190_ERROR invalid executable or LE header; no files written');return 1
    if actual!=expected:
        print('RM_M190_ERROR dispatcher signature mismatch; no files written');return 1
    page_size=u32(b,LE+0x28);page_shift=u32(b,LE+0x2c)
    obj_rel=u32(b,LE+0x40);obj_count=u32(b,LE+0x44)
    page_rel=u32(b,LE+0x48);data_off=u32(b,LE+0x80)
    print(f'RM_M190_LE header={LE} pageSize={page_size} pageShift={page_shift} objectTableRel={obj_rel} objectCount={obj_count} pageMapRel={page_rel} dataPagesOffset={data_off}')
    if not (0<obj_count<128 and 0<page_size<=65536 and safe(b,LE+obj_rel,obj_count*24)):
        print('RM_M190_ERROR implausible LE object metadata');return 1
    out.mkdir(parents=True,exist_ok=True)
    code_start=DISPATCH-32;code_end=DISPATCH+768
    code=b[code_start:code_end];(out/f'm190-dispatch-{code_start}.bin').write_bytes(code)
    print(f'RM_M190_WINDOW name=dispatch from={code_start} to={code_end} file=m190-dispatch-{code_start}.bin sha256={hashlib.sha256(code).hexdigest()}')
    for i in range(obj_count):
        p=LE+obj_rel+i*24
        size,base,flags,page_index,page_count,reserved=struct.unpack_from('<6I',b,p)
        print(f'RM_M190_OBJECT index={i+1} size={size} base=0x{base:x} flags=0x{flags:x} firstPage={page_index} pageCount={page_count}')
        if TABLE_OFFSET+36>size or not page_index: continue
        page_no=TABLE_OFFSET//page_size;in_page=TABLE_OFFSET%page_size
        if in_page+36>page_size:
            print(f'RM_M190_CANDIDATE object={i+1} crossesPage=1');continue
        mp=LE+page_rel+(page_index-1+page_no)*4
        if not safe(b,mp,4):continue
        entry=u32(b,mp);physical_page=entry&0xffffff;flags_byte=entry>>24
        # LE page number is 1-based. For LE, data pages start at absolute file offset.
        physical=data_off+(physical_page-1)*page_size+in_page if physical_page else -1
        print(f'RM_M190_CANDIDATE object={i+1} pageMapOffset={mp} pageEntry=0x{entry:08x} physicalPage={physical_page} pageFlags={flags_byte} physical={physical}')
        if physical_page and flags_byte==0 and safe(b,physical,36):
            raw=b[physical:physical+36];name=f'm190-table-object{i+1}-{physical}.bin'
            (out/name).write_bytes(raw)
            print(f'RM_M190_TABLE object={i+1} file={name} raw={raw.hex()} values='+','.join(f'0x{u32(raw,j*4):08x}' for j in range(9)))
        else:print(f'RM_M190_TABLE object={i+1} extractable=0 reason=nonstandardPageOrOutOfRange')
    print('RM_M190_LIMITATION tableObjectUnverified=1 runtimeRelocationsUnapplied=1 handlerSemanticsUnverified=1')
    return 0
if __name__=='__main__':raise SystemExit(main())
