#!/usr/bin/env python3
"""M350: corrected LE page mapping and resource jump-table fixup inspection.

Read-only diagnostic. LE page-map page numbers are 24-bit big-endian.
Relocation records are inspected as bytes and conservatively decoded; no
relocations are applied. Requires only Python standard library.
"""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LE = 229780
TABLES = (0x12334, 0x13488)

def word(b, p):
    return struct.unpack_from("<H", b, p)[0] if 0 <= p <= len(b)-2 else None

def dword(b, p):
    return struct.unpack_from("<I", b, p)[0] if 0 <= p <= len(b)-4 else None

def fmt(v):
    return "none" if v is None else f"0x{v:x}"

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    b = args.ovl.read_bytes()
    digest = hashlib.sha256(b).hexdigest()
    print(f"M350_INPUT bytes={len(b)} sha256={digest}")
    if len(b) != EXPECTED_SIZE or digest != EXPECTED_SHA or b[LE:LE+2] != b"LE":
        raise SystemExit("M350_ERROR unexpected executable")
    count = dword(b, LE+0x44)
    objtab = LE+dword(b, LE+0x40)
    pagetab = LE+dword(b, LE+0x48)
    pagesize = dword(b, LE+0x28)
    datapages_field = dword(b, LE+0x80)
    fixpage = LE+dword(b, LE+0x68)
    fixrecord = LE+dword(b, LE+0x6c)
    totalpages = dword(b, LE+0x14)
    # This embedded LE image stores its data-pages offset relative to the LE header.
    # M349's known physical layout places the first data page at 503700.
    datastart = LE + datapages_field
    print(f"M350_LAYOUT objectCount={count} pageSize={pagesize} pageTablePhysical={pagetab} dataPagesHeaderValue={fmt(datapages_field)} dataStartPhysical={datastart} fixupPagePhysical={fixpage} fixupRecordPhysical={fixrecord} totalPages={totalpages}")
    objects = []
    for i in range(min(count, 32)):
        p = objtab+i*24
        size, base, flags, first, npages = (dword(b,p+j*4) for j in range(5))
        o = dict(index=i+1,size=size,base=base,flags=flags,first=first,count=npages)
        objects.append(o)
        print(f"M350_OBJECT index={i+1} size={fmt(size)} base={fmt(base)} flags={fmt(flags)} firstPage={first} pageCount={npages}")
    page_map = {}
    for o in objects:
        for j in range(o["count"]):
            logical = o["first"]+j
            p = pagetab+(logical-1)*4
            raw = b[p:p+4]
            if len(raw) != 4: continue
            number = int.from_bytes(raw[:3], "big")
            flags = raw[3]
            physical = datastart+(number-1)*pagesize if number else None
            page_map[(o["index"],j)] = (physical,flags,number)
            if j<3 or j>=o["count"]-2:
                print(f"M350_PAGE object={o['index']} local={j} logical={logical} raw={raw.hex()} pageNumber={number} flags={flags} physical={fmt(physical)} inFile={int(physical is not None and 0<=physical<len(b))}")
    def map_rva(o,rva):
        if rva < 0 or rva >= o["size"] or pagesize<=0:return None
        j,within=divmod(rva,pagesize)
        item=page_map.get((o["index"],j))
        if not item:return None
        phys,flags,num=item
        if phys is None or flags != 0:return None
        p=phys+within
        return p if 0<=p<len(b) else None
    fix_starts=[]
    if 0 < totalpages < 100000 and fixpage+(totalpages+1)*4 <= len(b):
        fix_starts=[dword(b,fixpage+i*4) for i in range(totalpages+1)]
        print(f"M350_FIXUP_INDEX entries={len(fix_starts)} monotonic={int(all(x<=y for x,y in zip(fix_starts,fix_starts[1:])))}")
    for table in TABLES:
        print(f"M350_TABLE_BEGIN address={fmt(table)}")
        for o in objects:
            for mode,rva in (("absolute_minus_base",table-o["base"]),("object_relative",table)):
                p=map_rva(o,rva)
                print(f"M350_TABLE_LOCATION address={fmt(table)} object={o['index']} mode={mode} rva={fmt(rva)} physical={fmt(p)}")
                if p is None or p+28>len(b):continue
                for state in range(7):
                    offset=rva+state*4
                    entryphys=map_rva(o,offset)
                    raw=dword(b,entryphys) if entryphys is not None else None
                    print(f"M350_TABLE_ENTRY address={fmt(table)} object={o['index']} mode={mode} stateIndex={state} entryPhysical={fmt(entryphys)} raw={fmt(raw)}")
                    if raw is None:continue
                    for target in objects:
                        for targetmode,targetrva in (("absolute_minus_base",raw-target["base"]),("object_relative",raw)):
                            dest=map_rva(target,targetrva)
                            if dest is not None:
                                print(f"M350_RAW_TARGET_CANDIDATE table={fmt(table)} stateIndex={state} targetObject={target['index']} mode={targetmode} targetRva={fmt(targetrva)} physical={dest} unverified=1")
                if not fix_starts:continue
                local=rva//pagesize
                logical=o["first"]+local
                if not 1<=logical<=totalpages:continue
                lo,hi=fix_starts[logical-1:logical+1]
                if lo is None or hi is None or hi<lo or fixrecord+hi>len(b):continue
                rec=b[fixrecord+lo:fixrecord+hi]
                print(f"M350_FIXUP_PAGE table={fmt(table)} object={o['index']} mode={mode} logicalPage={logical} recordBytes={len(rec)} recordPhysical={fixrecord+lo} tableOffsetInPage={rva%pagesize}")
                # LE fixups use variable-width records. Report candidate records
                # starting at the conventional source-type byte 0x07 (32-bit
                # offset), with target flags and source offset, without treating
                # arbitrary bytes as proven record boundaries.
                positions={((rva+4*i)%pagesize):i for i in range(7)}
                for k in range(max(0,len(rec)-3)):
                    if rec[k]!=7:continue
                    flags=rec[k+1]
                    source=word(rec,k+2)
                    if source not in positions:continue
                    state=positions[source]
                    # Context includes possible object index and target offset;
                    # parsing is deliberately withheld until boundaries verified.
                    context=rec[max(0,k-4):min(len(rec),k+20)].hex()
                    print(f"M350_FIXUP_CANDIDATE table={fmt(table)} object={o['index']} mode={mode} stateIndex={state} sourceOffset={fmt(source)} targetFlags={fmt(flags)} recordByteOffset={k} context={context} boundaryUnverified=1")
                print(f"M350_FIXUP_PREFIX table={fmt(table)} object={o['index']} mode={mode} hex={rec[:80].hex()}")
        print(f"M350_TABLE_END address={fmt(table)}")
    print("M350_LIMITATIONS noFixupsApplied=1 fixupCandidateBoundariesUnverified=1 runtimeAddressAssumptionsUnverified=1 rawTargetNotProof=1 SPRDecoderUnverified=1")

if __name__ == "__main__":
    main()
