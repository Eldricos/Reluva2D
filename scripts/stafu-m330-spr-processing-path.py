#!/usr/bin/env python3
"""M330: focused SPR control-flow evidence around confirmed tag comparisons.

Read-only disassembly. Reports branch destinations, direct calls, and bounded
memory operations. Physical-offset disassembly is a static approximation; this
does not claim runtime behavior, resolved fixups, or codec semantics.
Requires: python -m pip install capstone
"""
import argparse
import hashlib
import pathlib
import struct
import sys

SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = [
    ("callee_577491",577491,578100),
    ("pmoc_mocs_first",579170,579370),
    ("caller_579718",579610,579860),
    ("pmoc_list_dispatch",919890,920135),
]
ROOTS = [577491,579267,579718,579729,919979,919988,919997]
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("ovl",type=pathlib.Path)
    args=p.parse_args()
    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_GRP_CALL,CS_GRP_JUMP
        from capstone.x86_const import X86_OP_IMM
    except ImportError:
        print("M330_ERROR install capstone: python -m pip install capstone",file=sys.stderr)
        return 2
    try: data=args.ovl.read_bytes()
    except OSError as e:
        print(f"M330_ERROR input={e}",file=sys.stderr)
        return 2
    sha=hashlib.sha256(data).hexdigest()
    valid=len(data)==2008455 and sha==SHA
    print(f"M330_INPUT bytes={len(data)} sha256={sha} knownMatch={int(valid)}")
    if not valid:return 2
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    md.detail=True
    # These starts are inspection anchors, not asserted function boundaries.
    for name,start,end in REGIONS:
        print(f"M330_REGION name={name} physical={start}:{end} startProven=0")
        instructions=list(md.disasm(data[start:end],start))
        boundaries={i.address for i in instructions}
        for i in instructions:
            control=i.group(CS_GRP_CALL) or i.group(CS_GRP_JUMP)
            imm=None
            if control and i.operands and i.operands[0].type==X86_OP_IMM:
                imm=i.operands[0].imm
            target=f"{imm}" if imm is not None else "indirect_or_none"
            in_region=int(imm in boundaries) if imm is not None else 0
            memory=int(any(op.type==3 for op in i.operands))
            print(f"M330_INSN region={name} physical={i.address} bytes={i.bytes.hex()} "
                  f"mnemonic={i.mnemonic} operands={i.op_str} "
                  f"branchTarget={target} targetOnSweepBoundary={in_region} memoryOperand={memory}")
        for root in ROOTS:
            if start<=root<end:
                status="aligned" if root in boundaries else "not_aligned"
                print(f"M330_ANCHOR region={name} physical={root} status={status}")
    # Show direct E8 call sites targeting the focal routine, with bounded context.
    for target in (577491,):
        hits=[]
        for site in range(0,len(data)-4):
            if data[site]!=0xe8:continue
            actual=site+5+struct.unpack_from("<i",data,site+1)[0]
            if actual==target:hits.append(site)
        print(f"M330_XREF target={target} rawE8Count={len(hits)}")
        for site in hits[:80]:
            print(f"M330_XREF_SITE target={target} site={site}")
        if len(hits)>80:print(f"M330_XREF_TRUNCATED target={target} omitted={len(hits)-80}")
    print("M330_LIMITATION anchorsNotProvenFunctionEntries=1 "
          "linearSweepNotCFGProof=1 branchTargetsPhysicalOnly=1 "
          "LEFixupsUnapplied=1 indirectCallsUnresolved=1 "
          "runtimeResourcePathUnverified=1 decoderUnverified=1")
    return 0
if __name__=="__main__":
    sys.exit(main())
