#!/usr/bin/env python3
"""STAFU M412: raw-byte call boundaries and conservative stack-lifetime windows.

Read-only diagnostic. No runtime execution, reachability, or ABI claims.
Requires sibling M409 decoder for supplementary (not authoritative) disassembly.
"""
import argparse
import hashlib
import importlib.util
import struct
from pathlib import Path

LE=0x38194
SITES=[
 ("ALLOC_A",0x28a70,8),("ALLOC_B",0x2cbc7,8),
 ("BROADCAST_A",0x2cc8b,12),("BROADCAST_B",0x2cd19,12),
 ("VMFILEIO",0x2ce6d,12),
]
PREFIX={0x26:"es",0x2e:"cs",0x36:"ss",0x3e:"ds"}
REG=("ax","cx","dx","bx","sp","bp","si","di")
BASE=("bx+si","bx+di","bp+si","bp+di","si","di","bp","bx")

def decode_m409():
    p=Path(__file__).with_name("stafu-m409-corrected-indirect-call-decoder.py")
    spec=importlib.util.spec_from_file_location("stafu_m409",p)
    module=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.decode

def signed(v,bits):
    return v-(1<<bits) if v & (1<<(bits-1)) else v

def mem(b,pos,seg=None):
    if pos>=LE:return None
    m=b[pos];pos+=1
    mode=m>>6;reg=(m>>3)&7;rm=m&7
    if mode==3:return (REG[rm],pos,reg,mode)
    if mode==0 and rm==6:
        if pos+2>LE:return None
        op="[%04xh]"%struct.unpack_from("<H",b,pos)[0];pos+=2
    else:
        name=BASE[rm];disp=0
        if mode==1:
            if pos>=LE:return None
            disp=signed(b[pos],8);pos+=1
        elif mode==2:
            if pos+2>LE:return None
            disp=signed(struct.unpack_from("<H",b,pos)[0],16);pos+=2
        op="["+name+(("%+d"%disp) if disp else "")+"]"
    return ((seg+":" if seg else "")+op,pos,reg,mode)

def instruction(b,at):
    """Small strict 16-bit decoder for known call-site instructions."""
    p=at;seg=None
    if p>=LE:return None
    if b[p] in PREFIX:
        seg=PREFIX[b[p]];p+=1
    if p>=LE:return None
    op=b[p];p+=1
    def out(t,category="other",stack=0,uncertain=False):
        return (p-at,t,category,stack,uncertain)
    if 0x50<=op<=0x57 and not seg:return out("push "+REG[op-0x50],"push",2)
    if 0x58<=op<=0x5f and not seg:return out("pop "+REG[op-0x58],"pop",-2)
    if op in (0x06,0x0e,0x16,0x1e) and not seg:
        return out("push "+{6:"es",14:"cs",22:"ss",30:"ds"}[op],"push",2)
    if op in (0x07,0x17,0x1f) and not seg:
        return out("pop "+{7:"es",23:"ss",31:"ds"}[op],"pop",-2)
    if op==0x68 and not seg and p+2<=LE:
        val=struct.unpack_from("<H",b,p)[0];p+=2
        return out("push 0x%x"%val,"push",2)
    if op==0x6a and not seg and p<LE:
        val=signed(b[p],8);p+=1
        return out("push %d"%val,"push",2)
    if op in (0x8e,0x8b,0x89,0x8d,0x8c,0x8a,0x88,0xc4,0xc5,0xff,0x80,0x81,0x83,0xc6,0xc7,0x20,0x21,0x22,0x23,0x24,0x25):
        if op in (0x24,0x25):
            size=1 if op==0x24 else 2
            if p+size>LE:return None
            val=b[p] if size==1 else struct.unpack_from("<H",b,p)[0]
            p+=size;return out("and "+("al" if size==1 else "ax")+",0x%x"%val)
        parsed=mem(b,p,seg)
        if not parsed:return None
        operand,p,reg,mode=parsed
        if op==0xff:
            if reg==6:return out("push "+operand,"push",2)
            if reg in (2,3):return out("call "+("far " if reg==3 else "near ")+operand,"call",0,True)
            if reg in (4,5):return out("jmp "+operand,"barrier",0,True)
            return out("ff/%d "%reg+operand)
        if op==0x8e:
            if reg>3 or reg==1:return None
            return out("mov "+("es","cs","ss","ds")[reg]+","+operand,"segment")
        if op==0x8c:
            if reg>3:return None
            return out("mov "+operand+","+("es","cs","ss","ds")[reg])
        if op in (0xc4,0xc5):
            return out(("les " if op==0xc4 else "lds ")+REG[reg]+","+operand,"segment")
        if op==0x8d:return out("lea "+REG[reg]+","+operand)
        if op in (0x8b,0x8a):return out("mov "+(REG[reg] if op==0x8b else ("al","cl","dl","bl","ah","ch","dh","bh")[reg])+","+operand)
        if op in (0x89,0x88):return out("mov "+operand+","+(REG[reg] if op==0x89 else ("al","cl","dl","bl","ah","ch","dh","bh")[reg]))
        if op in (0x80,0x81,0x83,0xc6,0xc7):
            if op in (0xc6,0xc7) and reg!=0:return None
            size=2 if op in (0x81,0xc7) else 1
            if p+size>LE:return None
            val=b[p] if size==1 else struct.unpack_from("<H",b,p)[0]
            p+=size
            if op==0x83:val=signed(val,8)
            names=("add","or","adc","sbb","and","sub","xor","cmp")
            name="mov" if op in (0xc6,0xc7) else names[reg]
            if op==0x83 and mode==3 and (b[at+(1 if seg else 0)+1]&7)==4 and reg==0:
                return out("add sp,%d"%val,"adjust",-val)
            if op==0x83 and mode==3 and (b[at+(1 if seg else 0)+1]&7)==4 and reg==5:
                return out("sub sp,%d"%val,"adjust",val)
            return out(name+" "+operand+","+str(val))
        if op in (0x20,0x21,0x22,0x23):
            r=("al","cl","dl","bl","ah","ch","dh","bh")[reg] if op in (0x20,0x22) else REG[reg]
            return out("and "+(r+","+operand if op in (0x22,0x23) else operand+","+r))
    if op==0xe8 and p+2<=LE:
        rel=signed(struct.unpack_from("<H",b,p)[0],16);p+=2
        return out("call near 0x%x"%(p+rel),"call",0,True)
    if op==0x9a and p+4<=LE:
        off,selector=struct.unpack_from("<HH",b,p);p+=4
        return out("call far %04x:%04x"%(selector,off),"call",0,True)
    if op in (0xc3,0xcb):return out("return","barrier",0,True)
    if op in (0xc2,0xca) and p+2<=LE:
        p+=2;return out("return imm","barrier",0,True)
    if op==0xeb and p<LE:
        rel=signed(b[p],8);p+=1
        return out("jmp short 0x%x"%(p+rel),"barrier",0,True)
    if op==0xe9 and p+2<=LE:
        rel=signed(struct.unpack_from("<H",b,p)[0],16);p+=2
        return out("jmp near 0x%x"%(p+rel),"barrier",0,True)
    if 0x70<=op<=0x7f and p<LE:
        rel=signed(b[p],8);p+=1
        return out("jcc 0x%x"%(p+rel),"branch",0,True)
    if op in (0x90,0xc9):return out("nop" if op==0x90 else "leave","other",0,op==0xc9)
    return None

