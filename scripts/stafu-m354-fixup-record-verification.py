#!/usr/bin/env python3
"""M354: verify 07 10 LE fixup records for two candidate jump tables.

Read-only diagnostic. Reconstructs contiguous nine-byte runs independently
of M351's hardcoded target values. It does not prove the meaning of the
displacement operands or identify handler entry points.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LE=229780
SIZE=2008455
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",0x12334,1),("global_state",0x13488,0))
PREFIX=bytes((0x07,0x10))
assert PREFIX.hex()=="0710" and len(PREFIX)==2

def u32(b,p):
    return struct.unpack_from("<I",b,p)[0]
def u16(b,p):
    return struct.unpack_from("<H",b,p)[0]
def hx(n):
    return "none" if n is None else "0x%x"%n

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    b=args.ovl.read_bytes()
    sha=hashlib.sha256(b).hexdigest()
    print("M354_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=SIZE or sha!=SHA or b[LE:LE+2]!=b"LE":
        raise SystemExit("M354_ERROR unexpected executable")
    pagesize=u32(b,LE+0x28)
    total=u32(b,LE+0x14)
    objcount=u32(b,LE+0x44)
    objtab=LE+u32(b,LE+0x40)
    pagetab=LE+u32(b,LE+0x48)
    fixpage=LE+u32(b,LE+0x68)
    fixrec=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    if pagesize!=4096 or not 0<total<10000 or not 0<objcount<32:
        raise SystemExit("M354_ERROR unexpected LE layout")
    objects=[]
    for i in range(objcount):
        p=objtab+i*24
        objects.append(dict(index=i+1,size=u32(b,p),base=u32(b,p+4),
                            first=u32(b,p+12),count=u32(b,p+16)))
    starts=[u32(b,fixpage+4*i) for i in range(total+1)]
    if any(x>y for x,y in zip(starts,starts[1:])) or fixrec+starts[-1]>len(b):
        raise SystemExit("M354_ERROR invalid fixup index")
    print("M354_LAYOUT pageSize=%d totalPages=%d dataOrigin=%d fixupRecords=%d prefix=%s"%
          (pagesize,total,origin,fixrec,PREFIX.hex()))
    def physical(obj,rva):
        o=objects[obj-1]
        if rva<0 or rva>=o["size"]:return None
        local,within=divmod(rva,pagesize)
        if local>=o["count"]:return None
        logical=o["first"]+local
        raw=b[pagetab+4*(logical-1):pagetab+4*logical]
        number=int.from_bytes(raw[:3],"big")
        if number==0 or raw[3]!=0:return None
        p=origin+(number-1)*pagesize+within
        return p if p<len(b) else None
    def decode(blob,pos):
        if pos<0 or pos+9>len(blob) or blob[pos:pos+2]!=PREFIX:return None
        return (u16(blob,pos+2),blob[pos+4],u32(blob,pos+5))
    for name,displacement,first_state in TABLES:
        print("M354_TABLE_BEGIN name=%s displacement=%s"%(name,hx(displacement)))
        for o in objects:
            for mode in ("object_relative","virtual_minus_base"):
                rva=displacement if mode=="object_relative" else displacement-o["base"]
                p=physical(o["index"],rva)
                print("M354_LOCATION table=%s object=%d mode=%s rva=%s physical=%s"%
                      (name,o["index"],mode,hx(rva),hx(p)))
                if p is None:continue
                local,offset=divmod(rva,pagesize)
                logical=o["first"]+local
                lo,hi=starts[logical-1:logical+1]
                blob=b[fixrec+lo:fixrec+hi]
                print("M354_PAGE table=%s object=%d mode=%s logical=%d recordPhysical=%d recordBytes=%d prefix80=%s"%
                      (name,o["index"],mode,logical,fixrec+lo,len(blob),blob[:80].hex()))
                positions={offset+4*i:i for i in range(7) if offset+4*i<pagesize}
                candidates={i:[] for i in range(7)}
                all_prefix=0
                for pos in range(len(blob)-8):
                    record=decode(blob,pos)
                    if record is None:continue
                    all_prefix+=1
                    source,target_obj,target_rva=record
                    if source in positions:
                        idx=positions[source]
                        candidates[idx].append((pos,target_obj,target_rva))
                        dest=physical(target_obj,target_rva) if 1<=target_obj<=len(objects) else None
                        print("M354_CANDIDATE table=%s object=%d mode=%s state=%d recordPos=%d recordHex=%s source=%s targetObj=%d targetRva=%s targetPhysical=%s"%
                              (name,o["index"],mode,first_state+idx,pos,blob[pos:pos+9].hex(),
                               hx(source),target_obj,hx(target_rva),hx(dest)))
                print("M354_COUNTS table=%s object=%d mode=%s prefixOccurrences=%d stateCounts=%s"%
                      (name,o["index"],mode,all_prefix,
                       ",".join(str(len(candidates[i])) for i in range(7))))
                # Source offsets can occur in reverse order in the record stream.
                for direction in (-1,1):
                    runs=[]
                    for pos,obj,target in candidates[0]:
                        seq=[]
                        for idx in range(7):
                            q=pos+direction*9*idx
                            record=decode(blob,q)
                            if record is None or record[0]!=offset+4*idx:break
                            seq.append((idx,q,record[1],record[2]))
                        if len(seq)==7:runs.append(seq)
                    print("M354_RUNS table=%s object=%d mode=%s direction=%d complete=%d"%
                          (name,o["index"],mode,direction,len(runs)))
                    for runid,seq in enumerate(runs):
                        for idx,pos,target_obj,target_rva in seq:
                            target_phys=physical(target_obj,target_rva) if 1<=target_obj<=len(objects) else None
                            virtual=(objects[target_obj-1]["base"]+target_rva) if 1<=target_obj<=len(objects) else None
                            print("M354_RESOLVED table=%s object=%d mode=%s run=%d state=%d recordPos=%d targetObj=%d targetRva=%s targetVirtual=%s targetPhysical=%s targetBytes=%s"%
                                  (name,o["index"],mode,runid,first_state+idx,pos,target_obj,
                                   hx(target_rva),hx(virtual),hx(target_phys),
                                   b[target_phys:target_phys+16].hex() if target_phys is not None else "none"))
        print("M354_TABLE_END name=%s"%name)
    print("M354_LIMITATIONS prefixPatternIsCandidateOnly=1 fullLEFixupGrammarNotParsed=1 operandDisplacementUnverified=1 handlerEntryPointsUnconfirmed=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
