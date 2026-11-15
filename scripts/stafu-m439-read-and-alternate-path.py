#!/usr/bin/env python3
"""STAFU M439: trace byte-read candidate and alternate resource acquisition.
Usage: python scripts/stafu-m439-read-and-alternate-path.py games/STTNG/STTNG.OVL > m439-read-and-alternate-path.txt
Requires capstone; read-only; addresses are physical OVL file offsets.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
TARGETS=[("read_candidate",0x11ACD0,0x11AE03),("open_backend",0x11A9F0,0x11AC38),
("alternate_backend",0x11AE03,0x11AEB4),("alternate_selector",0x7D7AC,0x7DA40)]
def target(i):
    try:return int(i.op_str,16) if i.op_str.startswith("0x") else None
    except ValueError:return None
def walk(md,data,entry,end,cap=500):
    pending=deque([entry]);seen=set();result={}
    while pending and len(result)<cap:
        p=pending.popleft()
        while entry<=p<end and p not in seen and len(result)<cap:
            seen.add(p);found=list(md.disasm(data[p:min(end,p+15)],p,count=1))
            if not found:break
            i=found[0];result[p]=i;n=p+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iretd","hlt","ud2"):break
            if i.mnemonic=="jmp":
                d=target(i)
                if d is not None and entry<=d<end:pending.append(d)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                d=target(i)
                if d is not None and entry<=d<end:pending.append(d)
            p=n
    return result
def show(md,data,name,start,end):
    instructions=walk(md,data,start,end)
    print(f"M439_FUNCTION name={name} entry={start:#x} bound={end:#x} decoded={len(instructions)}")
    for a,i in sorted(instructions.items()):
        print(f"M439_INSN function={name} file={a:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
        if i.mnemonic.startswith("call"):print(f"M439_CALL function={name} site={a:#x} target={i.op_str}")
        if i.mnemonic.startswith(("rep","movs","stos","lods")):print(f"M439_TRANSFER function={name} site={a:#x} asm={i.mnemonic} {i.op_str}")
    if len(instructions)>=500:print(f"M439_WARNING function={name} instruction_cap")
def raw_calls(data,dest):
    return [p for p in range(LO,HI-4) if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==dest]
def caller(md,data,site):
    print(f"M439_CALLSITE file={site:#x} preHex={data[max(LO,site-48):site].hex()} postHex={data[site+5:site+29].hex()}")
    for delta in (40,32,24,16,8):
        start=site-delta
        if start<LO:continue
        ins=list(md.disasm(data[start:site+5],start))
        if ins and ins[-1].address==site and ins[-1].mnemonic=="call":
            print(f"M439_ALIGNMENT_CANDIDATE site={site:#x} anchor={start:#x}")
            for i in ins[-15:]:print(f"M439_PRE file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            break
    for i in md.disasm(data[site+5:site+40],site+5):
        print(f"M439_POST file={i.address:#x} asm={i.mnemonic} {i.op_str}")
        if i.address>=site+23 or i.mnemonic.startswith("ret"):break
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M439_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==SHA}")
    if sha!=SHA:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,start,end in TARGETS:show(md,data,name,start,end)
    for name,start,end in TARGETS:
        sites=raw_calls(data,start)
        print(f"M439_RAW_CALLERS function={name} target={start:#x} count={len(sites)}")
        for site in sites[:24]:caller(md,data,site)
        if len(sites)>24:print(f"M439_OMITTED function={name} count={len(sites)-24}")
    print("M439_CAUTION static disassembly and raw call matches do not establish execution, file I/O semantics, or SPR usage.")
    print("M439_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