def decode_paths(b,end,window=90):
    paths=[]
    for anchor in range(max(0,end-window),end):
        p=anchor;seq=[]
        while p<end:
            d=instruction(b,p)
            if not d or p+d[0]>end:break
            seq.append((p,*d));p+=d[0]
        if p==end and seq:paths.append((anchor,seq))
    return paths

def segment_events(seq):
    return [(r[0],r[2]) for r in seq if r[3]=="segment"]

def report(b,name,opcode,cleanup):
    start=opcode-1 if b[opcode-1] in PREFIX else opcode
    print("M412_TARGET name=%s opcode=0x%x start=0x%x cleanup=%d bytes=%s"%(
        name,opcode,start,cleanup,b[start:opcode+4].hex()))
    print("M412_RAW_WINDOW name=%s start=0x%x bytes=%s"%(
        name,max(0,start-48),b[max(0,start-48):opcode+12].hex()))
    paths=decode_paths(b,start)
    valid=[]
    for anchor,seq in paths:
        if any(row[3]=="barrier" for row in seq):continue
        # Caller cleanup and calls make stack lifetime ambiguous; only count
        # pushes after the latest call or explicit stack adjustment.
        last_reset=max((i for i,r in enumerate(seq) if r[3] in ("call","adjust")),default=-1)
        suffix=seq[last_reset+1:]
        pushed=sum(r[4] for r in suffix if r[3] in ("push","pop"))
        calls=sum(r[3]=="call" for r in seq)
        valid.append((anchor,seq,suffix,pushed,calls))
    valid.sort(key=lambda x:(x[3]!=cleanup,-len(x[2]),x[4],-x[0]))
    print("M412_PATH_COUNTS name=%s total=%d withoutUnconditionalBarrier=%d"%(
        name,len(paths),len(valid)))
    for rank,(anchor,seq,suffix,pushed,calls) in enumerate(valid[:4],1):
        print("M412_PATH name=%s rank=%d anchor=0x%x lastLifetimeStart=0x%x pushedBytes=%d cleanupBytes=%d match=%d earlierCalls=%d"%(
            name,rank,anchor,suffix[0][0] if suffix else start,pushed,cleanup,int(pushed==cleanup),calls))
        for addr,length,ins,kind,delta,uncertain in seq[-25:]:
            print("M412_INSN name=%s rank=%d file=0x%x raw=%s text=%s kind=%s stack=%d uncertain=%d"%(
                name,rank,addr,b[addr:addr+length].hex(),ins,kind,delta,int(uncertain)))
        print("M412_SEGMENTS name=%s rank=%d events=%s"%(
            name,rank,";".join("0x%x:%s"%x for x in segment_events(seq)) or "none"))
    if not valid:print("M412_NO_VALID_PATH name=%s"%name)
    after=b[opcode+4:opcode+12]
    print("M412_POST name=%s bytes=%s"% (name,after.hex()))

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    b=ap.parse_args().ovl.read_bytes()
    print("M412_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
    if len(b)<LE+2 or b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":
        raise SystemExit("M412_ERROR unexpected MZ/LE layout")
    # Verify the byte-level opcode responsible for M411's generic ALU label.
    for at in (0x2cc6e,0x2ccfc):
        print("M412_AND_CHECK file=0x%x raw=%s decoded=%s"%(
            at,b[at:at+6].hex(),str(instruction(b,at))))
    for name,opcode,cleanup in SITES:report(b,name,opcode,cleanup)
    print("M412_NOTE earlier calls are lifetime boundaries; callee cleanup is unknown")
    print("M412_LIMITATIONS arbitraryAnchors=1 partialDecoder=1 controlFlowUnverified=1 segmentValuesUnverified=1 noABIAssumptions=1")
if __name__=="__main__":main()
