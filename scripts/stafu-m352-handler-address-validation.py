#!/usr/bin/env python3
"""M352: validate LE jump-table targets against known physical-code anchors.

Read-only diagnostic for the known STTNG.OVL. Uses Capstone. Reports
alternative file origins, instruction boundaries and relocation-target
interpretations without claiming a unique mapping.
"""
import argparse
import hashlib
import struct
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

LE = 229780
EXPECTED_SIZE = 2008455
EXPECTED_SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES = [
    ("object_state", 0x12334, [0x1239b,0x1238e,0x123ca,0x123f8,0x123eb,0x123ca,0x12429],1),
    ("global_state", 0x13488, [0x13598,0x13598,0x1356e,0x134ed,0x13598,0x1356e,0x13561],0),
]
ANCHORS = [
    ("resource_dispatch",566260),("resource_dispatch_jump",566541),
    ("resource_lifecycle",566552),("resource_check",566876),
    ("constructor",567212),("stream_position_wrapper",563123),
    ("global_state",570696),("global_state_jump",570960),
    ("spr_constructor_call",580767),("spr_state_call",580820),
]

def u32(b,p):
    return struct.unpack_from("<I",b,p)[0] if 0<=p<=len(b)-4 else None

def h(n):
    return "none" if n is None else f"0x{n:x}"

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    b=args.ovl.read_bytes()
    digest=hashlib.sha256(b).hexdigest()
    print(f"M352_INPUT bytes={len(b)} sha256={digest}")
    if len(b)!=EXPECTED_SIZE or digest!=EXPECTED_SHA or b[LE:LE+2]!=b"LE":
        raise SystemExit("M352_ERROR unexpected STTNG.OVL")
    size=u32(b,LE+0x28)
    datafield=u32(b,LE+0x80)
    objtab=LE+u32(b,LE+0x40)
    pagetab=LE+u32(b,LE+0x48)
    objects=[]
    for i in range(u32(b,LE+0x44)):
        p=objtab+24*i
        objects.append(dict(index=i+1,size=u32(b,p),base=u32(b,p+4),
                            first=u32(b,p+12),count=u32(b,p+16)))
    print(f"M352_HEADER pageSize={size} dataField={h(datafield)} leHeader={LE} origins=headerPlusField,absoluteField")
    for o in objects:
        print(f"M352_OBJECT index={o['index']} base={h(o['base'])} size={h(o['size'])} first={o['first']} pages={o['count']}")

    def physical(obj,rva,origin):
        o=objects[obj-1]
        if rva<0 or rva>=o["size"]:return None
        local,inside=divmod(rva,size)
        if local>=o["count"]:return None
        logical=o["first"]+local
        raw=b[pagetab+(logical-1)*4:pagetab+logical*4]
        if len(raw)!=4:return None
        page=int.from_bytes(raw[:3],"big")
        if page==0 or raw[3]!=0:return None
        p=origin+(page-1)*size+inside
        return p if 0<=p<len(b) else None

    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=False
    def disassemble_at(physical_offset,limit=12):
        if physical_offset is None:return []
        result=[]
        for ins in md.disasm(b[physical_offset:min(len(b),physical_offset+72)],physical_offset):
            result.append(f"{ins.address}:{ins.mnemonic} {ins.op_str}")
            if len(result)>=limit or ins.mnemonic.startswith("ret"):break
        return result

    def boundary_score(p):
        if p is None:return (0,0)
        # Examine forward decode coverage from the exact candidate address.
        # This is a heuristic only: x86 decoding can start mid-instruction.
        ins=list(md.disasm(b[p:min(len(b),p+48)],p))
        count=len(ins)
        covered=sum(x.size for x in ins)
        return count,covered

    for name,p in ANCHORS:
        print(f"M352_ANCHOR name={name} physical={p} hex={b[p:p+20].hex()}")
        for line in disassemble_at(p,5):
            print(f"M352_ANCHOR_INSN name={name} {line}")

    origins=(("headerPlusField",LE+datafield),("absoluteField",datafield))
    for origin_name,origin in origins:
        print(f"M352_ORIGIN_BEGIN name={origin_name} physical={origin}")
        for name,table,targets,first_state in TABLES:
            print(f"M352_TABLE_BEGIN origin={origin_name} name={name} tableRva={h(table)}")
            for interpretation in ("objectRelative","absoluteMinusBase"):
                rva=table if interpretation=="objectRelative" else table-objects[0]["base"]
                p=physical(1,rva,origin)
                print(f"M352_TABLE_LOCATION interpretation={interpretation} rva={h(rva)} physical={h(p)} rawHex={b[p:p+28].hex() if p is not None else 'none'}")
            for index,target in enumerate(targets):
                state=index+first_state
                for interpretation in ("objectRelative","absoluteMinusBase"):
                    rva=target if interpretation=="objectRelative" else target-objects[0]["base"]
                    p=physical(1,rva,origin)
                    count,covered=boundary_score(p)
                    print(f"M352_TARGET origin={origin_name} table={name} state={state} relocationValue={h(target)} interpretation={interpretation} rva={h(rva)} physical={h(p)} insnCount={count} coveredBytes={covered} firstBytes={b[p:p+16].hex() if p is not None else 'none'}")
                    if p is None:continue
                    for line in disassemble_at(p,7):
                        print(f"M352_TARGET_INSN origin={origin_name} table={name} state={state} interpretation={interpretation} {line}")
                    # Local scan helps locate the start of a plausible handler
                    # when a candidate lands inside a previous instruction.
                    for delta in (-8,-4,-2,-1,0,1,2,4,8):
                        q=p+delta
                        if not 0<=q<len(b):continue
                        cnt,coverage=boundary_score(q)
                        first=next(md.disasm(b[q:min(len(b),q+16)],q),None)
                        print(f"M352_NEIGHBOR origin={origin_name} table={name} state={state} interpretation={interpretation} delta={delta} physical={q} instructions={cnt} coveredBytes={coverage} first={(first.mnemonic+' '+first.op_str) if first else 'none'}")
            print(f"M352_TABLE_END origin={origin_name} name={name}")
        print(f"M352_ORIGIN_END name={origin_name}")

    # Compare mapped RVAs for previously established physical anchors.
    for origin_name,origin in origins:
        for name,p in ANCHORS:
            for o in objects:
                page_index=(p-origin)//size
                if page_index<0:continue
                local=page_index-(o["first"]-1)
                if not 0<=local<o["count"]:continue
                rva=local*size+(p-origin)%size
                mapped=physical(o["index"],rva,origin)
                if mapped==p:
                    print(f"M352_ANCHOR_INVERSE origin={origin_name} name={name} object={o['index']} rva={h(rva)} runtimeBasePlusRva={h(o['base']+rva)}")
    print("M352_LIMITATIONS x86BoundaryHeuristicOnly=1 noFixupsApplied=1 anchorsArePhysicalOffsetsFromEarlierDiagnostics=1 targetSemanticsNotProven=1 SPRDecoderUnverified=1")

if __name__=="__main__":
    main()
