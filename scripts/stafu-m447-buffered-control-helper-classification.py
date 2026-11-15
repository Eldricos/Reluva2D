#!/usr/bin/env python3
"""STAFU M447: classify four helpers in buffered-transfer control (read-only).
Usage: python scripts/stafu-m447-buffered-control-helper-classification.py games/STTNG/STTNG.OVL > m447-buffered-control-helper-classification.txt
Requires capstone. Physical file offsets, static evidence only.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
TARGETS=(0x89801,0x8C536,0x7C34C,0x7C36C)
CALLERS=(0x7BF08,0x7BFAC,0x7C005,0x7C028,0x7C04C)
def dest(i):
    try:return int(i.op_str,16) if i.op_str.startswith("0x") else None
    except ValueError:return None
def ins(md,data,pc):
    return next(iter(md.disasm(data[pc:pc+15],pc,count=1)),None)
def walk(md,data,start,limit=240,span=0x500):
    todo=deque([start]);seen=set();out={};edges=[];calls=[]
    while todo and len(out)<limit:
        pc=todo.popleft()
        while start<=pc<min(HI,start+span) and pc not in seen and len(out)<limit:
            seen.add(pc);i=ins(md,data,pc)
            if not i:break
            out[pc]=i;nxt=pc+i.size;d=dest(i)
            if i.mnemonic=="call":calls.append((pc,d,i.op_str))
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                if d is not None and start<=d<start+span:todo.append(d);edges.append((pc,d))
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                if d is not None and start<=d<start+span:todo.append(d);edges.append((pc,d))
            pc=nxt
    return out,edges,calls
def raw_calls(data,targets):
    result={t:[] for t in targets}
    for p in range(LO,HI-4):
        if data[p]!=0xE8:continue
        d=p+5+struct.unpack_from("<i",data,p+1)[0]
        if d in result:result[d].append(p)
    return result
def dump(prefix,seq):
    for p,i in sorted(seq.items()):
        print(f"{prefix} file={p:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M447_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    callers=raw_calls(data,TARGETS)
    for t in TARGETS:
        print(f"M447_HELPER target={t:#x} rawCallers={len(callers[t])} sites={','.join(hex(x) for x in callers[t][:80])}")
        out,edges,calls=walk(md,data,t)
        print(f"M447_CFG target={t:#x} decoded={len(out)} branches={len(edges)} calls={len(calls)} capReached={len(out)>=240}")
        dump("M447_INSN",out)
        for p,d,operand in calls:print(f"M447_CALLEE helper={t:#x} site={p:#x} target={hex(d) if d is not None else 'indirect'} operand={operand}")
        for a,b in edges:print(f"M447_EDGE helper={t:#x} from={a:#x} to={b:#x}")
        for p in callers[t][:20]:
            if p in CALLERS or abs(p-0x7BF00)<0x180:
                before={}
                for back in range(45,0,-1):
                    start=p-back
                    if start<LO:continue
                    seq=list(md.disasm(data[start:p+5],start))
                    if seq and seq[-1].address==p and seq[-1].mnemonic=="call" and dest(seq[-1])==t:
                        before={i.address:i for i in seq[-12:]};break
                print(f"M447_CALL_CONTEXT target={t:#x} site={p:#x} aligned={bool(before)}")
                dump("M447_BEFORE",before)
                after={};q=p+5
                for _ in range(15):
                    i=ins(md,data,q)
                    if not i:break
                    after[q]=i;q+=i.size
                    if i.mnemonic.startswith("ret"):break
                dump("M447_AFTER",after)
    print("M447_CAUTION raw E8 scans and bounded static CFG do not prove execution or helper semantics.")
    print("M447_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
