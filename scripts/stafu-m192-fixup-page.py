#!/usr/bin/env python3
"""STAFU M192: read-only targeted LE fixup and jump-table evidence extraction."""
import hashlib, pathlib, struct, sys
SHA='65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
if len(sys.argv)!=2: sys.exit('Usage: python stafu-m192-fixup-page.py games/STTNG/STTNG.OVL')
p=pathlib.Path(sys.argv[1]); b=p.read_bytes(); digest=hashlib.sha256(b).hexdigest()
print(f'RM_M192_INPUT bytes={len(b)} sha256={digest} knownShaMatch={int(digest==SHA)}')
if digest!=SHA: sys.exit('ERROR: SHA mismatch; no output written')
H=229780
u32=lambda o:struct.unpack_from('<I',b,o)[0]
page_size=u32(H+0x28); object_rel=u32(H+0x40); page_rel=u32(H+0x48); fixpage_rel=u32(H+0x68); fixrecord_rel=u32(H+0x6c); data_off=u32(H+0x80)
print(f'RM_M192_LE pageSize={page_size} objectRel={object_rel} pageRel={page_rel} fixupPageRel={fixpage_rel} fixupRecordRel={fixrecord_rel} dataOffset={data_off}')
if page_size!=4096 or fixpage_rel!=1738 or fixrecord_rel!=3226:sys.exit('ERROR: LE layout unexpected')
# Object 1's physical page 59 is object page 59; LE fixup-page table is indexed by logical page, zero-based.
fp=H+fixpage_rel; fr=H+fixrecord_rel
for idx in (57,58,59,60):
 a=u32(fp+idx*4); z=u32(fp+(idx+1)*4)
 print(f'RM_M192_FIXUP_PAGE index0={idx} logicalPage={idx+1} start={a} end={z} countBytes={z-a}')
 if z<a or z-a>100000 or fr+z>len(b):
  print(f'RM_M192_FIXUP_PAGE_ERROR index0={idx}');continue
 chunk=b[fr+a:fr+z]; name=f'm192-fixups-page-{idx+1}.bin';pathlib.Path(name).write_bytes(chunk)
 print(f'RM_M192_FIXUP_RECORDS logicalPage={idx+1} file={name} bytes={len(chunk)} sha256={hashlib.sha256(chunk).hexdigest()} hexPrefix={chunk[:96].hex()}')
# Adjacent table, 36 bytes immediately before dispatcher.
for label,lo,hi in [('adjacent',513928,513964),('dispatch',513900,514220),('fixup-metadata',fp,fp+320)]:
 chunk=b[lo:hi];name=f'm192-{label}-{lo}.bin';pathlib.Path(name).write_bytes(chunk)
 print(f'RM_M192_WINDOW name={label} from={lo} to={hi} file={name} sha256={hashlib.sha256(chunk).hexdigest()}')
 if label=='adjacent':print('RM_M192_ADJACENT entries='+','.join(f'0x{x:08x}' for x in struct.unpack('<9I',chunk)))
print('RM_M192_LIMITATION fixupRecordsNotDecoded=1 runtimeSelectorMappingUnverified=1 handlerTargetsUnverified=1')
