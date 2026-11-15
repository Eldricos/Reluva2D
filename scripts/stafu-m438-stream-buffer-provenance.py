#!/usr/bin/env python3
"""STAFU M438: inspect candidate stream acquisition and buffer transfer helpers.
Usage: python scripts/stafu-m438-stream-buffer-provenance.py games/STTNG/STTNG.OVL > m438-stream-buffer-provenance.txt
Requires capstone. Read-only; all addresses are OVL file offsets, not runtime addresses.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
FUNCTIONS=[("resource_access_7D560",0x7D560,0x7D900),("handle_acquire_11A9CE",0x11A9CE,0x11AD00),("resource_lookup_7E034",0x7E034,0x7E410),("reference_prepare_7D2B0",0x7D2B0,0x7D560)]
def direct(i):
    try:return int(i.op_str,16) if i.op_str.startswith("0x") else None
    except ValueError:return None
def cfg(md,data,start,end,cap=500):
    todo=deque([start]);seen=set();instructions={}
    while todo and len(instructions)<cap:
        p=todo.popleft()
        while start<=p<end and p not in seen and len(instructions)<cap:
            seen.add(p)
            found=list(md.disasm(data[p:min(p+15,end)],p,count=1))
            if not found:break
            i=found[0];instructions[p]=i;n=p+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iretd","hlt","ud2"):break
            if i.mnemonic=="jmp":
                d=direct(i)
                if d is not None and start<=d<end:todo.append(d)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                d=direct(i)
                if d is not None and start<=d<end:todo.append(d)
            p=n
    return instructions
def dump(md,data,name,start,end):
    ins=cfg(md,data,start,end)
    print(f"M438_FUNCTION name={name} entry={start:#x} bound={end:#x} decoded={len(ins)}")
    for addr,i in sorted(ins.items()):
        print(f"M438_INSN name={name} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
        if i.mnemonic.startswith("call"):print(f"M438_CALL name={name} file={addr:#x} target={i.op_str}")
        if i.mnemonic.startswith("rep") or i.mnemonic.startswith("movs") or i.mnemonic.startswith("stos"):
            print(f"M438_TRANSFER_CANDIDATE name={name} file={addr:#x} asm={i.mnemonic} {i.op_str}")
    if len(ins)>=500:print(f"M438_WARNING name={name} instruction_cap")
def caller_sites(data,target):
    return [p for p in range(LO,HI-4) if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]
def context(md,data,site):
    print(f"M438_RAW_CALLSITE file={site:#x} before={data[max(LO,site-48):site].hex()} after={data[site+5:site+21].hex()}")
    for offset in (40,32,24,16,8):
        start=site-offset
        if start<LO:continue
        seq=list(md.disasm(data[start:site+5],start))
        if seq and seq[-1].address==site and seq[-1].mnemonic=="call":
            print(f"M438_POSSIBLE_ALIGNMENT anchor={start:#x} site={site:#x}")
            for i in seq[-14:]:print(f"M438_CONTEXT file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            return
    print(f"M438_ALIGNMENT_UNRESOLVED site={site:#x}")
def main():
    if len(sys.argv)!=2:
        print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M438_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,start,end in FUNCTIONS:dump(md,data,name,start,end)
    for name,start,end in FUNCTIONS:
        hits=caller_sites(data,start)
        print(f"M438_CALLERS name={name} target={start:#x} rawCount={len(hits)}")
        for site in hits[:25]:context(md,data,site)
        if len(hits)>25:print(f"M438_OMITTED name={name} count={len(hits)-25}")
    print("M438_CAUTION raw E8 hits, inferred boundaries, and alignment anchors are static hypotheses, not execution evidence.")
    print("M438_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
