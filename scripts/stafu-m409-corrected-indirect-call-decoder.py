#!/usr/bin/env python3
"""STAFU M409: corrected 16-bit x86 decoding at loader export call sites.

Read-only standard-library diagnostic. This is NOT a full disassembler:
candidate starts are untrusted, reachability and runtime DS remain unknown.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LIMIT = 0x38194
EXPORTS = {
    "VMFABIND":0x1A04, "VMFMA":0x1A10, "VMFILEIO":0x1A08,
    "DVX_Global_Seg_Associate":0x3290, "DVX_Global_Seg_Alloc":0x32AC,
    "DVX_Global_Seg_Broadcast":0x32B0, "DVX_Dos4gx_Get_Option":0x32A8,
    "VMALOCINST":0x3294, "VSUPERRESIZE":0x3298,
    "VSUPERFREE":0x329C, "DOS4G_GETENV":0x32A0,
    "DOS4G_GET_APPPATH":0x32A4,
}
R16=("ax","cx","dx","bx","sp","bp","si","di")
R8=("al","cl","dl","bl","ah","ch","dh","bh")
SREG=("es","cs","ss","ds")
BASE=("[bx+si]","[bx+di]","[bp+si]","[bp+di]","[si]","[di]",None,"[bx]")
PREFIX={0x26:"es",0x2e:"cs",0x36:"ss",0x3e:"ds"}

def u16(b,p): return struct.unpack_from("<H",b,p)[0]
def s8(v): return v-256 if v>=128 else v
def s16(v): return v-65536 if v>=32768 else v
def decode(b,at):
    p=at; segment=None
    if p>=LIMIT:return None
    if b[p] in PREFIX:
        segment=PREFIX[b[p]];p+=1
    if p>=LIMIT:return None
    op=b[p];p+=1
    def result(text,kind="other",barrier=False):
        return (p-at,text,kind,barrier)
    def operand(modrm,width=16):
        nonlocal p
        mod=modrm>>6;rm=modrm&7
        if mod==3:return (R16 if width==16 else R8)[rm]
        if mod==0 and rm==6:
            if p+2>LIMIT:raise ValueError()
            disp=u16(b,p);p+=2
            value="[%04xh]"%disp
        else:
            value=("[bp]" if rm==6 else BASE[rm])
            if mod==1:
                if p>=LIMIT:raise ValueError()
                disp=s8(b[p]);p+=1
                if disp:value=value[:-1]+("%+d"%disp)+"]"
            if mod==2:
                if p+2>LIMIT:raise ValueError()
                disp=s16(u16(b,p));p+=2
                if disp:value=value[:-1]+("%+d"%disp)+"]"
        return (segment+":" if segment else "")+value
    try:
        if 0x50<=op<=0x57:return result("push "+R16[op-0x50],"push")
        if 0x58<=op<=0x5f:return result("pop "+R16[op-0x58],"pop")
        if op in (0x06,0x0e,0x16,0x1e):
            return result("push "+{0x06:"es",0x0e:"cs",0x16:"ss",0x1e:"ds"}[op],"push")
        if op in (0x07,0x17,0x1f):
            return result("pop "+{0x07:"es",0x17:"ss",0x1f:"ds"}[op],"segment")
        if op in (0x68,0x6a):
            size=2 if op==0x68 else 1
            if p+size>LIMIT:return None
            val=u16(b,p) if size==2 else s8(b[p]);p+=size
            return result("push "+hex(val),"push")
        if 0xb8<=op<=0xbf:
            if p+2>LIMIT:return None
            val=u16(b,p);p+=2
            return result("mov %s,0x%x"%(R16[op-0xb8],val))
        if 0xb0<=op<=0xb7:
            if p>=LIMIT:return None
            val=b[p];p+=1
            return result("mov %s,0x%x"%(R8[op-0xb0],val))
        if 0x40<=op<=0x4f:
            return result(("inc " if op<0x48 else "dec ")+R16[op&7])
        if op in (0xa0,0xa1,0xa2,0xa3):
            if p+2>LIMIT:return None
            addr=u16(b,p);p+=2
            mem=(segment+":" if segment else "")+"[%04xh]"%addr
            reg="ax" if op&1 else "al"
            return result("mov "+(reg+","+mem if op in (0xa0,0xa1) else mem+","+reg))
        if op in (0xc3,0xcb,0xc9,0x90,0x99,0x9c,0x9d):
            return result({0xc3:"ret",0xcb:"retf",0xc9:"leave",0x90:"nop",0x99:"cwd",0x9c:"pushf",0x9d:"popf"}[op],"flow" if op in (0xc3,0xcb) else "other",op in (0xc3,0xcb))
        if op in (0xc2,0xca):
            if p+2>LIMIT:return None
            val=u16(b,p);p+=2
            return result(("ret " if op==0xc2 else "retf ")+hex(val),"flow",True)
        if op in (0xe8,0xe9):
            if p+2>LIMIT:return None
            rel=s16(u16(b,p));p+=2
            return result(("call" if op==0xe8 else "jmp")+" rel16 "+str(rel),"flow",op==0xe9)
        if op in (0xeb,0xe0,0xe1,0xe2,0xe3) or 0x70<=op<=0x7f:
            if p>=LIMIT:return None
            rel=s8(b[p]);p+=1
            return result(("jmp" if op==0xeb else "branch")+" rel8 "+str(rel),"flow",op==0xeb)
        if op==0x9a:
            if p+4>LIMIT:return None
            off=u16(b,p);sel=u16(b,p+2);p+=4
            return result("call far %04x:%04x"%(sel,off),"flow")
        if op in (0x04,0x0c,0x24,0x2c,0x34,0x3c,0xa8):
            if p>=LIMIT:return None
            val=b[p];p+=1
            return result("alu al,0x%x"%val)
        if op in (0x05,0x0d,0x25,0x2d,0x35,0x3d,0xa9):
            if p+2>LIMIT:return None
            val=u16(b,p);p+=2
            return result("alu ax,0x%x"%val)
        if op in (0x8b,0x89,0x8a,0x88,0x8c,0x8e,0x8d,0x8f,
                  0xff,0xfe,0x80,0x81,0x83,0xc6,0xc7,
                  0x00,0x01,0x02,0x03,0x08,0x09,0x0a,0x0b,
                  0x10,0x11,0x12,0x13,0x18,0x19,0x1a,0x1b,
                  0x20,0x21,0x22,0x23,0x28,0x29,0x2a,0x2b,
                  0x30,0x31,0x32,0x33,0x38,0x39,0x3a,0x3b,
                  0x69,0x6b,0xc4,0xc5,0xd0,0xd1,0xd2,0xd3,0xf6,0xf7):
            if p>=LIMIT:return None
            m=b[p];p+=1;reg=(m>>3)&7
            width=8 if op in (0x88,0x8a,0x80,0xc6,0xfe,0xf6,0xd0,0xd2) or (op<=0x3b and op%2==0) else 16
            dest=operand(m,width)
            src=(R8 if width==8 else R16)[reg]
            if op==0xff:
                kinds={0:"inc",1:"dec",2:"call near",3:"call far",4:"jmp near",5:"jmp far",6:"push"}
                if reg not in kinds:return None
                return result(kinds[reg]+" "+dest,"push" if reg==6 else "flow" if reg in (2,3,4,5) else "other",reg in (4,5))
            if op==0x8e:
                if reg>3 or reg==1:return None
                return result("mov "+SREG[reg]+","+dest,"segment")
            if op==0x8c:
                if reg>3:return None
                return result("mov "+dest+","+SREG[reg])
            if op==0x8f:
                if reg!=0:return None
                return result("pop "+dest,"pop")
            if op in (0x80,0x81,0x83,0xc6,0xc7,0x69,0x6b):
                if op in (0xc6,0xc7) and reg!=0:return None
                size=2 if op in (0x81,0xc7,0x69) else 1
                if p+size>LIMIT:return None
                imm=u16(b,p) if size==2 else b[p];p+=size
                if op==0x83:imm=s8(imm)
                opname=("add" if reg==0 else "sub" if reg==5 else "alu") if op in (0x80,0x81,0x83) else "mov" if op in (0xc6,0xc7) else "imul"
                return result("%s %s,%s"%(opname,dest,hex(imm)),"cleanup" if op==0x83 and reg==0 and m==0xc4 else "other")
            if op in (0xf6,0xf7) and reg in (0,1):
                size=1 if op==0xf6 else 2
                if p+size>LIMIT:return None
                p+=size
            if op in (0xc4,0xc5):return result(("les " if op==0xc4 else "lds ")+src+","+dest,"segment")
            if op==0x8d:return result("lea "+src+","+dest)
            if op in (0x8b,0x8a):return result("mov "+src+","+dest)
            if op in (0x89,0x88):return result("mov "+dest+","+src)
            return result("op_%02x %s,%s"%(op,dest,src))
        return None
    except (IndexError,ValueError,struct.error):
        return None

def paths(b,site):
    found=[]
    for start in range(max(0,site-80),site):
        p=start;seq=[]
        while p<site:
            d=decode(b,p)
            if not d or p+d[0]>site:break
            seq.append((p,*d));p+=d[0]
        if p==site and seq:found.append((start,seq))
    return found

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    b=ap.parse_args().ovl.read_bytes()
    print("M409_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if b[:2]!=b"MZ" or b[LIMIT:LIMIT+2]!=b"LE":
        raise SystemExit("M409_ERROR unexpected MZ/LE layout")
    total=0
    for name,disp in EXPORTS.items():
        pattern=b"\xff\x1e"+struct.pack("<H",disp)
        sites=[];pos=0
        while True:
            pos=b.find(pattern,pos,LIMIT)
            if pos<0:break
            sites.append(pos);pos+=1
        print("M409_EXPORT name=%s dsOffset=0x%04x sites=%d"%(name,disp,len(sites)))
        for site in sites:
            total+=1
            candidates=paths(b,site)
            clean=[(start,seq) for start,seq in candidates if not any(x[4] for x in seq)]
            # Prioritize longest barrier-free sequence, then longest raw sequence.
            chosen=max(clean or candidates,key=lambda x:(len(x[1]),-x[0]),default=None)
            print("M409_SITE name=%s file=0x%x candidatePaths=%d barrierFreePaths=%d chosenStart=%s"%(name,site,len(candidates),len(clean),hex(chosen[0]) if chosen else "none"))
            if chosen:
                seq=chosen[1]
                for addr,n,ins,kind,barrier in seq[-16:]:
                    print("M409_INSN call=0x%x file=0x%x len=%d kind=%s barrier=%d text=%s"%(site,addr,n,kind,int(barrier),ins))
                pushes=[]
                for ins in reversed(seq):
                    if ins[3]=="push":pushes.append(ins)
                    else:break
                pushes.reverse()
                cleanup=b[site+4:site+8]
                cleanup_bytes=(cleanup[2] if cleanup[:2]==b"\x83\xc4" else 4 if cleanup[:2]==b"\x5b\x5b" else None)
                print("M409_PUSH_RUN call=0x%x count=%d bytes=%d cleanupBytes=%s sizeAgreement=%s args=%s"%(site,len(pushes),len(pushes)*2,str(cleanup_bytes) if cleanup_bytes is not None else "unknown",str(cleanup_bytes==len(pushes)*2) if cleanup_bytes is not None else "unknown",";".join("%x:%s"%(x[0],x[2]) for x in pushes) or "none"))
                segment=[(x[0],x[2]) for x in seq if x[3]=="segment"]
                print("M409_SEGMENT_OBS call=0x%x operations=%s"%(site,";".join("%x:%s"%x for x in segment) or "none"))
            else:
                print("M409_UNDECODED call=0x%x preceding=%s"%(site,b[max(0,site-32):site].hex()))
            print("M409_CALL_BYTES file=0x%x bytes=%s after=%s"%(site,b[site:site+4].hex(),b[site+4:site+12].hex()))
    print("M409_TOTAL exports=%d calls=%d"%(len(EXPORTS),total))
    print("M409_LIMITATIONS partialDecoder=1 arbitraryStarts=1 linearPathsNotReachability=1 runtimeDsUnknown=1 instructionSizesNotABI=1")
if __name__=="__main__":main()
