#!/usr/bin/env python3
"""M440: rank direct callers of confirmed DOS read wrapper and trace buffer provenance.
Read-only; file offsets, not runtime addresses. Requires capstone.
Usage: python scripts/stafu-m440-dos-read-caller-provenance.py games/STTNG/STTNG.OVL > m440-dos-read-caller-provenance.txt
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32

EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
READ=0x11ACD0
OPEN=0x11A9F0
SEEK=0x11AE03
TARGETS={READ:"dos_read",OPEN:"dos_open",SEEK:"dos_seek"}

def raw_calls(data,target):
    return [p for p in range(LO,min(HI-4,len(data)-4)) if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]

def direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None

def linear_context(md,data,site,before=96,after=128):
    """Find plausible instruction alignment from several starts; never assert executable provenance."""
    candidates=[]
    for delta in range(before,3,-1):
        start=site-delta
        if start<LO:continue
        ins=list(md.disasm(data[start:min(len(data),site+5)],start))
        if ins and ins[-1].address==site and ins[-1].mnemonic=="call" and direct(ins[-1])==READ:
            score=sum(i.mnemonic not in ("int3","ud2","hlt") for i in ins)
            candidates.append((score,len(ins),start,ins))
    if not candidates:return None
    _,_,start,ins=max(candidates,key=lambda t:(t[0],t[1]))
    return start,ins

def follow_window(md,data,site,span=128):
    end=min(HI,site+5+span)
    pc=site+5
    out=[]
    while pc<end and len(out)<48:
        v=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
        if not v:break
        i=v[0];out.append(i);pc+=i.size
        if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","ud2","hlt"):break
        if i.mnemonic=="jmp":break
    return out

def scan_local_calls(data,site,radius=512):
    left=max(LO,site-radius);right=min(HI-4,site+radius)
    found=[]
    for p in range(left,right):
        if data[p]!=0xe8:continue
        dest=p+5+struct.unpack_from("<i",data,p+1)[0]
        if dest in TARGETS:
            found.append((p,dest,TARGETS[dest]))
    return found

def main():
    if len(sys.argv)!=2:
        print(__doc__);return 2
    with open(sys.argv[1],"rb") as f:data=f.read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M440_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    sites=raw_calls(data,READ)
    print(f"M440_READ_TARGET file={READ:#x} rawCallers={len(sites)}")
    for n,site in enumerate(sites,1):
        print(f"M440_CALLER index={n} site={site:#x} bytesBefore={data[max(LO,site-32):site].hex()} bytesAfter={data[site+5:site+21].hex()}")
        ctx=linear_context(md,data,site)
        if ctx is None:
            print(f"M440_CONTEXT_UNCERTAIN site={site:#x}")
        else:
            start,ins=ctx
            print(f"M440_CONTEXT_CANDIDATE site={site:#x} start={start:#x} instructions={len(ins)} caution=alignment_not_verified")
            for i in ins[-24:]:
                print(f"M440_BEFORE site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
        for i in follow_window(md,data,site):
            print(f"M440_AFTER site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
        nearby=scan_local_calls(data,site)
        print(f"M440_NEARBY_DOS_CALLS site={site:#x} radius=512 count={len(nearby)}")
        for p,dest,label in nearby[:24]:
            print(f"M440_NEARBY site={site:#x} callsite={p:#x} target={dest:#x} name={label} delta={p-site:+d}")
        if len(nearby)>24:print(f"M440_NEARBY_OMITTED site={site:#x} count={len(nearby)-24}")
    print("M440_CAUTION raw E8 matches, speculative alignment and sequential post-call disassembly do not prove execution. A DOS read may be followed by checks before any decoder.")
    print("M440_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
