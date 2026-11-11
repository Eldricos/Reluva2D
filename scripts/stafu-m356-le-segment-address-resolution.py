#!/usr/bin/env python3
"""M356: compare LE object, CS-relative, and linear-address interpretations.

Read-only. Does not assume the CS selector base or apply a full LE loader.
Audits relocation records at the dispatcher operand fields, possible table
locations, and control-flow plausibility of candidate target addresses.
"""
import argparse,hashlib,struct
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",566541,0x12334,1),("global_state",570965,0x13488,0))
PREFIX=bytes((7,16))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def hx(x):return "none" if x is None else "0x%x"%x
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
    b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M356_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M356_ERROR unexpected OVL")
    pagesize=u32(b,LE+0x28);n=u32(b,LE+0x44);total=u32(b,LE+0x14)
    objtab=LE+u32(b,LE+0x40);pagetab=LE+u32(b,LE+0x48)
    fixtab=LE+u32(b,LE+0x68);fixrec=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
    objects=[dict(size=u32(b,objtab+i*24),base=u32(b,objtab+i*24+4),flags=u32(b,objtab+i*24+8),first=u32(b,objtab+i*24+12),count=u32(b,objtab+i*24+16)) for i in range(n)]
    starts=[u32(b,fixtab+i*4) for i in range(total+1)]
    if pagesize!=4096 or n!=2 or any(x>y for x,y in zip(starts,starts[1:])):raise SystemExit("M356_ERROR layout")
    print("M356_LAYOUT pages=%d pageSize=%d dataOrigin=%d fixupRecords=%d"%(total,pagesize,origin,fixrec))
    for i,o in enumerate(objects,1):
        print("M356_OBJECT number=%d base=%s size=%s flags=%s firstPage=%d count=%d"%(i,hx(o["base"]),hx(o["size"]),hx(o["flags"]),o["first"],o["count"]))
    def physical(obj,rva):
        if not 1<=obj<=n:return None
        o=objects[obj-1]
        if not 0<=rva<o["size"]:return None
        page,within=divmod(rva,pagesize)
        if page>=o["count"]:return None
        logical=o["first"]+page;raw=b[pagetab+4*(logical-1):pagetab+4*logical]
        num=int.from_bytes(raw[:3],"big")
        if not num or raw[3]:return None
        p=origin+(num-1)*pagesize+within
        return p if p<len(b) else None
    def inverse(p):
        results=[]
        for obj,o in enumerate(objects,1):
            for local in range(o["count"]):
                page=physical(obj,local*pagesize)
                if page is not None and page<=p<page+pagesize:
                    rva=local*pagesize+p-page
                    if rva<o["size"]:results.append((obj,rva,o["base"]+rva))
        return results
    def records(obj,rva):
        o=objects[obj-1];local,offset=divmod(rva,pagesize);logical=o["first"]+local
        lo,hi=starts[logical-1:logical+1];blob=b[fixrec+lo:fixrec+hi]
        hits=[]
        for pos in range(len(blob)-8):
            if blob[pos:pos+2]!=PREFIX or u16(blob,pos+2)!=offset:continue
            target_obj=blob[pos+4];target=u32(blob,pos+5)
            hits.append((pos,target_obj,target,blob[pos:pos+9].hex()))
        return logical,offset,hits
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,site,disp,first in TABLES:
        print("M356_DISPATCH_BEGIN name=%s physical=%d displacement=%s"%(name,site,hx(disp)))
        ins=next(md.disasm(b[site:site+15],site,count=1),None)
        print("M356_JUMP bytes=%s decoded=%s"%(b[site:site+8].hex(),ins.mnemonic+" "+ins.op_str if ins else "none"))
        mappings=inverse(site)
        for obj,rva,virt in mappings:
            print("M356_JUMP_MAP object=%d rva=%s virtual=%s"%(obj,hx(rva),hx(virt)))
            # CS segment can use object base or a distinct selector base;
            # these are hypotheses, not loader-derived facts.
            for base_label,base in (("zero",0),("own_object_base",objects[obj-1]["base"]),("object1_base",objects[0]["base"]),("object2_base",objects[1]["base"])):
                linear=base+disp
                for target_obj,o in enumerate(objects,1):
                    candidate=linear-o["base"]
                    p=physical(target_obj,candidate)
                    print("M356_SEGMENT_HYPOTHESIS name=%s codeObject=%d csBaseMode=%s csBase=%s linear=%s targetObject=%d targetRva=%s targetPhysical=%s"%
                          (name,obj,base_label,hx(base),hx(linear),target_obj,hx(candidate),hx(p)))
        # For a disp32 indirect jmp, its displacement is the last four bytes.
        for fieldphys in range(site,site+8):
            for obj,rva,virt in inverse(fieldphys):
                logical,offset,hits=records(obj,rva)
                if fieldphys in (site+4,site+5,site+6,site+7):
                    print("M356_OPERAND_BYTE name=%s physical=%d object=%d rva=%s page=%d offset=%s exactFixups=%d"%
                          (name,fieldphys,obj,hx(rva),logical,hx(offset),len(hits)))
                    for pos,tobj,trva,raw in hits:
                        print("M356_OPERAND_FIXUP name=%s sourcePhysical=%d recordPos=%d targetObj=%d targetRva=%s raw=%s"%
                              (name,fieldphys,pos,tobj,hx(trva),raw))
        for obj,o in enumerate(objects,1):
            for mode,rva in (("object_relative",disp),("virtual_minus_base",disp-o["base"])):
                p=physical(obj,rva)
                print("M356_TABLE_LOCATION name=%s object=%d mode=%s rva=%s physical=%s"%
                      (name,obj,mode,hx(rva),hx(p)))
                if p is None:continue
                print("M356_TABLE_BYTES name=%s object=%d mode=%s hex=%s"%
                      (name,obj,mode,b[p:p+32].hex()))
                for i in range(7):
                    er=rva+4*i;ep=physical(obj,er)
                    if ep is None:continue
                    logical,offset,hits=records(obj,er)
                    print("M356_ENTRY name=%s object=%d mode=%s state=%d rva=%s raw=%s fixups=%d"%
                          (name,obj,mode,first+i,hx(er),hx(u32(b,ep)),len(hits)))
                    for pos,tobj,trva,raw in hits:
                        tp=physical(tobj,trva)
                        virtual=objects[tobj-1]["base"]+trva if 1<=tobj<=n else None
                        print("M356_ENTRY_FIXUP name=%s object=%d mode=%s state=%d recordPos=%d targetObject=%d targetRva=%s targetVirtual=%s targetPhysical=%s raw=%s"%
                              (name,obj,mode,first+i,pos,tobj,hx(trva),hx(virtual),hx(tp),raw))
        print("M356_DISPATCH_END name=%s"%name)
    print("M356_LIMITATIONS csBaseNotKnown=1 selectorDescriptorsNotDecoded=1 completeFixupGrammarNotParsed=1 noRuntimeExecution=1 candidateTargetsNotValidated=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
