#!/usr/bin/env python3
"""M414: DOS4GW helper frame/return and ALLOC_B branch verification.

Read-only, bounded static diagnostic. No claim of dynamic reachability.
"""
import argparse, hashlib, struct
from pathlib import Path

HELPER=0x2D05E
ALLOC=0x2CBC6
LE=0x38194

def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def signed8(x):return x-256 if x&128 else x
def signed16(x):return x-65536 if x&32768 else x

def print_ins(tag,b,p,n,text,kind="other",target=None,stack=None):
    print("M414_INSN tag=%s file=0x%x raw=%s length=%d kind=%s text=%s target=%s stackEffect=%s"%(
        tag,p,b[p:p+n].hex(),n,kind,text,hex(target) if target is not None else "none",
        str(stack) if stack is not None else "unknown"))

def helper(b):
    print("M414_HELPER_RAW start=0x%x bytes=%s"%(HELPER,b[HELPER:HELPER+112].hex()))
    p=HELPER
    if b[p]==0xc8:
        local=u16(b,p+1);nest=b[p+3]
        print_ins("helper",b,p,4,"enter %d,%d"%(local,nest),"frame",stack=-(local+4))
        print("M414_FRAME localBytes=%d nesting=%d"% (local,nest))
        p+=4
    else:print("M414_FRAME missingEnter=1")
    # Fixed landmarks must be validated from the actual bytes, not assumed.
    patterns=[
        ("push_ds",bytes.fromhex("1e")),
        ("mov_ax_00a8",bytes.fromhex("b8a800")),
        ("mov_ds_ax",bytes.fromhex("8ed8")),
        ("call_slot_1a0c",bytes.fromhex("ff1e0c1a")),
        ("cleanup_12",bytes.fromhex("83c40c")),
        ("far_call_0080_3efa",bytes.fromhex("9afa3e8000")),
        ("cleanup_10",bytes.fromhex("83c40a")),
        ("pop_ds_leave_ret4",bytes.fromhex("1fc9c20400")),
    ]
    for name,needle in patterns:
        offsets=[];start=HELPER
        while True:
            idx=b.find(needle,start,HELPER+120)
            if idx<0:break
            offsets.append(idx);start=idx+1
        print("M414_LANDMARK name=%s bytes=%s occurrences=%d offsets=%s"%(
            name,needle.hex(),len(offsets),",".join(hex(x) for x in offsets) or "none"))
        for idx in offsets:
            if name=="call_slot_1a0c":print_ins("helper",b,idx,4,"call near [1a0ch]","call")
            elif name=="cleanup_12":print_ins("helper",b,idx,3,"add sp,12","cleanup",stack=12)
            elif name=="far_call_0080_3efa":print_ins("helper",b,idx,5,"call far 0080:3efa","call")
            elif name=="cleanup_10":print_ins("helper",b,idx,3,"add sp,10","cleanup",stack=10)
            elif name=="pop_ds_leave_ret4":
                print_ins("helper",b,idx,1,"pop ds","restore",stack=2)
                print_ins("helper",b,idx+1,1,"leave","frame")
                print_ins("helper",b,idx+2,3,"ret 4","return",stack=4)
    # Consecutive argument sequences before each helper internal call:
    for name,needle in (("slot",bytes.fromhex("ff1e0c1a")),("far",bytes.fromhex("9afa3e8000"))):
        at=b.find(needle,HELPER,HELPER+100)
        if at>=0:
            print("M414_CALL_CONTEXT name=%s file=0x%x preceding=%s following=%s"%(
                name,at,b[max(HELPER,at-40):at].hex(),b[at+len(needle):at+len(needle)+16].hex()))
    # Does a four-byte caller argument sequence exist at the actual callsite?
    call=0x2ce51
    if b[call]==0xe8:
        target=call+3+signed16(u16(b,call+1))
        print("M414_HELPER_CALL file=0x%x bytes=%s target=0x%x targetMatches=%d preceding=%s following=%s"%(
            call,b[call:call+3].hex(),target,int(target==HELPER),b[call-20:call].hex(),b[call+3:call+18].hex()))
        if b[call-2:call]==bytes.fromhex("5250"):
            print("M414_CALLER_ARGUMENTS file=0x%x sequence=push_dx_push_ax bytes=4 status=exactAdjacentBytes"% (call-2))
    print("M414_HELPER_LIMITATION landmarkSearchNotFullDisassembly=1 controlFlowUnverified=1 calleeStackEffectFromRetOnly=1")

