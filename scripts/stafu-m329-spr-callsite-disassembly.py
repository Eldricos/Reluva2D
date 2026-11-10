#!/usr/bin/env python3
"""M329: instruction-level verification of selected SPR-region E8 call candidates.

Requires Capstone 5 (python -m pip install capstone). Read-only; reports
instruction-boundary evidence and immediate preceding instructions. Physical
offsets are NOT automatically runtime linear addresses; LE mapping is printed.
"""
import argparse
import hashlib
import pathlib
import struct
import sys

SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
SITES = [
    (579718, 577491), (919677, 589648), (919699, 589648),
    (919817, 589648), (920033, 589648), (920058, 589648),
    (920193, 589648), (920218, 589648), (920464, 589648),
    (920873, 589648), (920921, 589648), (921263, 589648),
    (921308, 589648), (987538, 589648),
    (573150, 503556), (573164, 508624),
    (919707, 503556), (919721, 508624),
    (920066, 503556), (920080, 508624),
]
TAGS = [(579269, "PMOC"), (579280, "MOCS"),
        (579731, "PMOC"), (579742, "MOCS"),
        (919982, "PMOC"), (919991, "PMOC"),
        (920000, "TSIL")]

def u32(data, at):
    return struct.unpack_from("<I", data, at)[0]

def le_mapper(data):
    h = 229780
    if data[h:h+2] != b"LE" or u32(data,h+0x28) != 4096:
        raise ValueError("unsupported LE layout")
    data_start = u32(data,h+0x80)
    obj_rel, count, map_rel = u32(data,h+0x40),u32(data,h+0x44),u32(data,h+0x48)
    pages = {}
    for i in range(count):
        size,base,flags,first,npage = struct.unpack_from("<IIIII",data,h+obj_rel+i*24)
        for j in range(npage):
            p = h+map_rel+(first-1+j)*4
            number = int.from_bytes(data[p:p+3],"big")
            if number and data[p+3] == 0:
                pages[number] = (i+1,base+j*4096,size-j*4096)
    def mapping(physical):
        if physical < data_start:
            return None
        number,offset = divmod(physical-data_start,4096)
        entry = pages.get(number+1)
        return (entry[0],entry[1]+offset) if entry and offset < entry[2] else None
    return mapping

def disassemble_window(md,data,start,end,callsite):
    # Linear sweep from arbitrary starting positions is NOT function-entry proof.
    # Multiple anchors show how sensitive boundaries are to the starting point.
    anchors = [max(0,callsite-48),max(0,callsite-32),max(0,callsite-16)]
    for anchor in anchors:
        instructions = list(md.disasm(data[anchor:end],anchor))
        at_site = next((ins for ins in instructions if ins.address == callsite),None)
        covering = next((ins for ins in instructions if ins.address < callsite < ins.address+ins.size),None)
        status = "aligned" if at_site else ("inside_instruction" if covering else "not_reached")
        print(f"M329_SWEEP anchor={anchor} site={callsite} status={status}")
        if at_site:
            before = [i for i in instructions if i.address+i.size <= callsite][-8:]
            after = [i for i in instructions if i.address >= callsite][:5]
            for ins in before+after:
                print(f"M329_INSN anchor={anchor} physical={ins.address} "
                      f"bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")
    return

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=pathlib.Path)
    args = ap.parse_args()
    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    except ImportError:
        print("M329_ERROR Capstone missing: python -m pip install capstone",file=sys.stderr)
        return 2
    try:
        data = args.ovl.read_bytes()
    except OSError as exc:
        print(f"M329_ERROR {exc}",file=sys.stderr)
        return 2
    digest = hashlib.sha256(data).hexdigest()
    valid = len(data)==2008455 and digest==SHA
    print(f"M329_INPUT bytes={len(data)} sha256={digest} knownMatch={int(valid)}")
    if not valid:
        return 2
    try:
        mapper = le_mapper(data)
    except (ValueError,struct.error) as exc:
        print(f"M329_ERROR {exc}",file=sys.stderr)
        return 2
    md = Cs(CS_ARCH_X86,CS_MODE_32)
    for site,target in SITES:
        raw = data[site:site+5]
        actual = site+5+struct.unpack_from("<i",data,site+1)[0] if raw[0]==0xE8 else None
        source_map,target_map = mapper(site),mapper(target)
        print(f"M329_CANDIDATE site={site} target={target} raw={raw.hex()} "
              f"rel32Matches={int(actual==target)} sourceMap={source_map} targetMap={target_map}")
        disassemble_window(md,data,max(0,site-48),min(len(data),site+48),site)
    for physical,tag in TAGS:
        lo=max(0,physical-32)
        hi=min(len(data),physical+32)
        print(f"M329_TAG_CONTEXT tag={tag} physical={physical} mapped={mapper(physical)}")
        for ins in md.disasm(data[lo:hi],lo):
            if physical-12 <= ins.address <= physical+12:
                print(f"M329_TAG_INSN anchor={lo} physical={ins.address} "
                      f"bytes={ins.bytes.hex()} mnemonic={ins.mnemonic} operands={ins.op_str}")
    print("M329_LIMITATION linearSweepsNotFunctionEntryProven=1 "
          "argumentSemanticsUnverified=1 indirectCallsUntraced=1 "
          "LEFixupsUnapplied=1 runtimeSPRConsumerUnverified=1")
    return 0

if __name__ == "__main__":
    sys.exit(main())
