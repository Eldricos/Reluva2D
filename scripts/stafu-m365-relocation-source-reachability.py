#!/usr/bin/env python3
"""M365: read-only direct-control-flow reachability audit for candidate LE tables.

Uses Capstone if installed for bounded recursive traversal of direct x86
branches/calls. Without Capstone, reports raw branch-target evidence only.
This cannot prove runtime reachability or absence of indirect transfers.
"""
import argparse, hashlib, struct
from collections import deque
from pathlib import Path

LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS=(("object_state",0x12334),("global_state",0x13488))
ANCHORS=(("object_dispatch",0xf579),("global_dispatch",0x106c1))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def s8(x):return x-256 if x>=128 else x
def s32(b,p):return struct.unpack_from("<i",b,p)[0]
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    ap.add_argument("--window",type=lambda x:int(x,0),default=0x1800,
                    help="RVA radius around each candidate region (default 0x1800)")
    args=ap.parse_args()
    b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M365_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M365_ERROR unexpected OVL")
    pages=u32(b,LE+0x14);ps=u32(b,LE+0x28)
    ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48);origin=LE+u32(b,LE+0x80)
    size=u32(b,ot);base=u32(b,ot+4);first=u32(b,ot+12);n=u32(b,ot+16)
    if pages!=371 or ps!=4096 or first!=1 or n!=217:raise SystemExit("M365_ERROR unexpected LE mapping")
    def physical(rva):
        if not 0<=rva<size:return None
        logical=first+rva//ps
        ent=b[pm+4*(logical-1):pm+4*logical]
        pg=int.from_bytes(ent[:3],"big")
        if not pg or ent[3]:return None
        p=origin+(pg-1)*ps+rva%ps
        return p if p<len(b) else None
    image=bytearray(size)
    for i in range(n):
        start=i*ps;end=min(size,start+ps)
        p=physical(start)
        if p is None:raise SystemExit("M365_ERROR missing object page")
        image[start:end]=b[p:p+end-start]
    print("M365_LAYOUT object1Size=0x%x preferredBase=0x%x objectPages=%d"%(size,base,n))
    for name,rva in REGIONS:
        p=physical(rva)
        print("M365_REGION name=%s rva=0x%x physical=0x%x length=28 bytes=%s"%
              (name,rva,p,bytes(image[rva:rva+28]).hex()))
    # Raw direct branch references: E8/E9 rel32, EB rel8, short Jcc,
    # 0F 8x rel32. These are byte-pattern candidates, not decoded instructions.
    for name,rva in REGIONS:
        lo=max(0,rva-args.window);hi=min(size,rva+28+args.window)
        hits=[]
        for p in range(lo,hi):
            op=image[p];target=None;kind=None
            if op in (0xe8,0xe9) and p+5<=size:
                target=p+5+s32(image,p+1);kind="call32" if op==0xe8 else "jmp32"
            elif op==0xeb and p+2<=size:
                target=p+2+s8(image[p+1]);kind="jmp8"
            elif 0x70<=op<=0x7f and p+2<=size:
                target=p+2+s8(image[p+1]);kind="jcc8"
            elif op==0x0f and p+6<=size and 0x80<=image[p+1]<=0x8f:
                target=p+6+s32(image,p+2);kind="jcc32"
            if target is not None and rva<=target<rva+28:
                hits.append((p,target,kind))
        print("M365_RAW_BRANCH_SCAN name=%s windowStart=0x%x windowEnd=0x%x hits=%d"%
              (name,lo,hi,len(hits)))
        for source,target,kind in hits[:40]:
            print("M365_RAW_BRANCH name=%s source=0x%x target=0x%x kind=%s"%
                  (name,source,target,kind))
    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_JUMP,CS_GRP_CALL,CS_GRP_RET
        from capstone.x86_const import X86_OP_IMM
    except ImportError:
        print("M365_CAPSTONE available=0 recursiveTraversal=skipped installHint=python_-m_pip_install_capstone")
        print("M365_LIMITATIONS rawBranchScanHasFalsePositives=1 directCFGNotRun=1 indirectTransfersUnresolved=1 dynamicReachabilityUnproven=1 SPRDecoderUnverified=1")
        return
    cs=Cs(CS_ARCH_X86,CS_MODE_32);cs.detail=True
    print("M365_CAPSTONE available=1")
    for name,rva in REGIONS:
        lo=max(0,rva-args.window);hi=min(size,rva+28+args.window)
        # Nearby entry seeds are heuristics, not verified function entries.
        seeds=[("dispatcher",x) for _,x in ANCHORS if lo<=x<hi]
        seeds += [("window_start",lo)]
        # Direct branches into the region are added only if their own sources
        # are decoded on a reachable traversal path, not from raw scanning.
        q=deque((seed,label) for label,seed in seeds)
        visited=set();edges=[];overlaps=[];steps=0;limit=100000
        while q and steps<limit:
            pc,originlabel=q.popleft()
            if not lo<=pc<hi or pc in visited:continue
            while lo<=pc<hi and pc not in visited and steps<limit:
                ins=next(iter(cs.disasm(bytes(image[pc:min(hi,pc+15)]),pc,count=1)),None)
                if ins is None or ins.size==0:break
                visited.add(pc);steps+=1
                end=pc+ins.size
                if pc<rva+28 and end>rva:
                    overlaps.append((pc,end,ins.mnemonic,ins.op_str,originlabel))
                groups=ins.groups
                jump=CS_GRP_JUMP in groups
                call=CS_GRP_CALL in groups
                ret=CS_GRP_RET in groups
                imm=None
                for op in ins.operands:
                    if op.type==X86_OP_IMM:imm=op.imm & 0xffffffff;break
                if (jump or call) and imm is not None:
                    # Capstone reports immediate branch targets as decoded
                    # instruction addresses; they are RVAs here.
                    target=imm
                    if lo<=target<hi:
                        edges.append((pc,target,ins.mnemonic))
                        q.append((target,"direct_target"))
                if ret or jump and ins.mnemonic=="jmp":break
                if jump and ins.mnemonic!="jmp":
                    pc=end;continue
                pc=end
        inbound=[(src,dst,mnem) for src,dst,mnem in edges if rva<=dst<rva+28]
        print("M365_CFG name=%s seeds=%s decodedStarts=%d directEdges=%d inboundDirect=%d overlappingInstructions=%d traversalLimitHit=%d"%
              (name,",".join("%s:0x%x"%(label,seed) for label,seed in seeds),
               len(visited),len(edges),len(inbound),len(overlaps),int(steps>=limit)))
        for src,dst,mnem in inbound[:40]:
            print("M365_CFG_INBOUND name=%s source=0x%x target=0x%x mnemonic=%s"%(name,src,dst,mnem))
        for start,end,mnem,operands,originlabel in overlaps[:40]:
            print("M365_CFG_OVERLAP name=%s start=0x%x end=0x%x mnemonic=%s operands=%s seedOrigin=%s"%
                  (name,start,end,mnem,operands.replace(" ","_"),originlabel))
    print("M365_LIMITATIONS heuristicEntrySeeds=1 boundedCFG=1 indirectTransfersUnresolved=1 selectorBaseUnverified=1 dynamicReachabilityUnproven=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
