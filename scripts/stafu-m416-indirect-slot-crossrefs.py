#!/usr/bin/env python3
"""M416: read-only DOS4GW indirect-slot and far-target cross-reference survey."""
import argparse,hashlib,struct
from pathlib import Path

SLOT=0x1a0c
FAR=bytes.fromhex("9afa3e8000")
BASE=0x30b74
END=0x38194

def hits(b,pattern,start=0,end=END):
    pos=start
    while True:
        p=b.find(pattern,pos,end)
        if p<0:break
        yield p
        pos=p+1

def context(b,p,before=24,after=24):
    return b[max(0,p-before):min(len(b),p+after)].hex()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    b=ap.parse_args().ovl.read_bytes()
    print("M416_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if b[:2]!=b"MZ" or b[0x38194:0x38196]!=b"LE":
        raise SystemExit("M416_ERROR unexpected MZ/LE layout")
    print("M416_SCOPE start=0 end=0x%x slot=0x%x candidateFile=0x%x candidateDSBase=0x%x"%(END,SLOT,BASE+SLOT,BASE))
    patterns=[
      ("call_far_ds_slot",bytes.fromhex("ff1e0c1a")),
      ("call_far_es_slot",bytes.fromhex("26ff1e0c1a")),
      ("jmp_far_ds_slot",bytes.fromhex("ff2e0c1a")),
      ("push_word_slot",bytes.fromhex("ff360c1a")),
      ("mov_ax_slot",bytes.fromhex("a10c1a")),
      ("mov_dx_slot",bytes.fromhex("8b160c1a")),
      ("mov_slot_ax",bytes.fromhex("a30c1a")),
      ("mov_slot_dx",bytes.fromhex("89160c1a")),
      ("mov_slot_ax_modrm",bytes.fromhex("89060c1a")),
      ("mov_slot_dx_modrm",bytes.fromhex("89160c1a")),
      ("mov_slot_imm16",bytes.fromhex("c7060c1a")),
      ("les_bx_slot",bytes.fromhex("c41e0c1a")),
      ("lds_bx_slot",bytes.fromhex("c51e0c1a")),
      ("far_call_0080_3efa",FAR),
    ]
    for name,pat in patterns:
        found=list(hits(b,pat))
        print("M416_PATTERN name=%s bytes=%s count=%d"%(name,pat.hex(),len(found)))
        for p in found:
            print("M416_HIT name=%s file=0x%x context=%s"%(name,p,context(b,p)))
    # Search slot displacements regardless of opcode; distinguish accidental bytes.
    disp=struct.pack("<H",SLOT)
    occurrences=list(hits(b,disp))
    print("M416_RAW_DISP count=%d"%len(occurrences))
    for p in occurrences:
        print("M416_DISP file=0x%x before=%s after=%s classification=unverified"%(p,b[max(0,p-5):p].hex(),b[p+2:p+8].hex()))
    # Candidate DS offset storage: M406 showed resolver return AX/DX
    # stored in adjacent 16-bit slots. Inspect +/- 32 bytes.
    fp=BASE+SLOT
    print("M416_SLOT_BYTES file=0x%x bytes=%s"%(fp,b[fp-32:fp+48].hex()))
    print("M416_SLOT_WORDS file=0x%x low=0x%04x high=0x%04x"%(fp,struct.unpack_from("<H",b,fp)[0],struct.unpack_from("<H",b,fp+2)[0]))
    # Look for stores to either half of far pointer (including DX high word).
    for offset in (SLOT,SLOT+2):
        x=struct.pack("<H",offset)
        for opcode,name in ((bytes.fromhex("a3"),"mov_mem_ax"),(bytes.fromhex("8906"),"mov_mem_ax_modrm"),
                            (bytes.fromhex("8916"),"mov_mem_dx_modrm"),(bytes.fromhex("c706"),"mov_mem_imm16")):
            pat=opcode+x
            found=list(hits(b,pat))
            print("M416_STORE slot=0x%x pattern=%s count=%d"%(offset,name,len(found)))
            for p in found:print("M416_STORE_HIT slot=0x%x file=0x%x bytes=%s context=%s"%(offset,p,pat.hex(),context(b,p)))
    # Nearby far calls to selector 0x80 and offset 0x3efa, both direct and raw.
    farptr=bytes.fromhex("fa3e8000")
    print("M416_FAR_POINTER_RAW count=%d"%len(list(hits(b,farptr))))
    for p in hits(b,farptr):
        print("M416_FAR_POINTER_HIT file=0x%x preceding=%s context=%s"%(p,b[max(0,p-3):p].hex(),context(b,p)))
    print("M416_LIMITATIONS rawPatternScan=1 instructionBoundaryUnverified=1 runtimeDSUnverified=1 crossRefsNotCallGraph=1")
if __name__=="__main__":main()
