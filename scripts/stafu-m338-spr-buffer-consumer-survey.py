#!/usr/bin/env python3
"""M338: survey consumers of candidate resource-object fields +0x70/+0x74.

Read-only static Capstone analysis of STTNG.OVL. Physical file offsets are
not runtime addresses. Candidate field hits are NOT proven object accesses.
"""
import argparse
import hashlib
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET
from capstone.x86_const import X86_OP_MEM, X86_OP_IMM

SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
REGIONS = [
    ("resource_helpers", 565780, 567212),
    ("constructor_572028", 572028, 572550),
    ("constructor_567212", 567212, 567497),
    ("caller_580470", 580470, 581170),
    ("record_processing", 577741, 582250),
    ("related_resource_helpers", 563650, 565780),
]
FIELDS = {0x44, 0x48, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x70, 0x74, 0x78, 0x88}

def fmt(i):
    return f"{i.address} {i.mnemonic} {i.op_str}"

def hits(i):
    result = []
    for o in i.operands:
        if o.type == X86_OP_MEM and o.mem.disp in FIELDS and o.mem.base:
            result.append((i.reg_name(o.mem.base), o.mem.disp))
    return result

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl", type=Path)
    args = ap.parse_args()
    data = args.ovl.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    ok = len(data) == 2008455 and digest == SHA
    print(f"M338_INPUT bytes={len(data)} sha256={digest} knownMatch={int(ok)}")
    if not ok:
        return 2
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    total = 0
    for name, begin, end in REGIONS:
        ins = list(md.disasm(data[begin:end], begin))
        print(f"M338_REGION name={name} start={begin} end={end} linearInstructions={len(ins)}")
        count = 0
        for index, i in enumerate(ins):
            matches = hits(i)
            if not matches:
                continue
            count += 1
            total += 1
            print(f"M338_FIELD region={name} at={i.address} fields={','.join(hex(v) for _,v in matches)} bases={','.join(b for b,_ in matches)} instruction={i.mnemonic} {i.op_str}")
            for j in ins[max(0,index-5):min(len(ins),index+7)]:
                print(f"M338_CONTEXT region={name} focus={i.address} instruction={fmt(j)}")
        print(f"M338_REGION_END name={name} fieldCandidates={count}")
    # Exhaustive byte-aligned sweep for common base-register memory addressing
    # encodings, using decoding from each candidate offset. No claim of CFG.
    print("M338_TARGETED_GLOBAL_SCAN object1DataApprox=273920..1160858 byteAlignedCandidates=1")
    global_count = 0
    limit = 100
    start, end = 273920, min(len(data), 1160858)
    for pos in range(start, end):
        # Most common disp8 field forms: [eax/ecx/edx/ebx/esi/edi+0x70/0x74].
        # 8B=load, 89=store, 8D=lea, 83/80/81/88/8A/C7 etc.
        if data[pos] not in (0x8b, 0x89, 0x8d, 0x83, 0x80, 0x81, 0x88, 0x8a, 0xc7, 0x0f):
            continue
        i = next(md.disasm(data[pos:pos+15],pos,count=1),None)
        if not i:
            continue
        matches = [(b,v) for b,v in hits(i) if v in (0x70,0x74)]
        if not matches:
            continue
        global_count += 1
        if global_count <= limit:
            print(f"M338_GLOBAL_CANDIDATE at={pos} fields={','.join(hex(v) for _,v in matches)} bases={','.join(b for b,_ in matches)} instruction={i.mnemonic} {i.op_str}")
    print(f"M338_GLOBAL_SUMMARY totalCandidates={global_count} printed={min(global_count,limit)} truncated={int(global_count>limit)}")
    print(f"M338_SUMMARY regionalFieldCandidates={total} objectIdentityUnverified=1")
    print("M338_LIMITATION linearSweepsMayDecodeData=1 globalByteAlignedFalsePositives=1 LEFixupsUnapplied=1 noRuntimeTrace=1 decoderUnverified=1")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
