#!/usr/bin/env python3
"""STAFU M446: inspect buffered-transfer caller control flow (read-only).
Usage: python scripts/stafu-m446-buffered-transfer-caller-semantics.py games/STTNG/STTNG.OVL > m446-buffered-transfer-caller-semantics.txt
Requires capstone. File offsets are physical offsets; static candidates do not establish execution.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
SITES=(0x7BF23,0x7BFF5,0x7C016)
TARGET=0x89924
def direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None
def decode(md,data,pc):
    return next(iter(md.disasm(data[pc:pc+15],pc,count=1)),None)
def walk(md,data,start,lower,upper,limit=700):
    todo=deque([start]);seen=set();out={};edges=[];calls=[]
    while todo and len(out)<limit:
        pc=todo.popleft()
        while lower<=pc<upper and pc not in seen and len(out)<limit:
            seen.add(pc);i=decode(md,data,pc)
            if i is None:break
            out[pc]=i;nxt=pc+i.size
            if i.mnemonic=="call":
                dest=direct(i);calls.append((pc,dest,i.op_str))
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                dest=direct(i)
                if dest is not None and lower<=dest<upper:todo.append(dest);edges.append((pc,dest))
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                dest=direct(i)
                if dest is not None and lower<=dest<upper:todo.append(dest);edges.append((pc,dest))
            pc=nxt
    return out,edges,calls
def raw_calls(data,target):
    return [p for p in range(LO,HI-4) if data[p]==0xE8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]
def print_ins(prefix,items):
    for addr,i in sorted(items.items()):
        print(f"{prefix} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M446_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    print(f"M446_TARGET target={TARGET:#x} rawCallers={','.join(hex(x) for x in raw_calls(data,TARGET))}")
    for site in SITES:
        i=decode(md,data,site);valid=bool(i and i.mnemonic=="call" and direct(i)==TARGET)
        print(f"M446_SITE file={site:#x} validDirectCall={valid} instruction={i.mnemonic+' '+i.op_str if i else 'none'}")
        if not valid:continue
        # Locate plausible entries using prologue candidates and inbound E8 targets.
        lower=max(LO,site-0x700);upper=min(HI,site+0x900)
        incoming={}
        for p in range(LO,HI-4):
            if data[p]!=0xE8:continue
            dst=p+5+struct.unpack_from("<i",data,p+1)[0]
            if lower<=dst<=site:incoming.setdefault(dst,[]).append(p)
        starts=set(incoming)
        for p in range(lower,site+1):
            if data[p:p+3] in (b"\x55\x8b\xec",b"\x55\x89\xe5"):starts.add(p)
        # Prefer nearest entries and inbound targets; report evidence, do not assume boundaries.
        starts=sorted(starts,key=lambda p:(site-p))[:32]
        print(f"M446_ENTRY_CANDIDATES site={site:#x} count={len(starts)}")
        for start in starts:
            out,edges,calls=walk(md,data,start,lower,upper)
            if site not in out:continue
            print(f"M446_REACHABLE_ENTRY site={site:#x} entry={start:#x} inbound={len(incoming.get(start,[]))} decoded={len(out)} edges={len(edges)} calls={len(calls)}")
            # Limit full-function dumps to the nearest reachable candidate, but include all relevant callsites.
        reachable=[]
        for start in starts:
            out,edges,calls=walk(md,data,start,lower,upper)
            if site in out:reachable.append((start,out,edges,calls))
        if reachable:
            start,out,edges,calls=min(reachable,key=lambda x:site-x[0])
            print(f"M446_SELECTED site={site:#x} entry={start:#x} heuristicOnly=1")
            print_ins("M446_FUNCTION",out)
            for p,d,operand in calls:print(f"M446_FUNCTION_CALL site={site:#x} file={p:#x} target={hex(d) if d is not None else 'indirect'} operand={operand}")
            for p,d in edges:print(f"M446_EDGE site={site:#x} from={p:#x} to={d:#x}")
            for p in incoming.get(start,[])[:30]:print(f"M446_INBOUND site={site:#x} entry={start:#x} caller={p:#x}")
        else:print(f"M446_NO_REACHABLE_ENTRY site={site:#x}")
        # Independent, exact post-call linear window; catches consumers even when entry heuristic fails.
        p=site+5;post={}
        while p<min(HI,site+0x145) and len(post)<90:
            ins=decode(md,data,p)
            if ins is None:break
            post[p]=ins;p+=ins.size
            if ins.mnemonic.startswith("ret"):break
        print_ins("M446_POST_LINEAR",post)
        # Pre-call candidate alignments, preserving only those ending exactly at site.
        for back in (96,80,64,48,40,32,24,16):
            p=site-back
            if p<LO:continue
            seq=list(md.disasm(data[p:site+5],p))
            if seq and seq[-1].address==site and seq[-1].mnemonic=="call":
                print(f"M446_PRE_ALIGNMENT site={site:#x} start={p:#x}")
                for ins in seq[-22:]:print(f"M446_PRE file={ins.address:#x} asm={ins.mnemonic} {ins.op_str}")
                break
    print("M446_CAUTION static control flow and raw call matches are not runtime evidence; no SPR decoder established.")
    print("M446_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
