#!/usr/bin/env python3
"""STAFU M410: segment-prefixed indirect export calls and push-run verification.

Read-only, standard library only. File offsets are static, not runtime addresses.
Decodes the complete prefixed FF /3 call and conservatively recognizes adjacent
16-bit PUSH instructions; it does not establish reachability or ABI semantics.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LE_OFFSET = 0x38194
EXPORTS = {
    "VMFABIND": 0x1A04, "VMFMA": 0x1A10, "VMFILEIO": 0x1A08,
    "DVX_Global_Seg_Associate": 0x3290,
    "DVX_Global_Seg_Alloc": 0x32AC,
    "DVX_Global_Seg_Broadcast": 0x32B0,
    "DVX_Dos4gx_Get_Option": 0x32A8,
    "VMALOCINST": 0x3294, "VSUPERRESIZE": 0x3298,
    "VSUPERFREE": 0x329C, "DOS4G_GETENV": 0x32A0,
    "DOS4G_GET_APPPATH": 0x32A4,
}
SEGMENT = {0x26:"es",0x2e:"cs",0x36:"ss",0x3e:"ds"}
REG = ("ax","cx","dx","bx","sp","bp","si","di")
BASE = ("bx+si","bx+di","bp+si","bp+di","si","di",None,"bx")

def u16(b, pos):
    return struct.unpack_from("<H",b,pos)[0]

def signed8(x):
    return x-256 if x>=128 else x

def signed16(x):
    return x-65536 if x>=32768 else x

def modrm_operand(b,pos):
    """Return (operand, new_position) for a 16-bit ModR/M memory operand."""
    if pos>=LE_OFFSET:
        return None
    m=b[pos];pos+=1
    mod,rm=m>>6,m&7
    if mod==3:
        return (REG[rm],pos)
    if mod==0 and rm==6:
        if pos+2>LE_OFFSET:return None
        return ("[%04xh]"%u16(b,pos),pos+2)
    base=BASE[rm] if rm!=6 else "bp"
    disp=0
    if mod==1:
        if pos>=LE_OFFSET:return None
        disp=signed8(b[pos]);pos+=1
    elif mod==2:
        if pos+2>LE_OFFSET:return None
        disp=signed16(u16(b,pos));pos+=2
    return ("["+base+(("%+d"%disp) if disp else "")+"]",pos)

def push_at(b,start,end):
    """Recognize exactly one PUSH occupying [start,end); no fuzzy byte matches."""
    p=start
    if p>=end:return None
    override=None
    if b[p] in SEGMENT:
        override=SEGMENT[b[p]];p+=1
    if p>=end:return None
    op=b[p];p+=1
    if 0x50<=op<=0x57 and not override and p==end:
        return "push "+REG[op-0x50]
    if op in (0x06,0x0e,0x16,0x1e) and not override and p==end:
        return "push "+{0x06:"es",0x0e:"cs",0x16:"ss",0x1e:"ds"}[op]
    if op==0x68 and not override and p+2==end:
        return "push 0x%x"%u16(b,p)
    if op==0x6a and not override and p+1==end:
        return "push %d"%signed8(b[p])
    if op==0xff and p<end and (b[p]>>3)&7==6:
        parsed=modrm_operand(b,p)
        if parsed and parsed[1]==end:
            return "push "+((override+":") if override else "")+parsed[0]
    return None

def push_chains(b,call_start,max_bytes=36,max_pushes=12):
    """Enumerate alternative adjacent push chains without assuming earlier boundaries."""
    result=[[]]
    stack=[(call_start,[])]
    while stack:
        cursor,reverse=stack.pop()
        if reverse:result.append(list(reversed(reverse)))
        if len(reverse)>=max_pushes or call_start-cursor>=max_bytes:continue
        for n in range(1,min(6,cursor, max_bytes-(call_start-cursor))+1):
            start=cursor-n
            decoded=push_at(b,start,cursor)
            if decoded:
                stack.append((start,[(start,cursor,decoded)]+reverse[::-1] if False else reverse+[(start,cursor,decoded)]))
    # Dedupe; rank by number of pushes, but do not treat the longest as authoritative.
    unique={}
    for chain in result:
        key=tuple((s,e,t) for s,e,t in chain)
        unique[key]=chain
    return sorted(unique.values(),key=lambda c:(-len(c),c[0][0] if c else call_start))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl",type=Path)
    b=parser.parse_args().ovl.read_bytes()
    print("M410_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if len(b)<LE_OFFSET+2 or b[:2]!=b"MZ" or b[LE_OFFSET:LE_OFFSET+2]!=b"LE":
        raise SystemExit("M410_ERROR unexpected MZ/LE layout")
    total=0;prefixed=0
    for name,offset in EXPORTS.items():
        pattern=b"\xff\x1e"+struct.pack("<H",offset)
        sites=[];pos=0
        while True:
            pos=b.find(pattern,pos,LE_OFFSET)
            if pos<0:break
            sites.append(pos);pos+=1
        print("M410_EXPORT name=%s dsOffset=0x%04x matches=%d"%(name,offset,len(sites)))
        for site in sites:
            total+=1
            # One segment override is legal; report multiple consecutive overrides
            # as ambiguous rather than assuming a reliable effective segment.
            p=site
            prefixes=[]
            while p>0 and b[p-1] in SEGMENT and len(prefixes)<4:
                p-=1;prefixes.insert(0,b[p])
            segment=SEGMENT[prefixes[-1]] if len(prefixes)==1 else "ds" if not prefixes else "ambiguous"
            if prefixes:prefixed+=1
            start=p
            print("M410_CALL name=%s opcodeFile=0x%x instructionStart=0x%x instructionLength=%d prefixes=%s pointerSegment=%s operand=[%04xh] bytes=%s"%(
                name,site,start,site+4-start,
                ",".join("%02x"%v for v in prefixes) or "none",
                segment,offset,b[start:site+4].hex()))
            if len(prefixes)>1:
                print("M410_WARNING opcodeFile=0x%x multipleAdjacentSegmentBytes=1 instructionStartTentative=1"%site)
            chains=push_chains(b,start)
            # Candidate sequences can extend into unrelated bytes. Report both
            # the longest and all distinct candidate push counts.
            best=chains[0]
            counts=sorted(set(len(c) for c in chains))
            after=b[site+4:site+12]
            cleanup=None;cleanup_kind="unknown"
            if after[:2]==b"\x83\xc4":
                cleanup=after[2];cleanup_kind="add_sp"
            elif after[:2]==b"\x5b\x5b":
                cleanup=4;cleanup_kind="pop_bx_twice"
            print("M410_PUSH_CANDIDATES opcodeFile=0x%x counts=%s longest=%d longestBytes=%d cleanup=%s cleanupBytes=%s longestMatches=%s"%(
                site,",".join(map(str,counts)),len(best),2*len(best),
                cleanup_kind,str(cleanup) if cleanup is not None else "unknown",
                str(2*len(best)==cleanup) if cleanup is not None else "unknown"))
            for i,(s,e,ins) in enumerate(best):
                print("M410_PUSH opcodeFile=0x%x index=%d file=0x%x len=%d text=%s"%(site,i,s,e-s,ins))
            if cleanup is not None:
                matching=[c for c in chains if 2*len(c)==cleanup]
                print("M410_CLEANUP_MATCH opcodeFile=0x%x matchingChains=%d bestMatchingStart=%s"%(
                    site,len(matching),hex(matching[0][0][0]) if matching and matching[0] else "none"))
            print("M410_AFTER opcodeFile=0x%x bytes=%s"%(site,after.hex()))
    print("M410_TOTAL exports=%d callMatches=%d prefixedCalls=%d"%(len(EXPORTS),total,prefixed))
    print("M410_LIMITATIONS adjacentPrefixHeuristic=1 arbitraryPushBoundaries=1 controlFlowUnverified=1 runtimeSegmentValueUnverified=1 argumentMeaningUnverified=1")
if __name__=="__main__":
    main()
