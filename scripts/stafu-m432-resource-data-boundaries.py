#!/usr/bin/env python3
"""STAFU M432: inspect resource data-access boundary candidates (read-only).

Requires capstone: python -m pip install capstone
Run: python scripts/stafu-m432-resource-data-boundaries.py games/STTNG/STTNG.OVL > m432-resource-data-boundaries.txt
All reported addresses are original OVL file offsets; static semantics are hypotheses.
"""
import argparse
import hashlib
import struct
from collections import Counter

EXPECTED="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
LO,HI=0x7AF94,0x15382E
TARGETS={
 "stream_position_wrapper":0x897B3,
 "resource_lifecycle":0x8A518,
 "state_transition_helper":0x93569,
 "lower_level_a":0x11B850,
 "lower_level_b":0x11BE64,
}
def disassemble(md,blob,start,end):
    return list(md.disasm(blob[start:end],start))

def inspect(md,blob,name,start,window):
    end=min(HI,start+window)
    print(f"M432_FUNCTION name={name} file={start:#x} windowEnd={end:#x}")
    instructions=disassemble(md,blob,start,end)
    for i in instructions:
        print(f"M432_INSN function={name} file={i.address:#x} bytes={i.bytes.hex()} mnemonic={i.mnemonic} operands={i.op_str}")
    calls=[i for i in instructions if i.mnemonic=="call"]
    returns=[i for i in instructions if i.mnemonic.startswith("ret")]
    print(f"M432_FUNCTION_SUMMARY name={name} decoded={len(instructions)} calls={len(calls)} returns={len(returns)}")
    for i in calls:
        print(f"M432_CALLEE function={name} site={i.address:#x} target={i.op_str}")
    print(f"M432_LIMIT name={name} linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1")

def callers(md,blob,name,target,context,max_hits):
    hits=[]
    for p in range(LO,HI-4):
        if blob[p]!=0xE8: continue
        rel=struct.unpack_from("<i",blob,p+1)[0]
        if p+5+rel==target: hits.append(p)
    print(f"M432_DIRECT_CALLS name={name} target={target:#x} rawCount={len(hits)}")
    for p in hits[:max_hits]:
        # Align locally from a plausible function prologue if possible; never
        # claim this proves a valid code path.
        anchors=[]
        for a in range(max(LO,p-64),p+1):
            seq=disassemble(md,blob,a,p+5)
            if seq and seq[-1].address==p and seq[-1].mnemonic=="call" and seq[-1].size==5:
                anchors.append((a,seq))
        if anchors:
            anchor,seq=max(anchors,key=lambda item:len(item[1]))
            prior=seq[-min(len(seq),10):]
            before="; ".join(f"{i.address:#x}:{i.mnemonic} {i.op_str}" for i in prior)
        else:
            anchor=None
            before=""
        post=disassemble(md,blob,p+5,min(HI,p+5+context))
        after="; ".join(f"{i.address:#x}:{i.mnemonic} {i.op_str}" for i in post[:8])
        print(f"M432_CALLSITE name={name} file={p:#x} localAnchor={hex(anchor) if anchor is not None else 'none'}")
        print(f"M432_BEFORE name={name} file={p:#x} instructions={before}")
        print(f"M432_AFTER name={name} file={p:#x} instructions={after}")
    if len(hits)>max_hits:
        print(f"M432_TRUNCATED name={name} omitted={len(hits)-max_hits} allSites={','.join(hex(x) for x in hits)}")
    print(f"M432_CALL_LIMIT name={name} rawOpcodeCandidates=1 localAlignmentNotCFG=1 indirectCallsNotScanned=1")

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl")
    ap.add_argument("--window",type=lambda s:int(s,0),default=0x180)
    ap.add_argument("--max-callers",type=int,default=20)
    args=ap.parse_args()
    with open(args.ovl,"rb") as f: blob=f.read()
    sha=hashlib.sha256(blob).hexdigest()
    print(f"M432_INPUT bytes={len(blob)} sha256={sha} hashMatch={sha==EXPECTED}")
    if sha!=EXPECTED or len(blob)<HI or blob[:2]!=b"MZ":
        raise SystemExit("M432_ERROR unexpected original OVL; offsets not applicable")
    try:
        from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    except ImportError:
        raise SystemExit("M432_ERROR install capstone: python -m pip install capstone")
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    print(f"M432_OBJECT1 start={LO:#x} end={HI:#x} mappingAssumedContiguous=1")
    for name,addr in TARGETS.items():
        inspect(md,blob,name,addr,args.window)
    for name,addr in TARGETS.items():
        callers(md,blob,name,addr,48,args.max_callers)
    print("M432_DONE readOnly=1 originalAssetsNotExtracted=1 dataMovementNotYetProven=1")

if __name__=="__main__":
    main()
