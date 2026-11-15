#!/usr/bin/env python3
"""STAFU M436: trace 0x11B850 data-path and call arguments (read-only).
Usage: python scripts/stafu-m436-resource-lower-level-trace.py games/STTNG/STTNG.OVL > m436-resource-lower-level-trace.txt
Requires: python -m pip install capstone
All addresses are file offsets; static hypotheses are not runtime proof.
"""
import hashlib
import struct
import sys
from collections import deque

SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
BEGIN,END=0x7AF94,0x15382E
TARGET=0x11B850
CALLER=0x8A4CD

def cfg(md,data,entry,limit,max_instructions=500):
    queue=deque([entry]); visited=set(); edges=[]; instructions={}
    while queue and len(instructions)<max_instructions:
        addr=queue.popleft()
        while entry<=addr<limit and addr not in visited and len(instructions)<max_instructions:
            visited.add(addr)
            seq=list(md.disasm(data[addr:min(addr+16,limit)],addr,count=1))
            if not seq or seq[0].address!=addr: break
            i=seq[0]; instructions[addr]=i
            next_addr=addr+i.size
            if i.mnemonic.startswith("ret") or i.mnemonic in ("int","int3","iret","ud2","hlt"): break
            if i.mnemonic=="jmp":
                if i.op_str.startswith("0x"):
                    dest=int(i.op_str,16); edges.append((addr,dest,"jmp"))
                    if entry<=dest<limit: queue.append(dest)
                break
            if i.mnemonic.startswith("j") or i.mnemonic in ("loop","loope","loopne","jecxz"):
                if i.op_str.startswith("0x"):
                    dest=int(i.op_str,16); edges.append((addr,dest,"branch"))
                    if entry<=dest<limit: queue.append(dest)
                addr=next_addr; continue
            addr=next_addr
    return instructions,edges

def dump(md,data,name,entry,limit):
    instructions,edges=cfg(md,data,entry,limit)
    print(f"M436_CFG name={name} entry={entry:#x} limit={limit:#x} instructions={len(instructions)} edges={len(edges)}")
    for addr,i in sorted(instructions.items()):
        print(f"M436_INSN name={name} file={addr:#x} bytes={i.bytes.hex()} mnemonic={i.mnemonic} operands={i.op_str}")
        if i.mnemonic.startswith("call"):
            print(f"M436_CALL name={name} file={addr:#x} target={i.op_str}")
        if i.mnemonic.startswith("ret"):
            print(f"M436_RETURN name={name} file={addr:#x}")
        if i.mnemonic.startswith("mov") or i.mnemonic in ("lea","cmp","test","rep movsb","rep movsd"):
            if "[" in i.op_str:
                print(f"M436_MEMORY name={name} file={addr:#x} operation={i.mnemonic} operands={i.op_str}")
    for src,dest,kind in edges:
        print(f"M436_EDGE name={name} source={src:#x} destination={dest:#x} kind={kind}")
    if len(instructions)>=500: print(f"M436_WARNING name={name} instruction_cap_reached")
    return instructions

def main():
    if len(sys.argv)!=2:
        print(__doc__.strip());return 2
    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    except ImportError:
        print("M436_ERROR capstone_missing");return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M436_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==SHA}")
    if sha!=SHA:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    dump(md,data,"lower_level_11B850",TARGET,TARGET+0x700)
    dump(md,data,"resource_state_8A3F4",0x8A3F4,0x8A518)
    print(f"M436_FOCAL_CALLSITE file={CALLER:#x}")
    # Anchor at known state-function entry, not at arbitrary preceding bytes.
    for i in md.disasm(data[0x8A490:0x8A510],0x8A490):
        print(f"M436_FOCAL_CONTEXT file={i.address:#x} mnemonic={i.mnemonic} operands={i.op_str}")
    hits=[]
    for p in range(BEGIN,END-4):
        if data[p]==0xE8 and p+5+struct.unpack_from("<i",data,p+1)[0]==TARGET:
            hits.append(p)
    print(f"M436_RAW_DIRECT_CALLERS target={TARGET:#x} count={len(hits)}")
    for p in hits:
        print(f"M436_CALLER file={p:#x} precedingHex={data[max(BEGIN,p-64):p].hex()} followingHex={data[p+5:p+37].hex()}")
        # Disassembly starting at the call itself is instruction-aligned for genuine E8.
        for i in md.disasm(data[p:min(END,p+40)],p):
            print(f"M436_AFTER_CALL caller={p:#x} file={i.address:#x} mnemonic={i.mnemonic} operands={i.op_str}")
            if i.address>=p+24:break
    print("M436_CAUTION raw E8 hits may be data or mid-instruction; no runtime or interprocedural provenance proof.")
    print("M436_DONE readOnly=1")
    return 0
if __name__=="__main__":
    raise SystemExit(main())
