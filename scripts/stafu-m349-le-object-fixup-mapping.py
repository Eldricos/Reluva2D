#!/usr/bin/env python3
"""M349: inspect LE object/page/fixup mapping and resource jump tables.

Read-only. Never applies fixups or changes STTNG.OVL. Mapping and table
interpretations are reported with explicit confidence limits.
"""
import argparse, hashlib, struct
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
SIZE=2008455
LE=229780
TABLES=(0x12334,0x13488)
def u8(b,p): return b[p] if 0<=p<len(b) else None
def u16(b,p): return struct.unpack_from("<H",b,p)[0] if 0<=p<=len(b)-2 else None
def u32(b,p): return struct.unpack_from("<I",b,p)[0] if 0<=p<=len(b)-4 else None
def hx(v): return "none" if v is None else hex(v)
def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    b=args.ovl.read_bytes()
    sha=hashlib.sha256(b).hexdigest()
    print(f"M349_INPUT bytes={len(b)} sha256={sha}")
    if len(b)!=SIZE or sha!=SHA:raise SystemExit("M349_ERROR unexpected OVL")
    if b[LE:LE+2]!=b"LE":raise SystemExit("M349_ERROR LE signature")
    fields={"cpu":(0x08,2),"os":(0x0a,2),"moduleFlags":(0x10,4),
      "pageCount":(0x14,4),"entryObject":(0x18,4),"entryOffset":(0x1c,4),
      "pageSize":(0x28,4),"lastPageSize":(0x2c,4),
      "objectTable":(0x40,4),"objectCount":(0x44,4),"objectPageTable":(0x48,4),
      "iteratedPages":(0x4c,4),"fixupPageTable":(0x68,4),
      "fixupRecordTable":(0x6c,4),"dataPagesOffset":(0x80,4),
      "preloadPages":(0x84,4),"nonresidentNames":(0x88,4)}
    f={}
    for name,(rel,n) in fields.items():
        v=u16(b,LE+rel) if n==2 else u32(b,LE+rel)
        f[name]=v
        print(f"M349_HEADER field={name} relative={rel:#x} value={hx(v)}")
    objects=[]
    for idx in range(min(f["objectCount"],32)):
        p=LE+f["objectTable"]+idx*24
        size,base,flags,first,count,reserved=(u32(b,p+i*4) for i in range(6))
        obj=dict(idx=idx+1,size=size,base=base,flags=flags,first=first,count=count)
        objects.append(obj)
        print(f"M349_OBJECT index={idx+1} tablePhysical={p} size={hx(size)} base={hx(base)} flags={hx(flags)} firstPage={first} pageCount={count} reserved={hx(reserved)}")
    page_size=f["pageSize"]
    page_table=LE+f["objectPageTable"]
    data_start=LE+f["dataPagesOffset"]
    print(f"M349_LAYOUT pageTablePhysical={page_table} dataStartPhysical={data_start} pageSize={page_size} note=LE_page_map_entry_is_4_bytes")
    maps={}
    for obj in objects:
        for local in range(obj["count"]):
            logical=obj["first"]+local
            p=page_table+(logical-1)*4
            # LE page map: 24-bit physical page number, 8-bit flags.
            raw=b[p:p+4]
            if len(raw)!=4:continue
            page_num=int.from_bytes(raw[:3],"little")
            flag=raw[3]
            # LE page numbers start at 1, physical data pages are relative to data_start.
            physical=data_start+(page_num-1)*page_size if page_num else None
            maps[(obj["idx"],local)]=(physical,flag,page_num)
            if local<4 or local>=obj["count"]-2:
                print(f"M349_PAGE object={obj['idx']} local={local} logical={logical} mapPhysical={p} raw={raw.hex()} pageNumber={page_num} flags={flag} physical={hx(physical)}")
        print(f"M349_PAGE_SUMMARY object={obj['idx']} mapped={sum((obj['idx'],j) in maps for j in range(obj['count']))}")
    def map_rva(obj,rva):
        if rva<0 or rva>=obj["size"] or not page_size:return None
        local=rva//page_size
        item=maps.get((obj["idx"],local))
        if not item:return None
        phys,flag,num=item
        if phys is None or flag!=0:return None
        out=phys+rva%page_size
        return out if 0<=out<len(b) else None
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for addr in TABLES:
        print(f"M349_TABLE_BEGIN runtimeAddress={addr:#x}")
        for obj in objects:
            for mode,rva in (("absolute_minus_object_base",addr-obj["base"]),("object_relative",addr)):
                phys=map_rva(obj,rva)
                print(f"M349_TABLE_CANDIDATE runtimeAddress={addr:#x} object={obj['idx']} mode={mode} rva={hx(rva)} physical={hx(phys)} valid={int(phys is not None)}")
                if phys is None or phys+28>len(b):continue
                for state in range(7):
                    value=u32(b,phys+state*4)
                    print(f"M349_TABLE_WORD runtimeAddress={addr:#x} object={obj['idx']} mode={mode} stateIndex={state} physical={phys+state*4} value={hx(value)}")
                    for target in objects:
                        for targetmode,off in (("absolute_minus_base",value-target["base"]),("object_relative",value)):
                            dest=map_rva(target,off)
                            if dest is None:continue
                            ins=next(md.disasm(b[dest:dest+12],dest,count=1),None)
                            if ins:
                                print(f"M349_TARGET_CANDIDATE table={addr:#x} stateIndex={state} object={target['idx']} mode={targetmode} physical={dest} first={ins.mnemonic}_{ins.op_str.replace(' ','_')} verified=0")
        print(f"M349_TABLE_END runtimeAddress={addr:#x}")
    # LE fixup page table gives record ranges per logical page. We deliberately
    # do not parse variable-length relocation records as fixed-width data.
    fixpage=LE+f["fixupPageTable"]
    fixrec=LE+f["fixupRecordTable"]
    total_pages=f["pageCount"]
    if total_pages<200000 and fixpage+(total_pages+1)*4<=len(b):
        starts=[u32(b,fixpage+i*4) for i in range(total_pages+1)]
        print(f"M349_FIXUP_TABLE pageTablePhysical={fixpage} recordTablePhysical={fixrec} entries={len(starts)} monotonic={int(all(x<=y for x,y in zip(starts,starts[1:])))}")
        for obj in objects:
            for addr in TABLES:
                for mode,rva in (("absolute_minus_object_base",addr-obj["base"]),("object_relative",addr)):
                    if not 0<=rva<obj["size"]:continue
                    local=rva//page_size
                    logical=obj["first"]+local
                    if not 1<=logical<=total_pages:continue
                    lo=starts[logical-1];hi=starts[logical]
                    print(f"M349_FIXUP_PAGE table={addr:#x} object={obj['idx']} mode={mode} logicalPage={logical} recordStart={lo} recordEnd={hi} recordBytes={hi-lo} recordPhysical={fixrec+lo}")
                    print(f"M349_FIXUP_HEX table={addr:#x} object={obj['idx']} mode={mode} prefix={b[fixrec+lo:fixrec+min(hi,lo+128)].hex()}")
    print("M349_LIMITATIONS fixupRecordsNotDecoded=1 relocationTargetsUnapplied=1 runtimeLoadBaseUnknown=1 LEPageFlagsMayRequireSpecialHandling=1 tableCandidateNotProof=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
