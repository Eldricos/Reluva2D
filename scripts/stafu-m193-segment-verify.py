#!/usr/bin/env python3
"""STAFU M193: read-only verification of the CS-relative dispatcher jump table."""
import hashlib
import pathlib
import struct
import sys

SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
TABLE_PHYS = 513928
DISPATCH_PHYS = 513964
JUMP_PHYS = 513987
TABLE_OPERAND = 0x56E4
EXPECTED = (0x5727, 0x57B4, 0x57B4, 0x57B4, 0x57B4, 0x57B4, 0x57B4, 0x57AF, 0x5773)

def fail(message):
    raise SystemExit('ERROR: ' + message)

def u32(b, at):
    return struct.unpack_from('<I', b, at)[0]

def main():
    if len(sys.argv) != 2:
        fail('usage: python stafu-m193-segment-verify.py games/STTNG/STTNG.OVL')
    p = pathlib.Path(sys.argv[1]); b = p.read_bytes()
    digest = hashlib.sha256(b).hexdigest()
    print(f'RM_M193_INPUT path={p} bytes={len(b)} sha256={digest} knownShaMatch={int(digest==SHA)}')
    if digest != SHA: fail('unexpected OVL SHA; no output written')
    if b[JUMP_PHYS:JUMP_PHYS+8] != bytes.fromhex('2eff2485e4560000'):
        fail('dispatcher jump instruction mismatch')
    if u32(b,JUMP_PHYS+4) != TABLE_OPERAND: fail('jump operand mismatch')
    entries = struct.unpack_from('<9I', b, TABLE_PHYS)
    if entries != EXPECTED: fail('adjacent table mismatch')
    # LE page-map: 3-byte big-endian page number and flags byte.
    le = 229780
    if b[le:le+2] != b'LE': fail('LE signature mismatch')
    page_size = u32(b,le+0x28)
    object_rel = u32(b,le+0x40)
    page_rel = u32(b,le+0x48)
    data_offset = u32(b,le+0x80)
    if page_size != 4096: fail('unexpected page size')
    obj = le+object_rel
    obj_size, obj_base, obj_flags, first_page, page_count = struct.unpack_from('<IIIII',b,obj)
    def inverse(physical):
        if physical < data_offset: fail('physical address precedes data pages')
        file_page, within = divmod(physical-data_offset,page_size)
        page_number=file_page+1
        for i in range(page_count):
            m=le+page_rel+(first_page-1+i)*4
            raw=b[m:m+4]
            if int.from_bytes(raw[:3],'big') == page_number and raw[3] == 0:
                return i*page_size+within
        fail(f'object1 mapping missing for physical={physical}')
    table_obj=inverse(TABLE_PHYS)
    dispatch_obj=inverse(DISPATCH_PHYS)
    segment_origin=table_obj-TABLE_OPERAND
    dispatch_cs=dispatch_obj-segment_origin
    print(f'RM_M193_MAPPING object=1 base=0x{obj_base:x} tablePhysical={TABLE_PHYS} tableObject=0x{table_obj:x} dispatcherPhysical={DISPATCH_PHYS} dispatcherObject=0x{dispatch_obj:x}')
    print(f'RM_M193_SEGMENT originObject=0x{segment_origin:x} dispatcherCS=0x{dispatch_cs:x} tableCS=0x{TABLE_OPERAND:x} tableMatches=1')
    # The table and all four unique targets must lie within the same LE page here.
    for mode, cs_target in enumerate(entries):
        obj_target=segment_origin+cs_target
        physical_target=TABLE_PHYS+(obj_target-table_obj)
        if inverse(physical_target) != obj_target:
            fail(f'noncontiguous page mapping for mode {mode}')
        code=b[physical_target:physical_target+8]
        print(f'RM_M193_MODE mode={mode} csTarget=0x{cs_target:04x} objectTarget=0x{obj_target:x} physical={physical_target} first8={code.hex()}')
    if b[513995:513997] != bytes.fromhex('b601'): fail('mode0 handler mismatch')
    if b[514071:514073] != bytes.fromhex('b401'): fail('mode8 handler mismatch')
    if b[514131:514136] != bytes.fromhex('bd01000000'): fail('error handler mismatch')
    if b[514136:514138] != bytes.fromhex('89e8'): fail('success return handler mismatch')
    print('RM_M193_HANDLER_CHECK mode0=1 mode8=1 invalid=1 successReturn=1')
    print('RM_M193_LIMITATION segmentOriginDerivedFromTable=1 runtimeSegmentDescriptorUnverified=1 processingAlgorithmsUnverified=1')

if __name__ == '__main__': main()
