#!/usr/bin/env python3
"""STAFU M435: instruction-aligned node allocation/release analysis.
Read-only; Capstone required. Offsets are OVL file offsets.
Usage: python scripts/stafu-m435-list-node-lifecycle.py games/STTNG/STTNG.OVL > m435-list-node-lifecycle.txt
"""
import hashlib
import struct
import sys

EXPECTED = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
START, END = 0x7AF94, 0x15382E
FUNCTIONS = [
    ("node_allocator_candidate", 0x930A4, 0x93200),
    ("node_release_candidate", 0x93287, 0x93400),
    ("node_constructor", 0x93569, 0x9359E),
    ("list_destructor", 0x936A1, 0x93732),
]
def disassemble(md, data, name, start, end):
    print(f"M435_FUNCTION name={name} entry={start:#x} limit={end:#x}")
    instructions = list(md.disasm(data[start:end], start))
    for ins in instructions:
        print(f"M435_INSN function={name} file={ins.address:#x} bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")
        if ins.mnemonic.startswith("call") and ins.op_str.startswith("0x"):
            print(f"M435_CALLEE function={name} callsite={ins.address:#x} target={ins.op_str}")
        if ins.mnemonic.startswith("ret") and ins.address >= start:
            print(f"M435_FIRST_RET function={name} file={ins.address:#x}")
            break
    print(f"M435_FUNCTION_END name={name} note=first-ret-not-CFG-proof")

def calls(md, data, name, target):
    sites=[]
    for p in range(START, END-4):
        if data[p] == 0xE8 and p+5+struct.unpack_from("<i", data, p+1)[0] == target:
            sites.append(p)
    print(f"M435_CALLS target={name} entry={target:#x} rawCount={len(sites)}")
    for p in sites[:40]:
        print(f"M435_CALL file={p:#x} before={data[max(START,p-20):p].hex()} after={data[p+5:p+29].hex()}")
        for anchor in (p-12,p-8,p-5,p):
            if anchor < START: continue
            ins=list(md.disasm(data[anchor:p+16],anchor))
            if any(x.address==p and x.mnemonic=="call" for x in ins):
                print(f"M435_CALL_ALIGNED file={p:#x} anchor={anchor:#x}")
                for x in ins:
                    if p-12 <= x.address <= p+12:
                        print(f"M435_CONTEXT file={x.address:#x} mnemonic={x.mnemonic} operands={x.op_str}")
                break
    if len(sites)>40:
        print(f"M435_TRUNCATED target={name} omitted={len(sites)-40}")

def main():
    if len(sys.argv)!=2:
        print(__doc__.strip()); return 2
    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    except ImportError:
        print("M435_ERROR capstone_missing"); return 2
    data=open(sys.argv[1],"rb").read()
    sha=hashlib.sha256(data).hexdigest()
    print(f"M435_INPUT bytes={len(data)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED: return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    for args in FUNCTIONS: disassemble(md,data,*args)
    for name,target in (("node_allocator_candidate",0x930A4),("node_release_candidate",0x93287)):
        calls(md,data,name,target)
    print("M435_CAUTION first-ret traversal omits alternate branches; raw call sites can include data/mid-instruction false positives.")
    print("M435_DONE readOnly=1")
    return 0
if __name__=="__main__":
    raise SystemExit(main())