def alloc(b):
    print("M414_ALLOC_RAW start=0x2cb96 bytes=%s"%b[0x2cb96:0x2cbcf].hex())
    # Decode short conditional branches at actual opcode starts based on
    # local alignment. Print every raw candidate but distinguish candidates.
    for at in (0x2cb9a,0x2cba4,0x2cbaa,0x2cbba):
        raw=b[at:at+8]
        op=b[at]
        if 0x70<=op<=0x7f:
            target=at+2+signed8(b[at+1])
            print("M414_BRANCH file=0x%x raw=%s target=0x%x fallthrough=0x%x targetInWindow=%d"%(
                at,b[at:at+2].hex(),target,at+2,int(0x2cb96<=target<=0x2cc10)))
        else:print("M414_BRANCH_CANDIDATE file=0x%x raw=%s notJccStart=1"%(at,raw.hex()))
    # Explicit instruction-boundary witness: jz at 0x2cbba occupies prefix byte 0x2cbbb.
    at=0x2cbba
    print("M414_ALLOC_BOUNDARY branchFile=0x%x branchBytes=%s fallthrough=0x%x competingPrefixFile=0x2cbbb competingPrefixByte=%02x"%(
        at,b[at:at+2].hex(),at+2,b[0x2cbbb]))
    pushes=((0x2cbbc,bytes.fromhex("ff760e"),"push [bp+14]"),
            (0x2cbbf,bytes.fromhex("680801"),"push 0108h"),
            (0x2cbc2,bytes.fromhex("6a00"),"push 0"),
            (0x2cbc4,bytes.fromhex("6a0f"),"push 15"))
    valid=all(b[at:at+len(needle)]==needle for at,needle,_ in pushes)
    for at,needle,description in pushes:
        print("M414_ALLOC_PUSH file=0x%x bytes=%s expected=%s matched=%d"%(
            at,b[at:at+len(needle)].hex(),description,int(b[at:at+len(needle)]==needle)))
    call=bytes.fromhex("26ff1eac32");clean=bytes.fromhex("83c408")
    print("M414_ALLOC_CALL file=0x%x bytes=%s matches=%d cleanupBytes=%s cleanupMatches=%d fourPushBytes=%d"%(
        ALLOC,b[ALLOC:ALLOC+5].hex(),int(b[ALLOC:ALLOC+5]==call),
        b[ALLOC+5:ALLOC+8].hex(),int(b[ALLOC+5:ALLOC+8]==clean),8 if valid else 0))
    # Nearby branch targets and bytes for a later full CFG pass.
    for at in range(0x2cb96,0x2cbbe):
        if b[at] in (0x74,0x75,0x7c,0x7d,0x7e,0x7f,0xeb):
            print("M414_RAW_BRANCH_CANDIDATE file=0x%x bytes=%s target=0x%x boundaryValidated=0"%(
                at,b[at:at+2].hex(),at+2+signed8(b[at+1])))
    print("M414_ALLOC_LIMITATION fullCFGUnverified=1 alternateEntryPointsUnverified=1")

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument("ovl",type=Path)
    b=p.parse_args().ovl.read_bytes()
    print("M414_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if len(b)<LE+2 or b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":
        raise SystemExit("M414_ERROR unexpected MZ/LE layout")
    helper(b);alloc(b)
    print("M414_LIMITATIONS staticOnly=1 bytePatternsNotProofOfReachability=1 noRuntimeSegments=1")
if __name__=="__main__":main()
