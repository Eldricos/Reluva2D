#!/usr/bin/env python3
"""M351: decode LE 32-bit internal fixups for two resource-state jump tables.

Read-only, original STTNG.OVL only. Decode 07 10 records using their fixed
nine-byte form (source type 07, target flags 10, source offset u16,
target object u8, target offset u32). Validate record boundaries by
consecutive entries and inspect code destinations; do not modify resources.
"""
import argparse
import hashlib
import struct
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

SIZE = 2008455
SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LE = 229780
TABLES = ((0x12334, "object_state_dispatch"), (0x13488, "global_resource_dispatch"))

def u32(data, offset):
    if not 0 <= offset <= len(data)-4:
        return None
    return struct.unpack_from("<I", data, offset)[0]

def u16(data, offset):
    if not 0 <= offset <= len(data)-2:
        return None
    return struct.unpack_from("<H", data, offset)[0]

def hx(value):
    return "none" if value is None else f"0x{value:x}"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl", type=Path)
    args = parser.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    print(f"M351_INPUT bytes={len(data)} sha256={digest}")
    if len(data) != SIZE or digest != SHA or data[LE:LE+2] != b"LE":
        raise SystemExit("M351_ERROR unexpected STTNG.OVL")

    object_count = u32(data, LE+0x44)
    object_table = LE+u32(data, LE+0x40)
    page_table = LE+u32(data, LE+0x48)
    fixup_page = LE+u32(data, LE+0x68)
    fixup_records = LE+u32(data, LE+0x6c)
    page_size = u32(data, LE+0x28)
    total_pages = u32(data, LE+0x14)
    data_start = LE+u32(data, LE+0x80)
    print(f"M351_LAYOUT objects={object_count} pages={total_pages} pageSize={page_size} dataStart={data_start} fixupPage={fixup_page} fixupRecords={fixup_records}")
    if object_count > 32 or not 0 < page_size <= 65536 or total_pages > 100000:
        raise SystemExit("M351_ERROR unsupported header bounds")

    objects = {}
    for i in range(1, object_count+1):
        p = object_table+(i-1)*24
        size,base,flags,first,count = (u32(data,p+4*j) for j in range(5))
        objects[i] = dict(size=size,base=base,flags=flags,first=first,count=count)
        print(f"M351_OBJECT index={i} base={hx(base)} size={hx(size)} firstPage={first} pages={count} flags={hx(flags)}")

    def map_offset(obj, rva):
        o = objects.get(obj)
        if o is None or not 0 <= rva < o["size"]:
            return None
        local,within = divmod(rva,page_size)
        if local >= o["count"]:
            return None
        logical = o["first"]+local
        pp = page_table+(logical-1)*4
        raw = data[pp:pp+4]
        if len(raw) != 4:
            return None
        number = int.from_bytes(raw[:3],"big")
        flags = raw[3]
        if not number or flags:
            return None
        physical = data_start+(number-1)*page_size+within
        return physical if 0 <= physical < len(data) else None

    starts = [u32(data,fixup_page+4*i) for i in range(total_pages+1)]
    if any(x is None for x in starts) or any(x>y for x,y in zip(starts,starts[1:])):
        raise SystemExit("M351_ERROR invalid fixup page index")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = False

    for table,name in TABLES:
        # M350 located each table as an object-1-relative offset.
        obj=1
        o=objects[obj]
        local,offset=divmod(table,page_size)
        logical=o["first"]+local
        table_phys=map_offset(obj,table)
        lo,hi=starts[logical-1],starts[logical]
        if hi<lo or fixup_records+hi>len(data):
            raise SystemExit("M351_ERROR fixup range outside file")
        records=data[fixup_records+lo:fixup_records+hi]
        print(f"M351_TABLE name={name} object={obj} rva={hx(table)} physical={hx(table_phys)} logicalPage={logical} offsetInPage={hx(offset)} fixupBytes={len(records)}")

        matches={}
        # A record with source type 07 and target flags 10 uses the nine-byte
        # internal 32-bit target form observed in M350. A matching source
        # offset alone does not prove a record boundary.
        for i in range(max(0,len(records)-8)):
            if records[i:i+2] != bytes((7, 16)):
                continue
            source=u16(records,i+2)
            if source is None or not offset <= source < offset+28 or (source-offset)%4:
                continue
            index=(source-offset)//4
            target_object=records[i+4]
            target_rva=u32(records,i+5)
            matches.setdefault(index,[]).append((i,target_object,target_rva))
            print(f"M351_FIXUP_CANDIDATE table={name} stateIndex={index} recordOffset={i} recordPhysical={fixup_records+lo+i} source={hx(source)} targetObject={target_object} targetRva={hx(target_rva)} bytes={records[i:i+9].hex()}")

        # A contiguous chain of seven nine-byte records is stronger evidence
        # than an isolated source-offset match. Records may occur in reverse
        # source order, as seen in M350.
        chains=[]
        for index,items in matches.items():
            for pos,_,_ in items:
                chain=[]
                for j in range(7):
                    at=pos+j*9
                    found=[(k,target_obj,target_rva) for k,target_obj,target_rva in matches.get((index-j)%7,[]) if k==at]
                    if not found:
                        break
                    chain.append(((index-j)%7,at,found[0][1]))
                if len(chain)==7 and sorted(x[0] for x in chain)==list(range(7)):
                    chains.append(chain)
        unique_chains={tuple((idx,pos) for idx,pos,_ in chain):chain for chain in chains}
        print(f"M351_CHAIN_SUMMARY table={name} completeChains={len(unique_chains)} candidateIndices={sorted(matches)}")
        confirmed_positions={pos for chain in unique_chains.values() for _,pos,_ in chain}
        for index in range(7):
            items=matches.get(index,[])
            print(f"M351_STATE_BEGIN table={name} index={index} candidates={len(items)}")
            for pos,target_object,target_rva in items:
                aligned=pos in confirmed_positions
                target_phys=map_offset(target_object,target_rva)
                raw_phys=map_offset(obj,table+4*index)
                raw=u32(data,raw_phys) if raw_phys is not None else None
                print(f"M351_STATE_CANDIDATE table={name} index={index} recordOffset={pos} inCompleteChain={int(aligned)} targetObject={target_object} targetRva={hx(target_rva)} targetPhysical={hx(target_phys)} rawTableWord={hx(raw)}")
                if target_phys is None:
                    continue
                code=data[target_phys:min(len(data),target_phys+96)]
                print(f"M351_TARGET_BYTES table={name} index={index} physical={target_phys} hex={code[:32].hex()}")
                for ins in md.disasm(code,target_phys):
                    print(f"M351_TARGET_INSN table={name} index={index} physical={ins.address} mnemonic={ins.mnemonic} operands={ins.op_str}")
                    if ins.address>=target_phys+48 or ins.mnemonic in ("ret","retf","jmp"):
                        break
            print(f"M351_STATE_END table={name} index={index}")
        print(f"M351_TABLE_END name={name}")

    print("M351_LIMITATIONS fixedNineByteRecordSubsetOnly=1 sourceRecordBoundaryNotGloballyParsed=1 relocationNotApplied=1 candidateCodeDisassemblyNotSemanticProof=1 physicalOffsetsNotRuntimeAddresses=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
