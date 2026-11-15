#!/usr/bin/env python3
"""STAFU M445: inspect buffered-stream byte consumers and refill boundaries.
Read-only. Usage: python scripts/stafu-m445-buffered-byte-consumers.py games/STTNG/STTNG.OVL > m445-buffered-byte-consumers.txt
Requires capstone. Static x86 analysis only; no runtime behavior or SPR decoding asserted.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
A,B=0x89800,0x89C80
GLOBALS={0x9EE68:"buffer",0x9EE6C:"remaining",0x9EE70:"buffer_state",0x12F78:"stream_index",0x12F7C:"stream_state"}
def target(i):
    try:return int(i.op_str,16) if i.op_str.startswith("0x") else None
    except ValueError:return None
def walk(md,data,start,low,high,limit=650):
    q=deque([start]);seen=set();out={}
    while q and len(out)<limit:
        pc=q.popleft()
        while low<=pc<high and pc not in seen and len(out)<limit:
            seen.add(pc)
            xs=list(md.disasm(data[pc:min(pc+15,high)],pc,count=1))
            if not xs:break
            i=xs[0];out[pc]=i;nxt=pc+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                t=target(i)
                if t is not None and low<=t<high:q.append(t)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                t=target(i)
                if t is not None and low<=t<high:q.append(t)
            pc=nxt
    return out
def raw_calls(data,t):
    return [p for p in range(LO,HI-4) if data[p]==0xE8 and p+5+struct.unpack_from("<i",data,p+1)[0]==t]
def emit(md,data,start,end,label):
    print(f"M445_WINDOW name={label} start={start:#x} end={end:#x}")
    for i in md.disasm(data[start:end],start):
        if i.address>=end:break
        print(f"M445_INSN group={label} file={i.address:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M445_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==SHA}")
    if sha!=SHA:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    inbound={}
    for p in range(LO,HI-4):
        if data[p]!=0xE8:continue
        t=p+5+struct.unpack_from("<i",data,p+1)[0]
        if A<=t<B:inbound.setdefault(t,[]).append(p)
    entries=set(inbound)
    for p in range(A,B-2):
        if data[p:p+3] in (b"\x55\x8b\xec",b"\x55\x89\xe5"):entries.add(p)
    entries.update((0x89801,0x89924,0x899A9,0x89A7E,0x89B00))
    print(f"M445_ENTRIES count={len(entries)}")
    for entry in sorted(entries):
        ins=walk(md,data,entry,A,B)
        if not ins:continue
        state=[i for i in ins.values() if any(f"0x{a:x}" in i.op_str for a in GLOBALS)]
        byte=[i for i in ins.values() if i.mnemonic in ("movzx","movsx","lodsb","stosb","shl","shr","sar","rol","ror","rcl","rcr","bt","bts","btr","btc","movsb")]
        refill=[i for i in ins.values() if i.mnemonic=="call" and target(i) in (0x7E684,0x89777,0x89794,0x8998C,0x899A9,0x89A7E)]
        print(f"M445_ENTRY file={entry:#x} inbound={len(inbound.get(entry,[]))} decoded={len(ins)} globalOps={len(state)} byteOrBitOps={len(byte)} refillCalls={len(refill)}")
        if entry in (0x89801,0x89924,0x89B00) or inbound.get(entry) or state or refill:
            for i in sorted(ins.values(),key=lambda i:i.address):
                print(f"M445_CFG entry={entry:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
            for p in inbound.get(entry,[])[:40]:
                print(f"M445_INBOUND entry={entry:#x} call={p:#x} before={data[max(LO,p-20):p].hex()} after={data[p+5:p+19].hex()}")
    for addr,name in GLOBALS.items():
        needle=struct.pack("<I",addr);hits=[];pos=LO
        while True:
            p=data.find(needle,pos,HI)
            if p<0:break
            hits.append(p);pos=p+1
        print(f"M445_GLOBAL name={name} addr={addr:#x} rawCount={len(hits)}")
        for p in hits:
            print(f"M445_GLOBAL_CONTEXT name={name} operand={p:#x} bytes={data[max(LO,p-12):min(HI,p+16)].hex()}")
    for label,s,e in (("consumer_start",0x89924,0x899A0),("refill_a",0x89990,0x89A15),("refill_b",0x89A40,0x89AD0),("consumer_tail",0x89AD0,0x89B50)):
        emit(md,data,s,e,label)
    for t in (0x89777,0x8998C,0x89A40,0x89A7E,0x89B00):
        calls=raw_calls(data,t)
        print(f"M445_CALL_TARGET target={t:#x} rawCount={len(calls)} sites={','.join(hex(p) for p in calls[:80])}")
    print("M445_CAUTION Raw matches and heuristic CFG do not establish execution, byte semantics, or SPR decoder identity.")
    print("M445_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
