#!/usr/bin/env python3
"""STAFU M413: validate ALLOC_B branch boundaries, 8-bit AND, VMFILEIO helper.

Read-only static byte analysis; no execution or proven reachability. Standard library.
"""
import argparse
import hashlib
import struct
from pathlib import Path

LE=0x38194
ALLOC_START=0x2cb96
ALLOC_END=0x2cbcf
HELPER=0x2d05e
REG8=("al","cl","dl","bl","ah","ch","dh","bh")
REG16=("ax","cx","dx","bx","sp","bp","si","di")
BASE=("bx+si","bx+di","bp+si","bp+di","si","di","bp","bx")
SEG={0x26:"es",0x2e:"cs",0x36:"ss",0x3e:"ds"}

def s8(x):return x-256 if x&128 else x
def s16(x):return x-65536 if x&32768 else x
def word(b,p):return struct.unpack_from("<H",b,p)[0]

def modrm(b,p,seg=None,wide=True):
    if p>=LE:return None
    v=b[p];p+=1;mode=v>>6;reg=(v>>3)&7;rm=v&7
    if mode==3:return ((REG16 if wide else REG8)[rm],p,reg,mode)
    if mode==0 and rm==6:
        if p+2>LE:return None
        operand="[%04xh]"%word(b,p);p+=2
    else:
        disp=0
        if mode==1:
            if p>=LE:return None
            disp=s8(b[p]);p+=1
        elif mode==2:
            if p+2>LE:return None
            disp=s16(word(b,p));p+=2
        operand="["+BASE[rm]+("%+d"%disp if disp else "")+"]"
    return ((seg+":" if seg else "")+operand,p,reg,mode)

def decode(b,start):
    p=start;seg=None
    if p>=LE:return None
    if b[p] in SEG:p+=1;seg=SEG[b[start]]
    if p>=LE:return None
    op=b[p];p+=1
    def result(text,kind="other",target=None,stack=0):
        return (p-start,text,kind,target,stack)
    if op==0x80 or op==0x81 or op==0x83:
        parsed=modrm(b,p,seg,op!=0x80)
        if not parsed:return None
        operand,p,reg,mode=parsed
        size=2 if op==0x81 else 1
        if p+size>LE:return None
        value=word(b,p) if size==2 else b[p];p+=size
        if op==0x83:value=s8(value)
        opname=("add","or","adc","sbb","and","sub","xor","cmp")[reg]
        stack= -value if op==0x83 and reg==0 and mode==3 and operand=="sp" else (value if op==0x83 and reg==5 and mode==3 and operand=="sp" else 0)
        return result("%s %s,%s"%(opname,operand,hex(value) if value>=0 else str(value)),"adjust" if stack else "other",stack=stack)
    if op in (0x8e,0x8b,0x89,0x8a,0x88,0xc4,0xc5,0x8d,0xff):
        parsed=modrm(b,p,seg,op not in (0x8a,0x88))
        if not parsed:return None
        operand,p,reg,mode=parsed
        if op==0x8e:return result("mov %s,%s"%(("es","cs","ss","ds")[reg] if reg<4 else "invalid",operand),"segment")
        if op==0xc4:return result("les %s,%s"%(REG16[reg],operand),"segment")
        if op==0xc5:return result("lds %s,%s"%(REG16[reg],operand),"segment")
        if op==0x8d:return result("lea %s,%s"%(REG16[reg],operand))
        if op==0x8b:return result("mov %s,%s"%(REG16[reg],operand))
        if op==0x89:return result("mov %s,%s"%(operand,REG16[reg]))
        if op==0x8a:return result("mov %s,%s"%(REG8[reg],operand))
        if op==0x88:return result("mov %s,%s"%(operand,REG8[reg]))
        if op==0xff:
            if reg==6:return result("push "+operand,"push",stack=2)
            if reg in (2,3):return result("call "+("far " if reg==3 else "near ")+operand,"call")
            if reg in (4,5):return result("jmp "+operand,"jmp")
            return result("ff/%d "%reg+operand)
    if op in (0x68,0x6a):
        if op==0x68 and p+2<=LE:
            val=word(b,p);p+=2;return result("push 0x%x"%val,"push",stack=2)
        if op==0x6a and p<LE:
            val=s8(b[p]);p+=1;return result("push %d"%val,"push",stack=2)
    if 0x50<=op<=0x57:return result("push "+REG16[op-0x50],"push",stack=2)
    if 0x58<=op<=0x5f:return result("pop "+REG16[op-0x58],"pop",stack=-2)
    if op in (0x06,0x0e,0x16,0x1e):
        return result("push "+{0x06:"es",0x0e:"cs",0x16:"ss",0x1e:"ds"}[op],"push",stack=2)
    if op in (0x07,0x17,0x1f):
        return result("pop "+{0x07:"es",0x17:"ss",0x1f:"ds"}[op],"pop",stack=-2)
    if 0x70<=op<=0x7f and p<LE:
        target=p+1+s8(b[p]);p+=1
        return result("jcc 0x%x"%target,"jcc",target)
    if op==0xeb and p<LE:
        target=p+1+s8(b[p]);p+=1;return result("jmp 0x%x"%target,"jmp",target)
    if op in (0xe8,0xe9) and p+2<=LE:
        target=p+2+s16(word(b,p));p+=2
        return result(("call" if op==0xe8 else "jmp")+" 0x%x"%target,"call" if op==0xe8 else "jmp",target)
    if op in (0xc3,0xcb):return result("ret" if op==0xc3 else "retf","ret")
    if op in (0xc2,0xca) and p+2<=LE:
        n=word(b,p);p+=2;return result(("ret" if op==0xc2 else "retf")+" %d"%n,"ret",stack=-n)
    if op==0x90:return result("nop")
    if op==0xc9:return result("leave","frame")
    if op==0x99:return result("cwd")
    if op==0x98:return result("cbw")
    if op==0x2b or op==0x0b:
        parsed=modrm(b,p,seg)
        if not parsed:return None
        operand,p,reg,mode=parsed
        return result(("sub " if op==0x2b else "or ")+REG16[reg]+","+operand)
    if op==0xa1 and p+2<=LE:
        disp=word(b,p);p+=2;return result("mov ax,"+(seg+":" if seg else "")+"[%04xh]"%disp)
    if op==0xa3 and p+2<=LE:
        disp=word(b,p);p+=2;return result("mov "+(seg+":" if seg else "")+"[%04xh],ax"%disp)
    if op==0x3d and p+2<=LE:
        val=word(b,p);p+=2;return result("cmp ax,0x%x"%val)
    if op==0x3c and p<LE:
        val=b[p];p+=1;return result("cmp al,0x%x"%val)
    if op==0xc1:
        parsed=modrm(b,p,seg)
        if not parsed or parsed[1]>=LE:return None
        operand,p,reg,mode=parsed;imm=b[p];p+=1
        return result(("shr" if reg==5 else "shift/%d"%reg)+" "+operand+","+str(imm))
    return None

