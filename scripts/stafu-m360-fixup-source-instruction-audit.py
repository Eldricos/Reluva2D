#!/usr/bin/env python3
"""M360: audit LE fixup page association against instruction streams.

Read-only diagnostic. Compares fixup source offsets on the indexed logical page
and neighboring pages, and disassembles from several anchored entry points.
Instruction overlap is evidence, not proof of loader semantics.
"""
import argparse, hashlib, struct
from pathlib import Path
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",19,0x334,1,566541,0x12334),
        ("global_state",20,0x488,0,570965,0x13488))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("ovl",type=Path)
    a=parser.parse_args();b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M360_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M360_ERROR unexpected executable")
    count=u32(b,LE+0x14);size=u32(b,LE+0x28)
    objtab=LE+u32(b,LE+0x40);pagemap=LE+u32(b,LE+0x48)
    fixindex=LE+u32(b,LE+0x68);fixrecords=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    if count!=371 or size!=4096:raise SystemExit("M360_ERROR unexpected LE page layout")
    starts=[u32(b,fixindex+4*i) for i in range(count+1)]
    obj1first=u32(b,objtab+12)
    def page(logical):
        if not 1<=logical<=count:return None
        raw=b[pagemap+4*(logical-1):pagemap+4*logical]
        n=int.from_bytes(raw[:3],"big")
        if not n or raw[3]:return None
        p=origin+(n-1)*size
        return p if p+size<=len(b) else None
    def physical_rva(rva):
        return (lambda p:None if p is None else p+rva%size)(page(obj1first+rva//size))
    md=Cs(CS_ARCH_X86,CS_MODE_32)
    def decode_window(physical,start,length):
        chunk=b[physical:physical+length]
        insns=list(md.disasm(chunk,start))
        return insns
    print("M360_LAYOUT pageCount=%d pageSize=%d object1FirstPage=%d dataOrigin=%d fixupRecordBase=%d"%(count,size,obj1first,origin,fixrecords))
    for name,logical,offset,state0,site,table in TABLES:
        lo,hi=starts[logical-1:logical+1]
        stream=b[fixrecords+lo:fixrecords+hi]
        matches={}
        for pos in range(len(stream)-8):
            if stream[pos:pos+2]!=bytes((7,16)):continue
            src=u16(stream,pos+2)
            if offset<=src<offset+28 and (src-offset)%4==0:
                matches.setdefault((src-offset)//4,[]).append((pos,stream[pos:pos+9]))
        print("M360_TABLE_BEGIN name=%s logical=%d fixupBytes=%d tableRva=0x%x sitePhysical=%d"%(name,logical,len(stream),table,site))
        for shift in (-2,-1,0,1,2):
            target_page=logical+shift
            base=page(target_page)
            if base is None:
                print("M360_PAGE_CANDIDATE name=%s shift=%d page=%d valid=0"%(name,shift,target_page))
                continue
            p=base+offset
            data=b[p:p+28]
            print("M360_PAGE_CANDIDATE name=%s shift=%d page=%d physical=%d tableBytes=%s"%(name,shift,target_page,p,data.hex()))
            for i in range(7):
                word=data[4*i:4*i+4]
                print("M360_SOURCE_WORD name=%s shift=%d state=%d physical=%d hex=%s fixupMatches=%d"%
                      (name,shift,state0+i,p+4*i,word.hex(),len(matches.get(i,[]))))
            # Disassemble across the 28-byte candidate range from its first byte.
            insns=decode_window(p,p,64)
            for ins in insns[:16]:
                overlap=max(ins.address,p)<min(ins.address+ins.size,p+28)
                if overlap:
                    print("M360_INSTRUCTION name=%s shift=%d address=%d size=%d bytes=%s text=%s %s"%
                          (name,shift,ins.address,ins.size,ins.bytes.hex(),ins.mnemonic,ins.op_str))
            for i in range(7):
                q=p+i*4
                ins=next(md.disasm(b[q:q+15],q,count=1),None)
                print("M360_ENTRY_DECODE name=%s shift=%d state=%d instruction=%s bytes=%s"%
                      (name,shift,state0+i,(ins.mnemonic+' '+ins.op_str) if ins else "decode_failed",ins.bytes.hex() if ins else "none"))
        # Anchor to verified dispatch opcode and examine its surrounding instructions.
        anchor=b[site:site+8]
        print("M360_DISPATCH_ANCHOR name=%s physical=%d bytes=%s expectedDisp=0x%x validOpcode=%d"%
              (name,site,anchor.hex(),table,int(anchor==bytes((0x2e,0xff,0x24,0x85))+struct.pack("<I",table))))
        for delta in (0,):
            for ins in decode_window(site+delta,site+delta,24)[:6]:
                print("M360_DISPATCH_INSTRUCTION name=%s physical=%d bytes=%s text=%s %s"%
                      (name,ins.address,ins.bytes.hex(),ins.mnemonic,ins.op_str))
        for i in range(7):
            records=matches.get(i,[])
            for pos,record in records:
                print("M360_FIXUP name=%s state=%d recordOffset=%d source=0x%x targetObject=%d targetRva=0x%x record=%s"%
                      (name,state0+i,pos,u16(record,2),record[4],u32(record,5),record.hex()))
        print("M360_TABLE_END name=%s"%name)
    print("M360_LIMITATIONS neighboringPageComparisonHeuristic=1 instructionBoundariesNotCFGValidated=1 completeLELoaderNotEmulated=1 fixupGrammarFromM358=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
