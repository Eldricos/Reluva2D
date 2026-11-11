#!/usr/bin/env python3
"""M366: independently validate candidate branch instructions and local CFG seeds.

Read-only. Uses Capstone. Direct-call targets are instruction-derived entry
candidates, not verified runtime entries. Reports the distinction explicitly.
"""
import argparse,hashlib,struct
from collections import deque
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_JUMP,CS_GRP_CALL,CS_GRP_RET
from capstone.x86_const import X86_OP_IMM

LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=(("object_state",0x12334,0x12334+28,(0x12312,0x1231d,0x12326,0x1233f)),
         ("global_state",0x13488,0x13488+28,(0x1348d,)))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    ap.add_argument("--radius",type=lambda s:int(s,0),default=0x2000)
    a=ap.parse_args();b=a.ovl.read_bytes();digest=hashlib.sha256(b).hexdigest()
    print("M366_INPUT bytes=%d sha256=%s"%(len(b),digest))
    if len(b)!=2008455 or digest!=SHA or b[LE:LE+2]!=b"LE":
        raise SystemExit("M366_ERROR unexpected OVL")
    ps=u32(b,LE+0x28);ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
    origin=LE+u32(b,LE+0x80);size=u32(b,ot);first=u32(b,ot+12);pages=u32(b,ot+16)
    if ps!=4096 or first!=1 or pages!=217:raise SystemExit("M366_ERROR unexpected mapping")
    data=bytearray(size)
    for i in range(pages):
        e=b[pm+4*i:pm+4*i+4];number=int.from_bytes(e[:3],"big")
        if not number or e[3]:raise SystemExit("M366_ERROR unsupported page")
        length=min(ps,size-i*ps)
        p=origin+(number-1)*ps
        data[i*ps:i*ps+length]=b[p:p+length]
    cs=Cs(CS_ARCH_X86,CS_MODE_32);cs.detail=True
    def insn(pc):
        if pc<0 or pc>=size:return None
        return next(iter(cs.disasm(bytes(data[pc:min(size,pc+15)]),pc,count=1)),None)
    def target(ins):
        if not ins or not (ins.group(CS_GRP_JUMP) or ins.group(CS_GRP_CALL)):return None
        for op in ins.operands:
            if op.type==X86_OP_IMM:return op.imm&0xffffffff
        return None
    def traverse(seeds,lo,hi,limit=200000):
        q=deque(seeds);seen=set();edges=[];overlap=[];indirect=[]
        while q and len(seen)<limit:
            pc,seed=q.popleft()
            while lo<=pc<hi and pc not in seen and len(seen)<limit:
                ins=insn(pc)
                if ins is None or pc+ins.size>hi:break
                seen.add(pc)
                for _,start,end in REGIONS:
                    if pc<end and pc+ins.size>start:overlap.append((pc,pc+ins.size,ins.mnemonic,seed))
                isjump=ins.group(CS_GRP_JUMP);iscall=ins.group(CS_GRP_CALL)
                dst=target(ins)
                if isjump or iscall:
                    if dst is None:indirect.append((pc,ins.mnemonic))
                    else:
                        edges.append((pc,dst,ins.mnemonic))
                        if lo<=dst<hi:q.append((dst,"direct_from_0x%x"%pc))
                if ins.group(CS_GRP_RET) or isjump and ins.mnemonic=="jmp":break
                pc+=ins.size
        return seen,edges,overlap,indirect,len(seen)>=limit
    print("M366_LAYOUT object1Size=0x%x radius=0x%x"%(size,a.radius))
    for name,start,end,raw_sources in REGIONS:
        lo=max(0,start-a.radius);hi=min(size,end+a.radius)
        print("M366_REGION name=%s start=0x%x end=0x%x window=0x%x..0x%x"%(name,start,end,lo,hi))
        # Identify direct CALL candidates by a linear sweep over the bounded
        # window; this sweep can decode data as code and is not authoritative.
        sweep=list(cs.disasm(bytes(data[lo:hi]),lo))
        calls=[]
        for ins in sweep:
            if ins.group(CS_GRP_CALL):
                dst=target(ins)
                if dst is not None and lo<=dst<hi:
                    calls.append((ins.address,dst))
        call_targets=sorted(set(dst for _,dst in calls))
        prologue_targets=[dst for dst in call_targets if bytes(data[dst:dst+3]) in (b"\x55\x8b\xec",b"\x55\x89\xe5")]
        print("M366_CALL_SEEDS name=%s linearSweepCalls=%d distinctTargets=%d prologueTargets=%d"%
              (name,len(calls),len(call_targets),len(prologue_targets)))
        for src,dst in calls[:40]:
            print("M366_CALL_CANDIDATE name=%s source=0x%x target=0x%x targetPrologue=%d"%
                  (name,src,dst,int(dst in prologue_targets)))
        # Compare each raw hit against a disassembly from nearby plausible
        # instruction boundaries, without claiming the paths are executable.
        for source in raw_sources:
            exact=insn(source)
            print("M366_RAW_SOURCE name=%s source=0x%x standalone=%s target=%s"%
                  (name,source,exact.mnemonic+"_"+exact.op_str.replace(" ","_") if exact else "none",
                   "0x%x"%target(exact) if target(exact) is not None else "none"))
            for back in range(1,9):
                begin=source-back
                if begin<lo:continue
                decoded=list(cs.disasm(bytes(data[begin:min(hi,source+16)]),begin))
                crossing=next((x for x in decoded if x.address<=source<x.address+x.size),None)
                if crossing is not None and crossing.address<source:
                    print("M366_BOUNDARY_ALTERNATIVE name=%s source=0x%x decodeFrom=0x%x crossingStart=0x%x crossingEnd=0x%x instruction=%s"%
                          (name,source,begin,crossing.address,crossing.address+crossing.size,crossing.mnemonic))
        # Prologue-bearing direct-call targets are the strongest available
        # local static seeds. Include raw call targets separately for coverage.
        for label,targets in (("prologue_call_targets",prologue_targets),
                              ("all_direct_call_targets",call_targets)):
            seeds=[(x,label) for x in targets[:300]]
            seen,edges,overlaps,indirect,limited=traverse(seeds,lo,hi)
            inbound=[(src,dst,mnem) for src,dst,mnem in edges if start<=dst<end]
            local=[x for x in overlaps if x[0]<end and x[1]>start]
            print("M366_CFG name=%s seedClass=%s seeds=%d visited=%d directEdges=%d inbound=%d overlaps=%d indirect=%d limitHit=%d"%
                  (name,label,len(seeds),len(seen),len(edges),len(inbound),len(local),len(indirect),int(limited)))
            for src,dst,mnem in inbound[:30]:
                print("M366_CFG_INBOUND name=%s seedClass=%s source=0x%x target=0x%x mnemonic=%s"%(name,label,src,dst,mnem))
            for p,q,mnem,seed in local[:30]:
                print("M366_CFG_OVERLAP name=%s seedClass=%s start=0x%x end=0x%x mnemonic=%s origin=%s"%(name,label,p,q,mnem,seed))
        print("M366_REGION_END name=%s"%name)
    print("M366_LIMITATIONS callSeedsDerivedFromLinearSweep=1 noVerifiedRuntimeFunctionEntries=1 noIndirectTargetResolution=1 noRuntimeReachabilityProof=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
