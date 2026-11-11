#!/usr/bin/env python3
"""M368: disassemble reconstructed LE dispatcher handlers (read-only).

Rebuilds all 32-bit internal-offset fixups on object 1 logical pages 19/20.
Reports relocated handler bytes, decoded instructions, pointer-source overlap,
and direct branch targets decoded from local linear sweeps. Static evidence
only; Capstone decoding does not establish reachability.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_CALL,CS_GRP_JUMP
from capstone.x86_const import X86_OP_IMM
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",19,0x334,1),("global_state",20,0x488,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def parse(blob,pos):
    start=pos
    if pos+2>len(blob):raise ValueError("truncated record")
    src,flags=blob[pos:pos+2];pos+=2
    kind=src&15
    if kind not in (0,2,3,5,6,7,8) or src&0xd0 or flags&0x8b or flags&3:
        raise ValueError("unsupported fixup at %d: %02x %02x"%(start,src,flags))
    if src&0x20:
        count=blob[pos];pos+=1
        if not 1<=count<=64:raise ValueError("invalid source count")
        offsets=[]
    else:
        count=1;offsets=[u16(blob,pos)];pos+=2
    obj=u16(blob,pos) if flags&0x40 else blob[pos]
    pos+=2 if flags&0x40 else 1
    target=u32(blob,pos) if flags&0x10 else u16(blob,pos)
    pos+=4 if flags&0x10 else 2
    additive=None
    if flags&4:
        additive=u32(blob,pos) if flags&0x20 else u16(blob,pos)
        pos+=4 if flags&0x20 else 2
    elif flags&0x20:raise ValueError("unexpected additive flag")
    if src&0x20:
        offsets=[u16(blob,pos+2*i) for i in range(count)];pos+=count*2
    if obj not in (1,2) or any(off>=4096 for off in offsets) or pos>len(blob):
        raise ValueError("invalid record at %d"%start)
    return pos,kind,obj,target,additive,offsets
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args();b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M368_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M368_ERROR unexpected OVL")
    ps=u32(b,LE+0x28);total=u32(b,LE+0x14)
    ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
    fixidx=LE+u32(b,LE+0x68);fixrec=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    objects=[]
    for i in range(u32(b,LE+0x44)):
        p=ot+24*i
        objects.append((u32(b,p),u32(b,p+4),u32(b,p+12),u32(b,p+16)))
    if total!=371 or ps!=4096 or len(objects)!=2:raise SystemExit("M368_ERROR unexpected LE layout")
    starts=[u32(b,fixidx+4*i) for i in range(total+1)]
    if any(a>z for a,z in zip(starts,starts[1:])):raise SystemExit("M368_ERROR fixup offsets")
    def getpage(logical):
        e=b[pm+4*(logical-1):pm+4*logical]
        number=int.from_bytes(e[:3],"big")
        if not number or e[3]:raise ValueError("unsupported page map")
        physical=origin+(number-1)*ps
        return bytearray(b[physical:physical+ps]),physical
    cs=Cs(CS_ARCH_X86,CS_MODE_32);cs.detail=True
    for name,logical,first,state0 in CASES:
        page,physical=getpage(logical)
        original=bytes(page)
        stream=b[fixrec+starts[logical-1]:fixrec+starts[logical]]
        p=0;records=0;writes=[];covered=set()
        while p<len(stream):
            end,kind,obj,target,additive,offsets=parse(stream,p)
            records+=1
            if kind!=7 or additive is not None:raise SystemExit("M368_ERROR unsupported relocation kind/additive")
            if target>=objects[obj-1][0]:raise SystemExit("M368_ERROR target outside object")
            for off in offsets:
                if off+4>ps or any(x in covered for x in range(off,off+4)):
                    raise SystemExit("M368_ERROR cross-page/overlapping relocation")
                value=objects[obj-1][1]+target
                page[off:off+4]=struct.pack("<I",value)
                covered.update(range(off,off+4))
                writes.append((off,value))
            p=end
        print("M368_PAGE name=%s logical=%d records=%d writes=%d physical=0x%x"%(name,logical,records,len(writes),physical))
        print("M368_TABLE name=%s bytes=%s"%(name,bytes(page[first:first+28]).hex()))
        targets=[]
        for i in range(7):
            linear=u32(page,first+4*i)
            # Targets for these dispatchers are known to be in object 1.
            rva=linear-objects[0][1]
            page_rva=(logical-objects[0][2])*ps
            off=rva-page_rva
            targets.append((state0+i,linear,rva,off))
        counts=Counter(linear for _,linear,_,_ in targets)
        for state,linear,rva,off in targets:
            if not 0<=off<ps:
                print("M368_HANDLER name=%s state=%d linear=0x%x rva=0x%x inPage=0"%(name,state,linear,rva));continue
            overlap=any(x in covered for x in range(off,min(ps,off+16)))
            atfixup=off in covered
            insns=list(cs.disasm(bytes(page[off:min(ps,off+80)]),rva,count=10))
            print("M368_HANDLER name=%s state=%d linear=0x%x rva=0x%x pageOffset=0x%x duplicateCount=%d startsInFixup=%d first16OverlapsFixup=%d decodeCount=%d originalFirst16=%s relocatedFirst16=%s"%
                  (name,state,linear,rva,off,counts[linear],int(atfixup),int(overlap),len(insns),
                   original[off:off+16].hex(),page[off:off+16].hex()))
            for j,ins in enumerate(insns):
                crosses=any(x in covered for x in range(ins.address-page_rva,ins.address-page_rva+ins.size))
                print("M368_INSN name=%s state=%d n=%d rva=0x%x bytes=%s mnemonic=%s operands=%s overlapsFixup=%d"%
                      (name,state,j,ins.address,ins.bytes.hex(),ins.mnemonic,ins.op_str.replace(" ","_"),int(crosses)))
        # Linear disassembly of the fully reconstructed page is a heuristic,
        # not a proven function boundary map.
        sweep=list(cs.disasm(bytes(page), (logical-objects[0][2])*ps))
        inbound=[]
        target_rvas={rva for _,_,rva,_ in targets}
        for ins in sweep:
            if not (ins.group(CS_GRP_CALL) or ins.group(CS_GRP_JUMP)):continue
            for op in ins.operands:
                if op.type==X86_OP_IMM and (op.imm&0xffffffff) in target_rvas:
                    inbound.append((ins.address,op.imm&0xffffffff,ins.mnemonic))
        print("M368_BRANCH_SWEEP name=%s decoded=%d directBranchesToHandlers=%d"%(name,len(sweep),len(inbound)))
        for src,dst,mnem in inbound[:50]:
            print("M368_BRANCH name=%s source=0x%x target=0x%x mnemonic=%s"%(name,src,dst,mnem))
    print("M368_LIMITATIONS preferredBasesAssumed=1 fullObjectRelocationNotReconstructed=1 linearSweepIsHeuristic=1 exactTargetDecodeNotProofOfBoundary=1 indirectBranchesUnresolved=1 runtimeUnverified=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
