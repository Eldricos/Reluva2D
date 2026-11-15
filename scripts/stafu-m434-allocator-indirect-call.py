#!/usr/bin/env python3
"""STAFU M434: inspect allocator 0x930A4 and indirect-call site 0x936D7.
Read-only static diagnostics. Requires capstone.
Usage: python scripts/stafu-m434-allocator-indirect-call.py games/STTNG/STTNG.OVL > m434-allocator-indirect-call.txt
All addresses printed are OVL file offsets, NOT runtime linear addresses.
"""
import hashlib
import struct
import sys

EXPECTED = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
START, END = 0x7AF94, 0x15382E
TARGETS = [(0x930A4, "pointer-provider"), (0x93569, "structure-initializer"), (0x936D7, "indirect-call-site")]
WINDOWS = [
    (0x92F80, 0x93220, "pointer-provider-neighborhood"),
    (0x934D0, 0x937A0, "initializer-and-indirect-neighborhood"),
]
def show(md, data, a, b, label):
    print(f"M434_WINDOW label={label} start={a:#x} end={b:#x}")
    ins = list(md.disasm(data[a:b], a))
    for i in ins:
        print(f"M434_INSN label={label} file={i.address:#x} bytes={i.bytes.hex()} mnemonic={i.mnemonic} operands={i.op_str}")
    stop = ins[-1].address + ins[-1].size if ins else a
    print(f"M434_WINDOW_END label={label} stop={stop:#x} requested={b:#x}")
    return ins

def main():
    if len(sys.argv) != 2:
        print(__doc__.strip())
        return 2
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        print("M434_ERROR missing_capstone install='python -m pip install capstone'")
        return 2
    with open(sys.argv[1], "rb") as f:
        data = f.read()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M434_INPUT bytes={len(data)} sha256={digest} hashMatch={digest == EXPECTED}")
    if digest != EXPECTED:
        print("M434_ERROR unexpected_input_hash")
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    print(f"M434_OBJECT1 fileStart={START:#x} fileEnd={END:#x} mode=file-offset")
    for a, b, label in WINDOWS:
        show(md, data, a, b, label)
    for target, name in TARGETS[:2]:
        hits = []
        for pos in range(START, END - 4):
            if data[pos] != 0xE8:
                continue
            displacement = struct.unpack_from("<i", data, pos + 1)[0]
            if pos + 5 + displacement == target:
                hits.append(pos)
        print(f"M434_DIRECT_CALLS target={name} file={target:#x} count={len(hits)}")
        for pos in hits[:60]:
            print(f"M434_CALLSITE target={name} file={pos:#x}")
            a = max(START, pos - 28)
            b = min(END, pos + 45)
            print(f"M434_CALLSITE_BYTES file={pos:#x} start={a:#x} hex={data[a:b].hex()}")
            # Decode from multiple nearby possible instruction boundaries; not CFG proof.
            for back in (16, 8, 5):
                anchor = max(START, pos - back)
                ins = list(md.disasm(data[anchor:min(END, pos + 24)], anchor))
                match = next((i for i in ins if i.address == pos and i.mnemonic == "call"), None)
                if match:
                    print(f"M434_CALL_ALIGNMENT file={pos:#x} anchor={anchor:#x} valid=True")
                    for i in ins:
                        if pos - 16 <= i.address <= pos + 16:
                            print(f"M434_CALL_CONTEXT file={i.address:#x} mnemonic={i.mnemonic} operands={i.op_str}")
                    break
            else:
                print(f"M434_CALL_ALIGNMENT file={pos:#x} valid=unverified")
        if len(hits) > 60:
            print(f"M434_CALLSITE_TRUNCATED target={name} omitted={len(hits)-60}")
    site = 0x936D7
    print(f"M434_INDIRECT_SITE file={site:#x} bytes={data[site:site+16].hex()}")
    for anchor in (site, site-8, site-16, site-32):
        print(f"M434_INDIRECT_DECODE anchor={anchor:#x}")
        for i in md.disasm(data[anchor:site+24], anchor):
            print(f"M434_INDIRECT_INSN file={i.address:#x} mnemonic={i.mnemonic} operands={i.op_str}")
    print("M434_CAUTION raw E8 matches may be data or mid-instruction; linear disassembly may cross functions.")
    print("M434_CAUTION pointer-provider, initializer, and callback roles are hypotheses pending dataflow proof.")
    print("M434_DONE readOnly=1")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
