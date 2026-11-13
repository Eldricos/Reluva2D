#!/usr/bin/env python3
"""M408: conservative boundary-aware analysis of 17 DOS4GW indirect export calls.

Read-only; Python standard library. Only a limited 16-bit x86 instruction subset
is decoded. Unknown instructions terminate a candidate path rather than being
guessed. Results do not establish reachability or runtime segment values.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LIMIT = 0x38194
EXPORTS = {
    "VMFABIND": 0x1A04, "VMFMA": 0x1A10, "VMFILEIO": 0x1A08,
    "DVX_Global_Seg_Associate": 0x3290, "DVX_Global_Seg_Alloc": 0x32AC,
    "DVX_Global_Seg_Broadcast": 0x32B0,
    "DVX_Dos4gx_Get_Option": 0x32A8, "VMALOCINST": 0x3294,
    "VSUPERRESIZE": 0x3298, "VSUPERFREE": 0x329C,
    "DOS4G_GETENV": 0x32A0, "DOS4G_GET_APPPATH": 0x32A4,
}
REG = ("ax", "cx", "dx", "bx", "sp", "bp", "si", "di")
SEG = ("es", "cs", "ss", "ds")
RM = ("[bx+si]", "[bx+di]", "[bp+si]", "[bp+di]", "[si]", "[di]", "[disp16]", "[bx]")

def word(b, p):
    return struct.unpack_from("<H", b, p)[0]

def signed8(v):
    return v - 256 if v >= 128 else v

def decode(b, p):
    """Return (length, text, category) or None. No speculative fallback."""
    if p >= LIMIT:
        return None
    start = p
    prefix = []
    while p < LIMIT and b[p] in (0x26, 0x2E, 0x36, 0x3E, 0xF2, 0xF3):
        prefix.append(b[p])
        p += 1
        if len(prefix) > 3:
            return None
    if p >= LIMIT:
        return None
    op = b[p]
    p += 1
    lead = (" ".join({0x26:"es:",0x2e:"cs:",0x36:"ss:",0x3e:"ds:",0xf2:"repne",0xf3:"rep"}.get(x,"?") for x in prefix) + " ") if prefix else ""
    def result(txt, cat="other"):
        return (p-start, lead+txt, cat)
    if 0x50 <= op <= 0x57:
        return result("push "+REG[op-0x50], "push")
    if 0x58 <= op <= 0x5F:
        return result("pop "+REG[op-0x58], "pop")
    if op in (0x06,0x0E,0x16,0x1E):
        return result("push "+{0x06:"es",0x0e:"cs",0x16:"ss",0x1e:"ds"}[op], "push")
    if op in (0x07,0x17,0x1F):
        return result("pop "+{0x07:"es",0x17:"ss",0x1f:"ds"}[op], "segment")
    if op in (0x68,0x6A):
        if p+(2 if op==0x68 else 1)>LIMIT:return None
        val=word(b,p) if op==0x68 else signed8(b[p])
        p+=2 if op==0x68 else 1
        return result("push "+hex(val), "push")
    if 0xB8 <= op <= 0xBF:
        if p+2>LIMIT:return None
        val=word(b,p);p+=2
        return result("mov "+REG[op-0xB8]+","+hex(val), "mov")
    if 0xB0 <= op <= 0xB7:
        if p>=LIMIT:return None
        val=b[p];p+=1
        return result("mov r8,"+hex(val))
    if op in (0xA0,0xA1,0xA2,0xA3):
        if p+2>LIMIT:return None
        disp=word(b,p);p+=2
        return result(("mov ax,[%04x]" if op==0xA1 else "mov [%04x],ax" if op==0xA3 else "mov byte_abs %04x")%disp)
    if op in (0xC3,0xCB,0xC9,0x90,0x99,0x9C,0x9D,0xC5):
        if op==0xC5: pass
        else:return result({0xc3:"ret",0xcb:"retf",0xc9:"leave",0x90:"nop",0x99:"cwd",0x9c:"pushf",0x9d:"popf"}[op], "flow" if op in (0xc3,0xcb) else "other")
    if op in (0xC2,0xCA):
        if p+2>LIMIT:return None
        n=word(b,p);p+=2
        return result(("ret " if op==0xC2 else "retf ")+hex(n),"flow")
    if op in (0xE8,0xE9):
        if p+2>LIMIT:return None
        rel=word(b,p);p+=2
        if rel>=0x8000:rel-=0x10000
        return result(("call" if op==0xe8 else "jmp")+" rel16 "+hex(rel),"flow")
    if op in (0xEB,0xE0,0xE1,0xE2,0xE3) or 0x70<=op<=0x7F:
        if p>=LIMIT:return None
        rel=signed8(b[p]);p+=1
        return result("branch rel8 "+str(rel),"flow")
    if op==0x9A:
        if p+4>LIMIT:return None
        off=word(b,p);sel=word(b,p+2);p+=4
        return result("call far %04x:%04x"%(sel,off),"flow")
    if op in (0x04,0x0C,0x24,0x2C,0x34,0x3C,0xA8):
        if p>=LIMIT:return None
        n=b[p];p+=1
        return result("alu al,"+hex(n))
    if op in (0x05,0x0D,0x25,0x2D,0x35,0x3D,0xA9):
        if p+2>LIMIT:return None
        n=word(b,p);p+=2
        return result("alu ax,"+hex(n))
    if op in (0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F):
        return result(("inc " if op<0x48 else "dec ")+REG[op%8])
    if op in (0x89,0x8B,0x88,0x8A,0x8C,0x8E,0x8D,0x8F,0xFF,0xFE,0x80,0x81,0x83,0xC6,0xC7,0x00,0x01,0x02,0x03,0x08,0x09,0x0A,0x0B,0x10,0x11,0x12,0x13,0x18,0x19,0x1A,0x1B,0x20,0x21,0x22,0x23,0x28,0x29,0x2A,0x2B,0x30,0x31,0x32,0x33,0x38,0x39,0x3A,0x3B,0x69,0x6B,0xC4,0xC5,0xD0,0xD1,0xD2,0xD3,0xF6,0xF7):
        if p>=LIMIT:return None
        modrm=b[p];p+=1
        mod=modrm>>6; reg=(modrm>>3)&7; rm=modrm&7
        disp=0
        if mod==0 and rm==6 or mod==2:
            if p+2>LIMIT:return None
            disp=word(b,p);p+=2
        elif mod==1:
            if p>=LIMIT:return None
            disp=signed8(b[p]);p+=1
        mem=REG[rm] if mod==3 else RM[rm]+(("%+d"%disp) if disp and not (mod==0 and rm==6) else "")
        if mod==0 and rm==6:mem="[%04x]"%disp
        if op==0xFF:
            kind={0:"inc",1:"dec",2:"call near",3:"call far",4:"jmp near",5:"jmp far",6:"push"}.get(reg,"invalid")
            if kind=="invalid":return None
            return result(kind+" "+mem, "push" if reg==6 else "flow" if reg in (2,3,4,5) else "other")
        if op==0x8E:
            if reg>3:return None
            return result("mov "+SEG[reg]+","+mem,"segment")
        if op==0x8C:
            if reg>3:return None
            return result("mov "+mem+","+SEG[reg])
        if op in (0x80,0x81,0x83,0xC6,0xC7,0x69,0x6B):
            sz=2 if op in (0x81,0xC7,0x69) else 1
            if p+sz>LIMIT:return None
            imm=word(b,p) if sz==2 else b[p];p+=sz
            return result("op_%02x %s,%s"%(op,mem,hex(imm)),"cleanup" if op==0x83 and reg==0 and mod==3 and rm==4 else "other")
        if op in (0xF6,0xF7) and reg in (0,1):
            sz=1 if op==0xF6 else 2
            if p+sz>LIMIT:return None
            p+=sz
        return result("op_%02x %s,reg%d"%(op,mem,reg))
    return None

def candidate_paths(b,site):
    # Search starts in the preceding 64 bytes; retain only fully decoded paths
    # ending exactly at the call. This does not validate a function entry point.
    paths=[]
    for start in range(max(0,site-64),site):
        p=start;instructions=[]
        while p<site:
            d=decode(b,p)
            if not d or p+d[0]>site:break
            instructions.append((p,*d))
            p+=d[0]
        if p==site and len(instructions)>=2:
            paths.append((start,instructions))
    return paths

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    b=ap.parse_args().ovl.read_bytes()
    print("M408_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if b[:2]!=b"MZ" or b[LIMIT:LIMIT+2]!=b"LE":
        raise SystemExit("M408_ERROR unexpected MZ/LE layout")
    total=0
    for name,disp in EXPORTS.items():
        pattern=b"\xff\x1e"+struct.pack("<H",disp)
        sites=[];at=0
        while True:
            at=b.find(pattern,at,LIMIT)
            if at<0:break
            sites.append(at);at+=1
        print("M408_EXPORT name=%s dsOffset=0x%04x sites=%d"%(name,disp,len(sites)))
        for site in sites:
            total+=1
            paths=candidate_paths(b,site)
            # Longest candidate path gives most context, not proof of correctness.
            chosen=max(paths,key=lambda x:len(x[1]),default=None)
            after=decode(b,site+4)
            print("M408_SITE name=%s file=0x%x raw=%s candidatePaths=%d longestStart=%s nextInstruction=%s"%(name,site,b[site:site+4].hex(),len(paths),hex(chosen[0]) if chosen else "none",after[1] if after else "unknown"))
            if chosen:
                tail=chosen[1][-12:]
                for p,n,txt,cat in tail:
                    print("M408_DECODE call=0x%x file=0x%x len=%d category=%s text=%s"%(site,p,n,cat,txt))
                contiguous=[]
                for p,n,txt,cat in reversed(chosen[1]):
                    if p+n==site or (contiguous and p+n==contiguous[-1][0]):
                        if cat=="push":contiguous.append((p,txt))
                        else:break
                contiguous.reverse()
                print("M408_PUSH_RUN call=0x%x count=%d bytes=%d instructions=%s"%(site,len(contiguous),2*len(contiguous),";".join("%x:%s"%x for x in contiguous) or "none"))
                seg=[(p,txt) for p,n,txt,cat in chosen[1] if cat=="segment"]
                print("M408_SEGMENT_OBS call=0x%x observations=%s"%(site,";".join("%x:%s"%(p,t) for p,t in seg) or "none"))
            q=site+4
            if b[q:q+3]==b"\x83\xc4\x08":cleanup="add_sp_8"
            elif b[q:q+3]==b"\x83\xc4\x0c":cleanup="add_sp_12"
            elif b[q:q+3]==b"\x83\xc4\x0a":cleanup="add_sp_10"
            elif b[q:q+2]==b"\x5b\x5b":cleanup="pop_bx_twice"
            else:cleanup="other_or_callee"
            print("M408_CLEANUP call=0x%x immediate=%s"%(site,cleanup))
    print("M408_TOTAL exports=%d callSites=%d"%(len(EXPORTS),total))
    print("M408_LIMITATIONS partialDecoder=1 candidateStartsNotValidated=1 branchReachabilityUnknown=1 runtimeSegmentStateUnknown=1 pushRunNotArgumentCount=1")
if __name__=="__main__":
    main()
