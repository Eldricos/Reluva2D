#!/usr/bin/env python3
"""STAFU M191: read-only LE address/page mapping and dispatcher table diagnostics."""
import hashlib
import pathlib
import struct
import sys

SHA = '65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
DISPATCH = 513964
TABLE_OPERAND = 0x56e4
CALL = bytes.fromhex('2eff2485e4560000')

def u32(buf, pos):
    if pos < 0 or pos + 4 > len(buf):
        raise ValueError(f'out-of-bounds u32 at {pos}')
    return struct.unpack_from('<I', buf, pos)[0]

def save(buf, start, end, name):
    if not (0 <= start < end <= len(buf)):
        print(f'RM_M191_WINDOW name={name} extractable=0')
        return
    path = pathlib.Path(f'm191-{name}-{start}.bin')
    part = buf[start:end]
    path.write_bytes(part)
    print(f'RM_M191_WINDOW name={name} from={start} to={end} bytes={len(part)} file={path.name} sha256={hashlib.sha256(part).hexdigest()}')

if len(sys.argv) != 2:
    sys.exit('Usage: python stafu-m191-address-map.py games/STTNG/STTNG.OVL')
p = pathlib.Path(sys.argv[1])
b = p.read_bytes()
hash_value = hashlib.sha256(b).hexdigest()
print(f'RM_M191_INPUT path={p} bytes={len(b)} sha256={hash_value} knownShaMatch={int(hash_value == SHA)}')
if hash_value != SHA:
    sys.exit('ERROR: unexpected executable; no output written')
if b[DISPATCH+27:DISPATCH+35] != CALL:
    # Dispatcher jmp is at 513987 = 513964 + 23, not +27.
    if b[DISPATCH+23:DISPATCH+31] != CALL:
        sys.exit('ERROR: dispatcher indirect jump signature mismatch; no output written')
print(f'RM_M191_DISPATCH offset={DISPATCH} jumpSite={DISPATCH+23} jumpBytes={b[DISPATCH+23:DISPATCH+31].hex()} operand=0x{TABLE_OPERAND:x}')

h = 229780
if b[h:h+2] != b'LE':
    sys.exit('ERROR: LE header signature mismatch')
page_size = u32(b, h+0x28)
objrel = u32(b, h+0x40)
objcount = u32(b, h+0x44)
maprel = u32(b, h+0x48)
dataoff = u32(b, h+0x80)
fixup_page_rel = u32(b, h+0x68)
fixup_record_rel = u32(b, h+0x6c)
print(f'RM_M191_LE header={h} pageSize={page_size} objects={objcount} objectTableRel={objrel} pageMapRel={maprel} dataPagesOffset={dataoff} fixupPageRel={fixup_page_rel} fixupRecordRel={fixup_record_rel}')
if page_size != 4096 or objcount != 2:
    sys.exit('ERROR: unexpected LE metadata')

# The executable's physical bytes around the dispatcher show candidate pointer data.
# Record the full 36 bytes immediately before the function, without presuming its virtual address.
near = DISPATCH - 36
vals = struct.unpack_from('<9I', b, near)
print(f'RM_M191_ADJACENT physical={near} entries=' + ','.join(f'0x{x:08x}' for x in vals))
save(b, DISPATCH-160, DISPATCH+256, 'dispatcher-neighborhood')

# Compare the page mapping for each object's virtual offset 0x56e4 with the
# physical page containing the dispatcher.  Also invert all object page mappings.
physical_page = (DISPATCH - dataoff)//page_size + 1
physical_within = (DISPATCH - dataoff)%page_size
print(f'RM_M191_PHYSICAL_PAGE dispatcher={DISPATCH} candidatePage={physical_page} within={physical_within}')
for i in range(objcount):
    off = h + objrel + i*24
    size, base, flags, first, count = struct.unpack_from('<IIIII', b, off)
    print(f'RM_M191_OBJECT index={i+1} size={size} base=0x{base:x} flags=0x{flags:x} firstPage={first} pageCount={count}')
    matches=[]
    for j in range(count):
        mo = h+maprel+(first-1+j)*4
        raw=b[mo:mo+4]
        if len(raw)!=4:
            sys.exit('ERROR: truncated page map')
        number=int.from_bytes(raw[:3], 'big')
        if number==physical_page:
            matches.append((j,mo,raw.hex(),j*page_size+physical_within))
    for j,mo,raw,virt in matches:
        print(f'RM_M191_INVERSE index={i+1} objectPage={j+1} mapOffset={mo} entry={raw} dispatcherObjectOffset=0x{virt:x} dispatcherLinear=0x{base+virt:x}')
    if not matches:
        print(f'RM_M191_INVERSE index={i+1} matchingPage=none')
    j=TABLE_OPERAND//page_size
    if j<count and TABLE_OPERAND+36<=size:
        mo=h+maprel+(first-1+j)*4
        raw=b[mo:mo+4]
        number=int.from_bytes(raw[:3],'big')
        flags_byte=raw[3]
        physical=dataoff+(number-1)*page_size+(TABLE_OPERAND%page_size) if number else -1
        print(f'RM_M191_OPERAND index={i+1} objectOffset=0x{TABLE_OPERAND:x} mapOffset={mo} entry={raw.hex()} page={number} pageFlags={flags_byte} physical={physical}')
        if number and flags_byte==0 and 0<=physical<=len(b)-36:
            print('RM_M191_OPERAND_ENTRIES index={} entries={}'.format(i+1, ','.join(f'0x{x:08x}' for x in struct.unpack_from('<9I',b,physical))))
            save(b,max(0,physical-64),min(len(b),physical+128),f'operand-object{i+1}')

# Inspect fixup metadata around pages of interest without assuming records
# have already been applied to the file image.
for name,rel in [('fixup-page',fixup_page_rel),('fixup-record',fixup_record_rel)]:
    start=h+rel
    if 0<=start<len(b):
        save(b,start,min(len(b),start+256),name)
print('RM_M191_LIMITATION runtimeRelocationsUnapplied=1 handlerTargetsUnverified=1 pageMappingInterpretationNeedsValidation=1')
