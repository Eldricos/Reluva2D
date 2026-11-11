#!/usr/bin/env python3
"""M353: independent LE page/fixup audit for the two resource-state jump tables.

Read-only, standard-library diagnostic. Does not infer handler identity from
x86-looking bytes. Uses exact fixup source offsets, checks record alignment
within contiguous runs, and reports both virtual-address interpretations.
"""
import argparse
import hashlib
import struct
from pathlib import Path

EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LE = 229780
TABLES = (("object_state", 0x12334, 1), ("global_state", 0x13488, 0))
ANCHORS = (566260, 566541, 570696, 570960)

def u16(data, at):
    return struct.unpack_from("<H", data, at)[0] if 0 <= at <= len(data)-2 else None

def u32(data, at):
    return struct.unpack_from("<I", data, at)[0] if 0 <= at <= len(data)-4 else None

def hx(value):
    return "none" if value is None else "0x%x" % value

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print("M353_INPUT bytes=%d sha256=%s" % (len(data), digest))
    if len(data) != EXPECTED_SIZE or digest != EXPECTED_SHA or data[LE:LE+2] != b"LE":
        raise SystemExit("M353_ERROR unexpected STTNG.OVL")
    page_size = u32(data, LE+0x28)
    obj_count = u32(data, LE+0x44)
    total_pages = u32(data, LE+0x14)
    obj_at = LE+u32(data, LE+0x40)
    map_at = LE+u32(data, LE+0x48)
    fix_at = LE+u32(data, LE+0x68)
    rec_at = LE+u32(data, LE+0x6c)
    origin = LE+u32(data, LE+0x80)
    if not (page_size == 4096 and 0 < obj_count < 32 and 0 < total_pages < 10000):
        raise SystemExit("M353_ERROR unexpected LE layout")
    objects = []
    for i in range(obj_count):
        p = obj_at+24*i
        o = dict(number=i+1, size=u32(data,p), base=u32(data,p+4),
                 flags=u32(data,p+8), first=u32(data,p+12), count=u32(data,p+16))
        objects.append(o)
        print("M353_OBJECT number=%d size=%s base=%s flags=%s first=%d pages=%d" %
              (o["number"],hx(o["size"]),hx(o["base"]),hx(o["flags"]),o["first"],o["count"]))
    print("M353_LAYOUT pageSize=%d pageMap=%d fixupIndex=%d fixupRecords=%d dataOrigin=%d totalPages=%d" %
          (page_size,map_at,fix_at,rec_at,origin,total_pages))
    mapping = {}
    reverse = {}
    for o in objects:
        for local in range(o["count"]):
            logical = o["first"]+local
            at = map_at+(logical-1)*4
            raw = data[at:at+4]
            if len(raw)!=4:
                raise SystemExit("M353_ERROR truncated page map")
            number = int.from_bytes(raw[:3],"big")
            flags = raw[3]
            physical = origin+(number-1)*page_size if number and flags==0 else None
            mapping[(o["number"],local)] = (physical,number,flags)
            if physical is not None:
                reverse.setdefault(physical,[]).append((o["number"],local))
    def physical(obj, rva):
        o=objects[obj-1]
        if rva<0 or rva>=o["size"]:
            return None
        local,within=divmod(rva,page_size)
        base,_,flags=mapping.get((obj,local),(None,None,None))
        if base is None or flags!=0 or base+within>=len(data):
            return None
        return base+within
    for anchor in ANCHORS:
        matches=[]
        for base, locs in reverse.items():
            if base<=anchor<base+page_size:
                for obj,local in locs:
                    rva=local*page_size+anchor-base
                    if physical(obj,rva)==anchor:
                        matches.append("obj%d:rva%s:virtual%s" %
                                       (obj,hx(rva),hx(objects[obj-1]["base"]+rva)))
        print("M353_ANCHOR physical=%d inverse=%s" % (anchor,",".join(matches) or "none"))
    starts=[u32(data,fix_at+4*i) for i in range(total_pages+1)]
    if any(v is None for v in starts) or any(a>b for a,b in zip(starts,starts[1:])):
        raise SystemExit("M353_ERROR invalid fixup index")
    print("M353_FIXUP_INDEX entries=%d totalBytes=%d monotonic=1" % (len(starts),starts[-1]))
    def page_records(logical):
        if not 1<=logical<=total_pages:
            return None
        lo,hi=starts[logical-1:logical+1]
        if hi<lo or rec_at+hi>len(data):
            return None
        return data[rec_at+lo:rec_at+hi]
    def decode_run(blob, position):
        # A candidate record is exactly 9 bytes only for source 0x07 and
        # target flags 0x10. No claim is made about unrelated record formats.
        if position<0 or position+9>len(blob) or blob[position:position+2]!=b"\\x07\\x10":
            return None
        return dict(source=u16(blob,position+2),target_obj=blob[position+4],
                    target_rva=u32(blob,position+5),position=position)
    for name, displacement, first_state in TABLES:
        print("M353_TABLE_BEGIN name=%s displacement=%s firstState=%d" %
              (name,hx(displacement),first_state))
        for o in objects:
            for mode in ("object_relative","virtual_minus_base"):
                rva=displacement if mode=="object_relative" else displacement-o["base"]
                print("M353_INTERPRETATION name=%s obj=%d mode=%s rva=%s physical=%s" %
                      (name,o["number"],mode,hx(rva),hx(physical(o["number"],rva))))
                for idx in range(7):
                    entry_rva=rva+4*idx
                    entry_phys=physical(o["number"],entry_rva)
                    if entry_phys is None:
                        continue
                    local,source=divmod(entry_rva,page_size)
                    logical=o["first"]+local
                    blob=page_records(logical)
                    raw=u32(data,entry_phys)
                    matches=[]
                    if blob is not None:
                        for pos in range(len(blob)-8):
                            rec=decode_run(blob,pos)
                            if rec and rec["source"]==source:
                                matches.append(rec)
                    print("M353_ENTRY name=%s obj=%d mode=%s state=%d entryRva=%s entryPhysical=%d raw=%s logicalPage=%d source=%s candidateCount=%d" %
                          (name,o["number"],mode,first_state+idx,hx(entry_rva),entry_phys,hx(raw),logical,hx(source),len(matches)))
                    for rec in matches:
                        target_obj=rec["target_obj"]
                        target_rva=rec["target_rva"]
                        target_phys=physical(target_obj,target_rva) if 1<=target_obj<=len(objects) else None
                        target_virtual=(objects[target_obj-1]["base"]+target_rva) if 1<=target_obj<=len(objects) else None
                        # Verify neighboring records occupy exact 9-byte boundaries.
                        neighbors=[]
                        for direction in (-1,1):
                            q=rec["position"]+9*direction
                            neighbor=decode_run(blob,q)
                            neighbors.append(neighbor["source"] if neighbor else None)
                        valid_target=(1<=target_obj<=len(objects) and
                                      target_rva<objects[target_obj-1]["size"])
                        sample=data[target_phys:target_phys+24].hex() if target_phys is not None else "none"
                        print("M353_FIXUP name=%s obj=%d mode=%s state=%d recordAt=%d recordHex=%s targetObj=%d targetRva=%s targetVirtual=%s targetPhysical=%s targetInObject=%d adjacentSources=%s,%s targetHex=%s" %
                              (name,o["number"],mode,first_state+idx,rec["position"],
                               blob[rec["position"]:rec["position"]+9].hex(),target_obj,
                               hx(target_rva),hx(target_virtual),hx(target_phys),
                               int(valid_target),hx(neighbors[0]),hx(neighbors[1]),sample))
        print("M353_TABLE_END name=%s" % name)
    print("M353_LIMITATIONS partialFixupParser=1 sourceRecordBoundariesNotFullyProven=1 codeValidityNotEstablished=1 displacementSegmentSemanticsUnverified=1 noHandlersConfirmed=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
