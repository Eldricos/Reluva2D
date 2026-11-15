#!/usr/bin/env python3
"""M441: bound the stream-read path and rank its callers (read-only).
Usage: python scripts/stafu-m441-stream-read-consumers.py games/STTNG/STTNG.OVL > m441-stream-read-consumers.txt
Requires capstone. All addresses are STTNG.OVL file offsets, not runtime addresses.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32

EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
REGION_LO,REGION_HI=0x7E680,0x7E940
READ_SITE=0x7E7B3
READ_TARGET=0x11ACD0

def direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None

def walk(md,data,start,end,limit=450):
    pending=deque([start]); seen=set(); out={}; edges=[]
    while pending and len(out)<limit:
        pc=pending.popleft()
        while start<=pc<end and pc not in seen and len(out)<limit:
            seen.add(pc)
            decoded=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
            if not decoded:break
            i=decoded[0];out[pc]=i; nxt=pc+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                target=direct(i)
                if target is not None and start<=target<end:
                    edges.append((pc,target));pending.append(target)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                target=direct(i)
                if target is not None and start<=target<end:
                    edges.append((pc,target));pending.append(target)
            pc=nxt
    return out,edges

def raw_callers(data,target):
    matches=[]
    for p in range(LO,HI-4):
        if data[p]==0xE8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target:
            matches.append(p)
    return matches

def show_context(md,data,site,span=56):
    print(f"M441_CALLER_SITE file={site:#x} preceding={data[max(LO,site-16):site].hex()}")
    for back in (48,40,32,24,16,8):
        begin=max(LO,site-back)
        instructions=list(md.disasm(data[begin:site+5],begin))
        if instructions and instructions[-1].address==site and instructions[-1].mnemonic=="call":
            print(f"M441_CALLER_CONTEXT site={site:#x} candidateStart={begin:#x} aligned=1")
            for i in instructions[-14:]:
                print(f"M441_CONTEXT file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            return
    print(f"M441_CALLER_CONTEXT site={site:#x} aligned=0")

def main():
    if len(sys.argv)!=2:
        print(__doc__);return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M441_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    print(f"M441_REGION start={REGION_LO:#x} end={REGION_HI:#x} readSite={READ_SITE:#x}")
    print(f"M441_READ_SITE_BYTES bytes={data[READ_SITE:READ_SITE+5].hex()}")
    if data[READ_SITE]==0xE8:
        target=READ_SITE+5+struct.unpack_from("<i",data,READ_SITE+1)[0]
        print(f"M441_READ_SITE_REL32 target={target:#x} expected={READ_TARGET:#x} matches={target==READ_TARGET}")
    else:print("M441_WARNING expected_read_call_opcode_absent")
    # Enumerate plausible entry points without assuming that a prologue proves a function boundary.
    starts=set([REGION_LO])
    for p in range(REGION_LO,READ_SITE+1):
        if data[p:p+3]==b"\\x55\\x8b\\xec" or data[p:p+3]==b"\\x55\\x89\\xe5":
            starts.add(p)
    # Direct-call destinations into this region are additional entry candidates.
    destinations={}
    for p in range(LO,HI-4):
        if data[p]!=0xE8:continue
        dst=p+5+struct.unpack_from("<i",data,p+1)[0]
        if REGION_LO<=dst<=READ_SITE:
            destinations.setdefault(dst,[]).append(p)
            starts.add(dst)
    print(f"M441_ENTRY_CANDIDATES count={len(starts)}")
    candidates=[]
    for start in sorted(starts):
        ins,edges=walk(md,data,start,REGION_HI)
        contains=READ_SITE in ins and ins[READ_SITE].mnemonic=="call" and direct(ins[READ_SITE])==READ_TARGET
        calls=destinations.get(start,[])
        print(f"M441_ENTRY entry={start:#x} decoded={len(ins)} reachesRead={contains} rawInbound={len(calls)} last={max(ins) if ins else 0:#x}")
        if contains:candidates.append((start,ins,edges))
    if not candidates:
        print("M441_WARNING no_entry_reaches_read_site; fallback_linear_window")
        for i in md.disasm(data[REGION_LO:REGION_HI],REGION_LO):
            if READ_SITE-100<=i.address<=READ_SITE+90:
                print(f"M441_FALLBACK file={i.address:#x} asm={i.mnemonic} {i.op_str}")
    else:
        # Prefer nearest entry with direct inbound calls; retain all candidates for review.
        candidates.sort(key=lambda c:(bool(destinations.get(c[0])),c[0]),reverse=True)
        for rank,(entry,ins,edges) in enumerate(candidates[:4],1):
            print(f"M441_REACHABLE_FUNCTION rank={rank} entry={entry:#x} decoded={len(ins)} edges={len(edges)} inbound={len(destinations.get(entry,[]))}")
            for addr,i in sorted(ins.items()):
                print(f"M441_INSN entry={entry:#x} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
            for src,dst in edges:print(f"M441_EDGE entry={entry:#x} from={src:#x} to={dst:#x}")
            callers=raw_callers(data,entry)
            print(f"M441_RAW_CALLERS entry={entry:#x} count={len(callers)}")
            for site in callers[:60]:show_context(md,data,site)
            if len(callers)>60:print(f"M441_OMITTED entry={entry:#x} count={len(callers)-60}")
    print("M441_DIRECT_DESTINATIONS_IN_REGION")
    for dst,sites in sorted(destinations.items()):
        print(f"M441_DEST target={dst:#x} count={len(sites)} callers={','.join(hex(p) for p in sites[:40])}")
    print("M441_CAUTION raw E8 references, prologue matches and bounded CFG walks are hypotheses; buffer ownership and SPR decoding remain unproven.")
    print("M441_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
