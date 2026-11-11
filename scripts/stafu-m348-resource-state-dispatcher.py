#!/usr/bin/env python3
"""M348: resource-state jump-table and dispatcher analysis (read-only).

Physical offsets into the known STTNG.OVL. The LE relocation fixups are NOT
applied; candidate jump targets are hypotheses, never asserted as resolved.
Requires capstone.
"""
import argparse, hashlib, struct
from collections import deque
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM

EXPECTED_SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
EXPECTED_SIZE=2008455
TABLE=0x12334
STATE_ENTRY=566260
DISPATCH_ENTRY=570696

def u32(b,p):
    return struct.unpack_from("<I",b,p)[0] if 0<=p<=len(b)-4 else None

def run_cfg(md,b,entry,low,high,limit=1800):
    q=deque([entry]); seen={}; edges=[]; failures=[]
    while q and len(seen)<limit:
        pc=q.popleft()
        while low<=pc<high and pc not in seen and len(seen)<limit:
            ins=next(md.disasm(b[pc:min(pc+15,high)],pc,count=1),None)
            if not ins:
                failures.append(pc);break
            seen[pc]=ins
            m=ins.mnemonic
            if m.startswith("j") or m.startswith("loop"):
                dest=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
                edges.append((pc,dest,m))
                if dest is not None and low<=dest<high:q.append(dest)
                if m=="jmp":break
            if m.startswith("ret") or m in ("hlt","ud2","int3"):break
            pc+=ins.size
    return seen,edges,failures

def show(md,b,label,entry,lo,hi,limit=1800):
    insns,edges,failures=run_cfg(md,b,entry,lo,hi,limit)
    print(f"M348_CFG name={label} entry={entry} lower={lo} upper={hi} decoded={len(insns)} edges={len(edges)} failures={len(failures)} bounded=1")
    for pc,ins in sorted(insns.items()):
        print(f"M348_INSN name={label} {pc} {ins.mnemonic} {ins.op_str}")
        if ins.mnemonic.startswith("call"):
            dest=ins.operands[0].imm if ins.operands and ins.operands[0].type==X86_OP_IMM else None
            print(f"M348_CALL name={label} site={pc} target={dest if dest is not None else 'indirect'}")
        for idx,op in enumerate(ins.operands):
            if op.type==X86_OP_MEM and (op.mem.disp in (0x44,0x48,0x50,0x54,0x58,0x5c,0x60,0x64,0x70,0x74,0x9ee74,0x9ee78,0x12334,0x13488)):
                base=ins.reg_name(op.mem.base) if op.mem.base else "-"
                index=ins.reg_name(op.mem.index) if op.mem.index else "-"
                print(f"M348_FIELD name={label} site={pc} operand={idx} base={base} index={index} disp={op.mem.disp:+#x} size={op.size}")
    for site,target,kind in edges:
        print(f"M348_EDGE name={label} site={site} target={target if target is not None else 'indirect'} kind={kind} outside={int(target is not None and not lo<=target<hi)}")
    for pc in failures:print(f"M348_FAILURE name={label} offset={pc}")
    print(f"M348_CFG_END name={label}")
    return insns

def main():
    p=argparse.ArgumentParser()
    p.add_argument("ovl",type=Path)
    a=p.parse_args()
    b=a.ovl.read_bytes()
    sha=hashlib.sha256(b).hexdigest()
    print(f"M348_INPUT bytes={len(b)} sha256={sha}")
    if len(b)!=EXPECTED_SIZE or sha!=EXPECTED_SHA:
        raise SystemExit("M348_ERROR unexpected STTNG.OVL hash/size")
    md=Cs(CS_ARCH_X86,CS_MODE_32);md.detail=True
    # Print LE header metadata from known physical header, without asserting mappings.
    le=229780
    print(f"M348_LE_HEADER offset={le} signature={b[le:le+2].hex()} objectTableOffset={u32(b,le+0x40)} objectCount={u32(b,le+0x44)} fixupPageTableOffset={u32(b,le+0x68)} fixupRecordTableOffset={u32(b,le+0x6c)}")
    # Jump table operand is a runtime linear address, not necessarily a file offset.
    # Explore literal file offset and a small set of plausible image-relative mappings.
    candidates=[("literal_physical",TABLE),("minus_image_base_10000",TABLE-0x10000),("le_header_plus_rva",le+TABLE),("le_header_plus_rva_minus_base",le+TABLE-0x10000)]
    for label,offset in candidates:
        if not 0<=offset<=len(b)-28:continue
        print(f"M348_TABLE_CANDIDATE mapping={label} offset={offset} unverified=1")
        for i in range(7):
            raw=u32(b,offset+i*4)
            print(f"M348_TABLE_ENTRY mapping={label} state={i+1} fileOffset={offset+i*4} raw={raw:#010x} rawPhysicalInFile={int(raw is not None and raw<len(b))}")
    # Stable, known physical call target entries and the extended dispatcher.
    show(md,b,"object_state_dispatch",STATE_ENTRY,566260,566552)
    show(md,b,"global_resource_dispatch",DISPATCH_ENTRY,570696,571450)
    # Separate anchored target blocks from M347, not guessed table resolutions.
    for name,start,end in (
        ("object_position_block",566382,566415),
        ("object_reset_block",566415,566470),
        ("object_mode_dispatch",566514,566552),
        ("global_state_loop",570927,571110),
    ):
        print(f"M348_ANCHOR name={name} start={start} end={end} entryAlignmentFromPriorTrace={int(start in (566382,566415,566514,570927))}")
        for ins in md.disasm(b[start:end],start):
            print(f"M348_ANCHOR_INSN name={name} {ins.address} {ins.mnemonic} {ins.op_str}")
        print(f"M348_ANCHOR_END name={name}")
    print("M348_LIMITATIONS static=1 physicalOffsets=1 LEFixupsUnapplied=1 jumpTableTargetsUnverified=1 candidateMappingsHypothetical=1 indirectCFGUnresolved=1 boundsHeuristic=1 SPRDecoderUnverified=1")

if __name__=="__main__":main()
