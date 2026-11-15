#!/usr/bin/env python3
"""STAFU M442: trace buffered stream reader state and consumers (read-only).
Usage: python scripts/stafu-m442-buffered-reader-consumers.py games/STTNG/STTNG.OVL > m442-buffered-reader-consumers.txt
Requires capstone. Addresses are physical file offsets; raw references are not proof of runtime execution.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
ENTRIES=[("buffered_reader_a",0x89800,0x89A40),("buffered_reader_b",0x89A40,0x89C80)]
GLOBALS={0x9EE68:"buffer_pointer",0x9EE6C:"buffer_remaining_or_limit",0x12F78:"stream_index_or_handle"}
def direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None
def walk(md,data,start,end,limit=350):
    todo=deque([start]);seen=set();out={}
    while todo and len(out)<limit:
        pc=todo.popleft()
        while start<=pc<end and pc not in seen and len(out)<limit:
            seen.add(pc)
            found=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
            if not found:break
            i=found[0];out[pc]=i;nextpc=pc+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                dest=direct(i)
                if dest is not None and start<=dest<end:todo.append(dest)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                dest=direct(i)
                if dest is not None and start<=dest<end:todo.append(dest)
            pc=nextpc
    return out
def raw_calls(data,target):
    return [p for p in range(LO,HI-4) if data[p]==0xE8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]
def context(md,data,site):
    print(f"M442_CALLER site={site:#x} beforeHex={data[max(LO,site-24):site].hex()}")
    for back in (48,40,32,24,16,8):
        start=site-back
        if start<LO:continue
        ins=list(md.disasm(data[start:site+5],start))
        if ins and ins[-1].address==site and ins[-1].mnemonic=="call":
            print(f"M442_CONTEXT_CANDIDATE site={site:#x} start={start:#x}")
            for i in ins[-14:]:print(f"M442_PRE site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            break
    for i in md.disasm(data[site+5:site+45],site+5):
        print(f"M442_POST site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
        if i.address>=site+24 or i.mnemonic.startswith("ret"):break
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M442_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    # Identify function boundaries using actual call targets and local prologues, not a fixed guess.
    region_start,region_end=0x897D0,0x89C80
    incoming={}
    for p in range(LO,HI-4):
        if data[p]!=0xE8:continue
        dst=p+5+struct.unpack_from("<i",data,p+1)[0]
        if region_start<=dst<region_end:incoming.setdefault(dst,[]).append(p)
    starts=set(incoming)
    for p in range(region_start,region_end-2):
        if data[p:p+3] in (b"\x55\x8b\xec",b"\x55\x89\xe5"):starts.add(p)
    starts.update([0x897D0,0x8998C,0x89A40])
    print(f"M442_ENTRY_CANDIDATES count={len(starts)}")
    for start in sorted(starts):
        ins=walk(md,data,start,region_end)
        hits=[a for a in (0x899A9,0x89A7E) if a in ins]
        print(f"M442_ENTRY entry={start:#x} decoded={len(ins)} inbound={len(incoming.get(start,[]))} readSites={','.join(hex(x) for x in hits) or 'none'}")
        if hits or incoming.get(start):
            for addr,i in sorted(ins.items()):
                print(f"M442_INSN entry={start:#x} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
            callers=incoming.get(start,[])
            print(f"M442_RAW_CALLERS entry={start:#x} count={len(callers)}")
            for site in callers[:35]:context(md,data,site)
            if len(callers)>35:print(f"M442_CALLERS_OMITTED entry={start:#x} count={len(callers)-35}")
    # Raw absolute references to global buffer state; classify by instruction decoding where possible.
    for address,label in GLOBALS.items():
        needle=struct.pack("<I",address)
        hits=[]
        pos=LO
        while True:
            p=data.find(needle,pos,HI)
            if p<0:break
            hits.append(p);pos=p+1
        print(f"M442_GLOBAL_REF name={label} address={address:#x} rawCount={len(hits)}")
        for p in hits[:75]:
            print(f"M442_GLOBAL_HIT name={label} operandFile={p:#x} before={data[max(LO,p-10):p].hex()} after={data[p+4:p+12].hex()}")
        if len(hits)>75:print(f"M442_GLOBAL_OMITTED name={label} count={len(hits)-75}")
    print("M442_CAUTION addresses and reference matches are static hypotheses; no SPR decoder or bitstream consumer is yet established.")
    print("M442_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
