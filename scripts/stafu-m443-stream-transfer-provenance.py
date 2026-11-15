#!/usr/bin/env python3
"""STAFU M443: shared stream transfer and buffered-reader caller provenance (read-only).
Usage: python scripts/stafu-m443-stream-transfer-provenance.py games/STTNG/STTNG.OVL > m443-stream-transfer-provenance.txt
Requires capstone. All addresses are physical STTNG.OVL file offsets.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
TARGETS={"transfer":0x7E684,"buffered_reader":0x89924,"dos_read":0x11ACD0,"dos_seek":0x11AE03,"position":0x7E638}
def direct(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None
def walk(md,data,start,end,limit=700):
    todo=deque([start]);seen=set();out={}
    while todo and len(out)<limit:
        pc=todo.popleft()
        while start<=pc<end and pc not in seen and len(out)<limit:
            seen.add(pc)
            ins=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
            if not ins:break
            i=ins[0];out[pc]=i
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            nxt=pc+i.size
            if i.mnemonic=="jmp":
                d=direct(i)
                if d is not None and start<=d<end:todo.append(d)
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                d=direct(i)
                if d is not None and start<=d<end:todo.append(d)
            pc=nxt
    return out
def raw_calls(data,target):
    return [p for p in range(LO,HI-4) if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]
def context(md,data,site,label):
    print(f"M443_CALLER target={label} site={site:#x} beforeHex={data[max(LO,site-32):site].hex()}")
    best=None
    for back in range(8,100):
        start=site-back
        if start<LO:break
        ins=list(md.disasm(data[start:site+5],start))
        if ins and ins[-1].address==site and ins[-1].mnemonic=="call" and direct(ins[-1])==TARGETS[label]:
            score=sum(1 for i in ins if i.mnemonic in ("push","mov","lea","add","sub","cmp","test"))
            if best is None or score>best[0]:best=(score,ins)
    if best:
        for i in best[1][-20:]:print(f"M443_PRE target={label} site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
    else:print(f"M443_ALIGNMENT_UNCERTAIN target={label} site={site:#x}")
    for i in md.disasm(data[site+5:site+100],site+5):
        print(f"M443_POST target={label} site={site:#x} file={i.address:#x} asm={i.mnemonic} {i.op_str}")
        if i.address>=site+52 or i.mnemonic.startswith("ret") or i.mnemonic=="jmp":break
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M443_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,start,end in [("transfer",0x7E684,0x7E800),("buffered_reader",0x89924,0x89B1E)]:
        ins=walk(md,data,start,end)
        print(f"M443_FUNCTION name={name} entry={start:#x} decoded={len(ins)} rangeEnd={end:#x}")
        for addr,i in sorted(ins.items()):
            print(f"M443_INSN name={name} file={addr:#x} bytes={i.bytes.hex()} asm={i.mnemonic} {i.op_str}")
        print(f"M443_DIRECT_CALLS name={name} calls="+",".join(f"{i.address:#x}->{direct(i):#x}" for i in ins.values() if i.mnemonic=="call" and direct(i) is not None))
    for name in ("transfer","buffered_reader"):
        sites=raw_calls(data,TARGETS[name])
        print(f"M443_RAW_CALLERS target={name} count={len(sites)} sites="+",".join(hex(s) for s in sites))
        for site in sites[:60]:context(md,data,site,name)
        if len(sites)>60:print(f"M443_OMITTED target={name} count={len(sites)-60}")
    # Examine the two transfer call sites with a shared entry and print nearby I/O calls.
    for center in (0x7E684,0x89924):
        low=max(LO,center-0x280);high=min(HI,center+0x480)
        hits=[]
        for p in range(low,high-4):
            if data[p]!=0xe8:continue
            d=p+5+struct.unpack_from("<i",data,p+1)[0]
            if d in TARGETS.values():hits.append((p,d))
        print(f"M443_NEARBY center={center:#x} matches="+",".join(f"{p:#x}->{d:#x}" for p,d in hits))
    print("M443_CAUTION raw E8 matches and linear pre-call alignments are hypotheses, not runtime execution proof. Function boundaries are provisional. No SPR decoding is asserted.")
    print("M443_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