def walk(b,start,end,tag):
    pos=start;ins=[];seen=set()
    while pos<end and pos not in seen:
        seen.add(pos)
        d=decode(b,pos)
        if not d or d[0]<1 or pos+d[0]>end:
            print("M413_STOP tag=%s file=0x%x reason=unsupported raw=%s"%(tag,pos,b[pos:min(end,pos+12)].hex()))
            break
        size,text,kind,target,stack=d
        print("M413_INSN tag=%s file=0x%x raw=%s text=%s kind=%s target=%s stack=%d"%(
            tag,pos,b[pos:pos+size].hex(),text,kind,hex(target) if target is not None else "none",stack))
        ins.append((pos,pos+size,kind,target))
        pos+=size
        if kind in ("jmp","ret"):break
    return ins

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    b=ap.parse_args().ovl.read_bytes()
    print("M413_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if len(b)<LE+2 or b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":
        raise SystemExit("M413_ERROR unexpected MZ/LE layout")
    for at in (0x2cc6e,0x2ccfc):
        print("M413_AND file=0x%x bytes=%s decode=%s"%(at,b[at:at+3].hex(),decode(b,at)))
    # Multiple anchored starts expose overlap; do not equate a matching path
    # with reachable control flow.
    for start in (0x2cb9c,0x2cba0,0x2cba6,0x2cbac,0x2cbb0,0x2cbb6,0x2cbbb,0x2cbbc):
        print("M413_ALLOC_ANCHOR file=0x%x"%start)
        walk(b,start,0x2cbcf,"alloc_%x"%start)
    for at in (0x2cbbb,0x2cbbc,0x2cbba,0x2cbc6):
        print("M413_ALLOC_FOCUS file=0x%x bytes=%s decode=%s"%(at,b[at:at+8].hex(),decode(b,at)))
    print("M413_ALLOC_RAW start=0x%x bytes=%s"%(ALLOC_START,b[ALLOC_START:ALLOC_END].hex()))
    print("M413_HELPER_CALL file=0x2ce51 bytes=%s target=0x%x"%(b[0x2ce51:0x2ce54].hex(),0x2ce54+s16(word(b,0x2ce52))))
    print("M413_HELPER_RAW start=0x%x bytes=%s"%(HELPER,b[HELPER:HELPER+192].hex()))
    for start in (HELPER,HELPER+1,HELPER+3):
        print("M413_HELPER_ANCHOR file=0x%x"%start)
        walk(b,start,HELPER+128,"helper_%x"%start)
    print("M413_VMFILEIO_POSTCALL start=0x2ce54")
    walk(b,0x2ce54,0x2ce71,"vmfileio_arguments")
    print("M413_LIMITATIONS staticOnly=1 arbitraryAnchors=1 partialDecoder=1 reachabilityUnverified=1 helperReturnConventionUnverified=1")
if __name__=="__main__":main()
