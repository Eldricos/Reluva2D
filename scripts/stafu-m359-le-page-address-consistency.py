#!/usr/bin/env python3
"""M359: independently audit LE page mappings and dispatcher/table/target locations.

Read-only; validates page-map uniqueness, flags, object page ranges and physical
overlap. Does not execute code, apply relocations, or claim runtime equivalence.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",566541,0x12334,[0x1239b,0x1238e,0x123ca,0x123f8,0x123eb,0x123ca,0x12429]),
        ("global_state",570965,0x13488,[0x13598,0x13598,0x1356e,0x134ed,0x13598,0x1356e,0x13561]))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def hx(v):return "none" if v is None else "0x%x"%v
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
    b=args.ovl.read_bytes();digest=hashlib.sha256(b).hexdigest()
    print("M359_INPUT bytes=%d sha256=%s"%(len(b),digest))
    if len(b)!=2008455 or digest!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M359_ERROR unexpected OVL")
    page_size=u32(b,LE+0x28);page_count=u32(b,LE+0x14);obj_count=u32(b,LE+0x44)
    obj_off=LE+u32(b,LE+0x40);map_off=LE+u32(b,LE+0x48)
    data_rel=u32(b,LE+0x80);origin=LE+data_rel
    if page_size!=4096 or page_count!=371 or obj_count!=2:raise SystemExit("M359_ERROR unexpected layout")
    objects=[]
    for i in range(obj_count):
        p=obj_off+i*24
        objects.append(dict(size=u32(b,p),base=u32(b,p+4),flags=u32(b,p+8),first=u32(b,p+12),count=u32(b,p+16)))
    entries=[]
    for i in range(page_count):
        raw=b[map_off+i*4:map_off+(i+1)*4]
        entries.append(dict(logical=i+1,raw=raw.hex(),physical_page=int.from_bytes(raw[:3],"big"),flags=raw[3]))
    print("M359_LAYOUT leHeader=%d pageSize=%d totalPages=%d objects=%d pageMapPhysical=%d dataRelative=%d dataOrigin=%d"%(LE,page_size,page_count,obj_count,map_off,data_rel,origin))
    for i,o in enumerate(objects,1):
        print("M359_OBJECT number=%d base=%s size=%s flags=%s firstPage=%d pageCount=%d lastPage=%d"%(i,hx(o["base"]),hx(o["size"]),hx(o["flags"]),o["first"],o["count"],o["first"]+o["count"]-1))
    counts=collections.Counter(e["flags"] for e in entries)
    nums=collections.Counter(e["physical_page"] for e in entries if e["physical_page"])
    duplicate={k:v for k,v in nums.items() if v>1}
    print("M359_PAGE_SUMMARY flags=%s zeroPageNumbers=%d uniquePhysicalPages=%d duplicates=%d minPhysical=%s maxPhysical=%s"%
          (",".join("%s:%d"%(hx(k),v) for k,v in sorted(counts.items())),
           sum(e["physical_page"]==0 for e in entries),len(nums),len(duplicate),
           hx(min(nums)) if nums else "none",hx(max(nums)) if nums else "none"))
    for number,count in sorted(duplicate.items()):
        logical=[str(e["logical"]) for e in entries if e["physical_page"]==number]
        print("M359_ALIAS physicalPage=%d count=%d logicalPages=%s"%(number,count,",".join(logical)))
    def resolve(obj,rva):
        if not 1<=obj<=len(objects):return None
        o=objects[obj-1]
        if not 0<=rva<o["size"]:return None
        local,within=divmod(rva,page_size)
        if local>=o["count"]:return None
        logical=o["first"]+local
        if not 1<=logical<=len(entries):return None
        e=entries[logical-1]
        number=e["physical_page"]
        physical=origin+(number-1)*page_size+within if number and e["flags"]==0 else None
        return dict(obj=obj,rva=rva,local=local,within=within,logical=logical,number=number,flags=e["flags"],raw=e["raw"],physical=physical,
                    virtual=o["base"]+rva)
    def inverse(physical):
        out=[]
        for obj,o in enumerate(objects,1):
            for local in range(o["count"]):
                mapped=resolve(obj,local*page_size)
                if mapped and mapped["physical"] is not None and mapped["physical"]<=physical<mapped["physical"]+page_size:
                    rva=local*page_size+physical-mapped["physical"]
                    if rva<o["size"]:out.append(resolve(obj,rva))
        return out
    for name,site,table,targets in TABLES:
        print("M359_DISPATCH_BEGIN name=%s sitePhysical=%d displacement=%s"%(name,site,hx(table)))
        for mapping in inverse(site):
            print("M359_SITE_MAP name=%s object=%d rva=%s virtual=%s logical=%d physicalPage=%d pageOffset=%s"%
                  (name,mapping["obj"],hx(mapping["rva"]),hx(mapping["virtual"]),mapping["logical"],mapping["number"],hx(mapping["within"])))
        for label,rva in [("table",table)]+[("target_%d"%i,t) for i,t in enumerate(targets)]:
            mapping=resolve(1,rva)
            if not mapping:
                print("M359_LOCATION name=%s label=%s rva=%s mapped=0"%(name,label,hx(rva)))
                continue
            p=mapping["physical"]
            print("M359_LOCATION name=%s label=%s object=1 rva=%s virtual=%s logical=%d physicalPage=%d flags=%s pageOffset=%s physical=%s rawMap=%s bytes=%s"%
                  (name,label,hx(rva),hx(mapping["virtual"]),mapping["logical"],mapping["number"],hx(mapping["flags"]),
                   hx(mapping["within"]),hx(p),mapping["raw"],b[p:p+12].hex() if p is not None else "none"))
        table_map=resolve(1,table)
        if table_map and table_map["physical"] is not None:
            physical=table_map["physical"]
            print("M359_TABLE_WINDOW name=%s physical=%d hex=%s"%(name,physical,b[physical:physical+64].hex()))
        print("M359_DISPATCH_END name=%s"%name)
    for logical in (1,2,16,17,19,20,217,218,371):
        if 1<=logical<=page_count:
            e=entries[logical-1]
            start=origin+(e["physical_page"]-1)*page_size if e["physical_page"] and e["flags"]==0 else None
            print("M359_PAGE_SAMPLE logical=%d raw=%s physicalPage=%d flags=%s start=%s end=%s"%
                  (logical,e["raw"],e["physical_page"],hx(e["flags"]),hx(start),hx(start+page_size-1) if start is not None else "none"))
    print("M359_LIMITATIONS pageMapFormatAssumedBigEndian24=1 physicalDataOriginAssumedHeaderRelative=1 noLELoaderExecution=1 fixupsNotApplied=1 runtimeSelectorBaseUnknown=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
