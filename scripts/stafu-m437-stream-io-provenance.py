#!/usr/bin/env python3
"""M437: inspect stream-position and underlying I/O helper call semantics, read-only.
Usage: python scripts/stafu-m437-stream-io-provenance.py games/STTNG/STTNG.OVL > m437-stream-io-provenance.txt
Requires capstone. All addresses below are file offsets, not runtime linear addresses.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32

EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
TARGETS=[("stream_wrapper",0x897B3,0x89A50),("position_manager",0x7E638,0x7E950),
         ("stream_helper_7E5A4",0x7E5A4,0x7E638),("stream_helper_7E410",0x7E410,0x7E5A4)]
def parse_direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None
def walk(md,data,start,end,cap=350):
    todo=deque([start]); seen=set(); ins={}
    while todo and len(ins)<cap:
        pc=todo.popleft()
        while start<=pc<end and pc not in seen and len(ins)<cap:
            seen.add(pc)
            item=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
            if not item:break
            i=item[0];ins[pc]=i;nextpc=pc+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                dst=parse_direct(i)
                if dst is not None and start<=dst<end:todo.append(dst)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                dst=parse_direct(i)
                if dst is not None and start<=dst<end:todo.append(dst)
            pc=nextpc
    return ins
def render(md,data,name,start,end):
    ins=walk(md,data,start,end)
    print(f"M437_FUNCTION name={name} entry={start:#x} bound={end:#x} decoded={len(ins)} last={max(ins) if ins else 0:#x}")
    for addr,i in sorted(ins.items()):
        print(f"M437_INSN function={name} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
        if i.mnemonic.startswith("call"):
            print(f"M437_CALL function={name} file={addr:#x} target={i.op_str}")
    if len(ins)>=350:print(f"M437_WARNING function={name} instruction_cap")
def callers(data,target):
    out=[]
    for p in range(LO,HI-4):
        if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target:out.append(p)
    return out
def call_context(md,data,site):
    # Inspect candidate starts; do not assume a linear decode is instruction aligned.
    print(f"M437_CALLSITE file={site:#x} before={data[site-40:site].hex()} after={data[site+5:site+21].hex()}")
    for delta in (32,24,16,8):
        start=max(LO,site-delta)
        sequence=list(md.disasm(data[start:site+5],start))
        if sequence and sequence[-1].address==site and sequence[-1].mnemonic=="call":
            print(f"M437_ALIGNED_CONTEXT candidateStart={start:#x} site={site:#x}")
            for i in sequence[-12:]:
                print(f"M437_CONTEXT file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            break
    else:print(f"M437_CONTEXT_UNCERTAIN site={site:#x}")
def main():
    if len(sys.argv)!=2:
        print(__doc__);return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M437_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,start,end in TARGETS:render(md,data,name,start,end)
    for name,start,end in TARGETS[:2]:
        sites=callers(data,start)
        print(f"M437_RAW_CALLERS function={name} target={start:#x} count={len(sites)}")
        for site in sites[:30]:call_context(md,data,site)
        if len(sites)>30:print(f"M437_OMITTED function={name} count={len(sites)-30}")
    print("M437_CAUTION raw E8 matches and disassembly windows do not establish runtime execution or semantics.")
    print("M437_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
