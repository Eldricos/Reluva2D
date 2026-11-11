#!/usr/bin/env python3
"""M355: audit dispatcher jump operands against independently decoded LE fixups.

Read-only STTNG.OVL diagnostic. Capstone required. Prints instruction bytes,
operand components, relocation records, simulated relocated table values,
and bounded disassembly around candidate targets. No handler is declared
verified from x86 decoding alone.
"""
import argparse
import hashlib
import struct
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_MEM, X86_OP_IMM

LE=229780
SIZE=2008455
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",566541,566260,566552,0x12334,1),
        ("global_state",570960,570696,571450,0x13488,0))
PREFIX=bytes((7,16))

def u32(b,p):
    return struct.unpack_from("<I",b,p)[0]
def u16(b,p):
    return struct.unpack_from("<H",b,p)[0]
def hx(v):
    return "none" if v is None else "0x%x"%v

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    b=args.ovl.read_bytes()
    digest=hashlib.sha256(b).hexdigest()
    print("M355_INPUT bytes=%d sha256=%s"%(len(b),digest))
    if len(b)!=SIZE or digest!=SHA or b[LE:LE+2]!=b"LE":
        raise SystemExit("M355_ERROR unexpected executable")
    size=u32(b,LE+0x28)
    nobj=u32(b,LE+0x44)
    total=u32(b,LE+0x14)
    objtab=LE+u32(b,LE+0x40)
    pagetab=LE+u32(b,LE+0x48)
    fixtab=LE+u32(b,LE+0x68)
    fixrec=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    if size!=4096 or nobj!=2 or total!=371:
        raise SystemExit("M355_ERROR unexpected LE layout")
    objects=[]
    for i in range(nobj):
        p=objtab+i*24
        objects.append(dict(size=u32(b,p),base=u32(b,p+4),
                            first=u32(b,p+12),count=u32(b,p+16)))
    starts=[u32(b,fixtab+i*4) for i in range(total+1)]
    if any(a>c for a,c in zip(starts,starts[1:])) or fixrec+starts[-1]>len(b):
        raise SystemExit("M355_ERROR invalid fixup index")
    print("M355_LAYOUT objects=%d pages=%d pageSize=%d dataOrigin=%d"%(nobj,total,size,origin))
    def physical(obj,rva):
        if not 1<=obj<=nobj:return None
        o=objects[obj-1]
        if not 0<=rva<o["size"]:return None
        local,within=divmod(rva,size)
        if local>=o["count"]:return None
        logical=o["first"]+local
        raw=b[pagetab+4*(logical-1):pagetab+4*logical]
        number=int.from_bytes(raw[:3],"big")
        if not number or raw[3]:return None
        p=origin+(number-1)*size+within
        return p if p<len(b) else None
    def inverse(p):
        result=[]
        for obj,o in enumerate(objects,1):
            for local in range(o["count"]):
                rva=local*size
                page=physical(obj,rva)
                if page is not None and page<=p<page+size:
                    candidate=rva+p-page
                    if candidate<o["size"]:
                        result.append((obj,candidate,o["base"]+candidate))
        return result
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    def show_ins(label,p):
        ins=next(md.disasm(b[p:p+15],p,count=1),None)
        if ins is None:
            print("M355_INSN label=%s physical=%d decode=failed"%(label,p))
            return None
        print("M355_INSN label=%s physical=%d size=%d hex=%s mnemonic=%s operands=%s"%
              (label,p,ins.size,ins.bytes.hex(),ins.mnemonic,ins.op_str))
        for i,op in enumerate(ins.operands):
            if op.type==X86_OP_MEM:
                mem=op.mem
                print("M355_MEM label=%s operand=%d base=%s index=%s scale=%d displacement=%s segment=%s size=%d"%
                      (label,i,ins.reg_name(mem.base) if mem.base else "none",
                       ins.reg_name(mem.index) if mem.index else "none",mem.scale,
                       hx(mem.disp),ins.reg_name(mem.segment) if mem.segment else "none",op.size))
            elif op.type==X86_OP_IMM:
                print("M355_IMM label=%s operand=%d immediate=%s"%(label,i,hx(op.imm)))
        return ins
    for name,site,entry,upper,table,first_state in TABLES:
        print("M355_DISPATCH_BEGIN name=%s site=%d tableDisplacement=%s"%(name,site,hx(table)))
        for label,p in (("entry",entry),("jump_candidate",site)):
            mapped=inverse(p)
            print("M355_ANCHOR name=%s label=%s physical=%d mappings=%s"%
                  (name,label,p,",".join("obj%d:rva%s:virtual%s"%t for t in
                   [(obj,hx(rva),hx(virt)) for obj,rva,virt in mapped]) or "none"))
            show_ins(name+"_"+label,p)
        # Independently find actual indirect jump instructions in a bounded
        # sequential decode. A jump target from an alternative CFG path might
        # not be on this sequential path; print nearby byte windows too.
        for start in (site-16,site-8,site-4,site):
            print("M355_SITE_WINDOW name=%s start=%d hex=%s"%
                  (name,start,b[start:site+16].hex()))
            for ins in md.disasm(b[start:site+16],start):
                if ins.address>site+8:break
                print("M355_SITE_DECODE name=%s from=%d physical=%d hex=%s instruction=%s %s"%
                      (name,start,ins.address,ins.bytes.hex(),ins.mnemonic,ins.op_str))
        o=objects[0]
        local,within=divmod(table,size)
        logical=o["first"]+local
        lo,hi=starts[logical-1:logical+1]
        blob=b[fixrec+lo:fixrec+hi]
        print("M355_FIXUP_PAGE name=%s logical=%d bytes=%d physical=%d"%
              (name,logical,len(blob),fixrec+lo))
        matches={}
        for pos in range(len(blob)-8):
            if blob[pos:pos+2]!=PREFIX:continue
            source=u16(blob,pos+2)
            if source<within or source>=within+28 or (source-within)%4:continue
            index=(source-within)//4
            matches.setdefault(index,[]).append((pos,blob[pos+4],u32(blob,pos+5)))
        for index in range(7):
            records=matches.get(index,[])
            print("M355_STATE name=%s state=%d records=%d"%(name,index+first_state,len(records)))
            for pos,obj,rva in records:
                target=physical(obj,rva)
                virt=objects[obj-1]["base"]+rva if 1<=obj<=nobj else None
                raw_at=physical(1,table+4*index)
                raw=u32(b,raw_at) if raw_at is not None else None
                prev=b[max(0,target-8):target].hex() if target is not None else "none"
                following=b[target:target+32].hex() if target is not None else "none"
                print("M355_RELOCATION name=%s state=%d recordPos=%d recordHex=%s source=%s targetObj=%d targetRva=%s targetVirtual=%s targetPhysical=%s originalWord=%s before8=%s after32=%s"%
                      (name,index+first_state,pos,blob[pos:pos+9].hex(),
                       hx(within+4*index),obj,hx(rva),hx(virt),hx(target),hx(raw),prev,following))
                if target is None:continue
                for delta in (-5,-2,-1,0,1,2):
                    p=target+delta
                    if p<0:continue
                    ins=next(md.disasm(b[p:p+15],p,count=1),None)
                    print("M355_TARGET_BOUNDARY name=%s state=%d delta=%d physical=%d first=%s"%
                          (name,index+first_state,delta,p,
                           (ins.mnemonic+" "+ins.op_str) if ins else "decode_failed"))
        print("M355_DISPATCH_END name=%s"%name)
    print("M355_LIMITATIONS noFullLEFixupGrammar=1 jumpOperandToTableLinkUnverified=1 relocatedValuesSimulatedNotExecuted=1 instructionBoundaryHeuristic=1 handlerSemanticsUnconfirmed=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
