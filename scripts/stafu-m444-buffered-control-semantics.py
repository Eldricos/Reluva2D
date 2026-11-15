#!/usr/bin/env python3
"""STAFU M444: inspect higher-level buffered-transfer control and helper semantics.
Usage: python scripts/stafu-m444-buffered-control-semantics.py games/STTNG/STTNG.OVL > m444-buffered-control-semantics.txt
Requires capstone. Read-only; physical file offsets, static inference only.
"""
import hashlib,struct,sys
from collections import deque
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
REGIONS=[("control",0x7BDB0,0x7C130),("buffer_setup",0x89801,0x89924),("control_helper",0x89D94,0x89F10),("boundary_helper",0x7C34C,0x7C470)]
TARGETS=[0x89801,0x89D94,0x7C34C,0x89924]
def dest(i):
    if i.op_str.startswith("0x"):
        try:return int(i.op_str,16)
        except ValueError:pass
    return None
def walk(md,data,start,end,limit=900):
    q=deque([start]);seen=set();out={};edges=[]
    while q and len(out)<limit:
        pc=q.popleft()
        while start<=pc<end and pc not in seen and len(out)<limit:
            seen.add(pc);arr=list(md.disasm(data[pc:min(pc+15,end)],pc,count=1))
            if not arr:break
            i=arr[0];out[pc]=i;nxt=pc+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","hlt","ud2"):break
            if i.mnemonic=="jmp":
                d=dest(i)
                if d is not None and start<=d<end:q.append(d);edges.append((pc,d))
                break
            if i.mnemonic.startswith("j") or i.mnemonic.startswith("loop"):
                d=dest(i)
                if d is not None and start<=d<end:q.append(d);edges.append((pc,d))
            pc=nxt
    return out,edges
def raw_calls(data,target):
    return [p for p in range(LO,HI-4) if data[p]==0xe8 and p+5+struct.unpack_from("<i",data,p+1)[0]==target]
def main():
    if len(sys.argv)!=2:print(__doc__);return 2
    data=open(sys.argv[1],"rb").read();sha=hashlib.sha256(data).hexdigest()
    print(f"M444_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==SHA}")
    if sha!=SHA:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for name,start,end in REGIONS:
        print(f"M444_REGION name={name} start={start:#x} end={end:#x}")
        # Find entry candidates from direct incoming calls, prologues, and specified region start.
        incoming={}
        for p in range(LO,HI-4):
            if data[p]!=0xe8:continue
            d=p+5+struct.unpack_from("<i",data,p+1)[0]
            if start<=d<end:incoming.setdefault(d,[]).append(p)
        entries=set(incoming)|{start}
        for p in range(start,end-3):
            if data[p:p+3] in (b"\x55\x8b\xec",b"\x55\x89\xe5"):entries.add(p)
        if name=="control":entries.update([0x7BE80,0x7BF00])
        print(f"M444_ENTRIES name={name} count={len(entries)}")
        for entry in sorted(entries):
            ins,edges=walk(md,data,entry,end)
            calls=[(i.address,dest(i)) for i in ins.values() if i.mnemonic=="call" and dest(i) is not None]
            print(f"M444_ENTRY name={name} entry={entry:#x} decoded={len(ins)} incoming={len(incoming.get(entry,[]))} edges={len(edges)} calls="+",".join(f"{a:#x}->{b:#x}" for a,b in calls))
            # Avoid duplicated disassembly for spurious interior prologues.
            if entry!=start and entry not in incoming and entry not in (0x7BF00,0x7BE80):continue
            for a,i in sorted(ins.items()):
                print(f"M444_INSN name={name} entry={entry:#x} file={a:#x} asm={i.mnemonic} {i.op_str}")
            for caller in incoming.get(entry,[])[:35]:print(f"M444_INBOUND name={name} entry={entry:#x} caller={caller:#x}")
    for target in TARGETS:
        sites=raw_calls(data,target)
        print(f"M444_CALLERS target={target:#x} count={len(sites)} sites="+",".join(hex(p) for p in sites))
        for site in sites[:40]:
            print(f"M444_CALLSITE target={target:#x} site={site:#x} before={data[max(LO,site-40):site].hex()} after={data[site+5:site+25].hex()}")
    for addr in (0x9EE68,0x9EE6C,0x9EE70,0x12F78,0x12F7C,0x69554):
        needle=struct.pack("<I",addr);sites=[];p=LO
        while True:
            p=data.find(needle,p,HI)
            if p<0:break
            sites.append(p);p+=1
        print(f"M444_GLOBAL address={addr:#x} rawCount={len(sites)} sites="+",".join(hex(p) for p in sites[:100]))
    print("M444_CAUTION bounded CFG can cross function boundaries; raw callsites and operand references need instruction-boundary verification. No SPR decoding conclusion.")
    print("M444_DONE readOnly=1")
    return 0
if __name__=="__main__":raise SystemExit(main())
