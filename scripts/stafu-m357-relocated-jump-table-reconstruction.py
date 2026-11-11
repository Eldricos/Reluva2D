#!/usr/bin/env python3
"""M357: reconstruct two candidate LE jump tables in isolated memory.

Uses only the fourteen observed 07 10 records; does not emulate the full LE
loader. The original executable is never modified. Reports code boundary
heuristics and distinct hypotheses for interpreting runtime pointer values.
"""
import argparse,hashlib,struct
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780
SIZE=2008455
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",0x12334,1),("global_state",0x13488,0))
PREFIX=bytes((7,16))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def hx(v):return "none" if v is None else "0x%x"%v
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl",type=Path)
    a=parser.parse_args();data=a.ovl.read_bytes()
    sha=hashlib.sha256(data).hexdigest()
    print("M357_INPUT bytes=%d sha256=%s"%(len(data),sha))
    if len(data)!=SIZE or sha!=SHA or data[LE:LE+2]!=b"LE":
        raise SystemExit("M357_ERROR unexpected OVL")
    size=u32(data,LE+0x28);total=u32(data,LE+0x14);n=u32(data,LE+0x44)
    objtab=LE+u32(data,LE+0x40);pagetab=LE+u32(data,LE+0x48)
    fixtab=LE+u32(data,LE+0x68);fixrec=LE+u32(data,LE+0x6c)
    origin=LE+u32(data,LE+0x80)
    if size!=4096 or n!=2 or total!=371:raise SystemExit("M357_ERROR unexpected LE layout")
    objects=[dict(size=u32(data,objtab+i*24),base=u32(data,objtab+i*24+4),
                  first=u32(data,objtab+i*24+12),count=u32(data,objtab+i*24+16))
             for i in range(n)]
    starts=[u32(data,fixtab+4*i) for i in range(total+1)]
    if any(a>b for a,b in zip(starts,starts[1:])) or fixrec+starts[-1]>len(data):
        raise SystemExit("M357_ERROR invalid fixup index")
    print("M357_LAYOUT pages=%d pageSize=%d objects=%d dataOrigin=%d"%(total,size,n,origin))
    def physical(obj,rva):
        if not 1<=obj<=n:return None
        o=objects[obj-1]
        if not 0<=rva<o["size"]:return None
        local,within=divmod(rva,size)
        if local>=o["count"]:return None
        logical=o["first"]+local
        raw=data[pagetab+4*(logical-1):pagetab+4*logical]
        page=int.from_bytes(raw[:3],"big")
        if not page or raw[3]:return None
        p=origin+(page-1)*size+within
        return p if p<len(data) else None
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,table,first in TABLES:
        obj=1
        local,within=divmod(table,size)
        logical=objects[0]["first"]+local
        lo,hi=starts[logical-1:logical+1]
        records=data[fixrec+lo:fixrec+hi]
        print("M357_TABLE_BEGIN name=%s tableRva=%s logicalPage=%d fixupBytes=%d"%(name,hx(table),logical,len(records)))
        candidates={}
        for pos in range(len(records)-8):
            if records[pos:pos+2]!=PREFIX:continue
            source=u16(records,pos+2)
            if not within<=source<within+28 or (source-within)%4:continue
            idx=(source-within)//4
            candidates.setdefault(idx,[]).append((pos,records[pos+4],u32(records,pos+5)))
        if any(len(candidates.get(i,[]))!=1 for i in range(7)):
            print("M357_ERROR name=%s candidateCounts=%s"%(name,",".join(str(len(candidates.get(i,[]))) for i in range(7))))
            continue
        positions=[candidates[i][0][0] for i in range(7)]
        reverse=all(positions[i]-positions[i+1]==9 for i in range(6))
        print("M357_CHAIN name=%s complete=1 reverseNineByte=%d positions=%s"%(name,int(reverse),",".join(map(str,positions))))
        # Apply only the observed candidate 32-bit internal fixups to a
        # 28-byte private copy. Source words are otherwise untouched.
        original=bytearray()
        for i in range(7):
            p=physical(obj,table+4*i)
            if p is None:raise SystemExit("M357_ERROR unmappable table")
            original.extend(data[p:p+4])
        patched=bytearray(original)
        for i in range(7):
            pos,target_obj,target_rva=candidates[i][0]
            if not 1<=target_obj<=n:raise SystemExit("M357_ERROR target object")
            target_linear=objects[target_obj-1]["base"]+target_rva
            struct.pack_into("<I",patched,i*4,target_linear)
            p=physical(target_obj,target_rva)
            print("M357_ENTRY name=%s state=%d sourceRva=%s sourcePhysical=%s recordPos=%d record=%s originalWord=%s patchedLinear=%s targetObject=%d targetRva=%s targetPhysical=%s"%
                  (name,first+i,hx(table+4*i),hx(physical(obj,table+4*i)),pos,
                   records[pos:pos+9].hex(),hx(u32(original,4*i)),hx(u32(patched,4*i)),
                   target_obj,hx(target_rva),hx(p)))
            # Explicitly compare the target's location under two possible
            # address interpretations. These are hypotheses, not runtime facts.
            for mode,interpreted_rva in (("object_relative",target_rva),
                                         ("linear_minus_object_base",target_linear-objects[target_obj-1]["base"]),
                                         ("linear_as_rva",target_linear)):
                q=physical(target_obj,interpreted_rva)
                print("M357_TARGET_MAP name=%s state=%d mode=%s rva=%s physical=%s"%
                      (name,first+i,mode,hx(interpreted_rva),hx(q)))
            if p is None:continue
            for delta in (-4,-3,-2,-1,0,1,2):
                q=p+delta
                if q<0:continue
                ins=next(md.disasm(data[q:q+15],q,count=1),None)
                print("M357_BOUNDARY name=%s state=%d delta=%d physical=%d mnemonic=%s bytes=%s"%
                      (name,first+i,delta,q,
                       (ins.mnemonic+" "+ins.op_str) if ins else "decode_failed",
                       ins.bytes.hex() if ins else "none"))
        print("M357_RECONSTRUCTED name=%s originalHex=%s patchedHex=%s"%
              (name,original.hex(),patched.hex()))
        print("M357_TABLE_END name=%s"%name)
    print("M357_LIMITATIONS subsetOfFixupsOnly=1 fullLELoaderNotEmulated=1 csSelectorBaseUnverified=1 codeBoundaryHeuristicOnly=1 runtimeDispatchUnproven=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
