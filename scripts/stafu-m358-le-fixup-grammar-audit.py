#!/usr/bin/env python3
"""M358: conservative LE fixup grammar and record-boundary audit (read-only).

Parse page fixup streams from their first byte, not by scanning for 07 10.
Supported ordinary internal-reference records: source kinds 0..8, target
types 0..3, 8/16-bit object ordinals, 16/32-bit offsets, additive fields,
and source lists. Unsupported variants are explicitly reported. Candidate
records are only validated if a complete page parse reaches their offsets.
"""
import argparse, hashlib, struct
from functools import lru_cache
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",19,0x334,1),("global_state",20,0x488,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def hx(x):return "0x%x"%x
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
    b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M358_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M358_ERROR unexpected OVL")
    total=u32(b,LE+0x14);pagesize=u32(b,LE+0x28);fixidx=LE+u32(b,LE+0x68);fixrec=LE+u32(b,LE+0x6c)
    starts=[u32(b,fixidx+4*i) for i in range(total+1)]
    if pagesize!=4096 or total!=371 or any(x>y for x,y in zip(starts,starts[1:])):raise SystemExit("M358_ERROR layout")
    print("M358_LAYOUT pages=%d pageSize=%d fixupRecords=%d"%(total,pagesize,fixrec))
    def parse_at(blob,pos,source_list_mode="count_first"):
        if pos+2>len(blob):return None
        src,flags=blob[pos],blob[pos+1]
        source_kind=src&0x0f;list_mode=bool(src&0x20);alias=bool(src&0x10)
        # LE source types: byte=0, selector16=2, pointer32=3,
        # offset16=5, pointer48=6, offset32=7, selfrelative32=8.
        if source_kind not in (0,2,3,5,6,7,8) or alias or (src&0xc0):return None
        target_type=flags&3
        if target_type!=0:return None  # internal target only
        if flags&0x80:return None       # ordinal width extension not handled
        # LE target flags: 0x10 32-bit offset; 0x40 16-bit object ordinal;
        # 0x04 additive fixup; 0x20 32-bit additive.
        if flags&0x08:return None
        p=pos+2;count=1;offsets=[]
        if list_mode:
            if p>=len(blob):return None
            count=blob[p];p+=1
            if not 1<=count<=64:return None
        else:
            if p+2>len(blob):return None
            offsets=[u16(blob,p)];p+=2
        objwide=bool(flags&0x40)
        if p+(2 if objwide else 1)>len(blob):return None
        obj=u16(blob,p) if objwide else blob[p];p+=2 if objwide else 1
        wide=bool(flags&0x10)
        if p+(4 if wide else 2)>len(blob):return None
        target=u32(blob,p) if wide else u16(blob,p);p+=4 if wide else 2
        if flags&0x04:
            addwide=bool(flags&0x20)
            if p+(4 if addwide else 2)>len(blob):return None
            p+=4 if addwide else 2
        elif flags&0x20:
            return None
        if list_mode:
            if p+count*2>len(blob):return None
            offsets=[u16(blob,p+2*i) for i in range(count)]
            p+=count*2
        if obj not in (1,2):return None
        if any(off>=4096 for off in offsets):return None
        return p,src,flags,obj,target,offsets
    for name,logical,first,first_state in TABLES:
        blob=b[fixrec+starts[logical-1]:fixrec+starts[logical]]
        print("M358_PAGE_BEGIN name=%s logical=%d bytes=%d physical=%d"%(name,logical,len(blob),fixrec+starts[logical-1]))
        # A page must be parsed sequentially. No searching for candidate prefixes
        # to determine record boundaries.
        @lru_cache(None)
        def paths(pos):
            if pos==len(blob):return ((),)
            if pos>len(blob):return ()
            parsed=parse_at(blob,pos)
            if parsed is None:return ()
            end=parsed[0]
            tails=paths(end)
            return tuple(((pos,parsed),)+tail for tail in tails[:2])
        solutions=paths(0)
        print("M358_PAGE_PARSE name=%s completePaths=%d"%(name,len(solutions)))
        if not solutions:
            pos=0;num=0
            while pos<len(blob):
                r=parse_at(blob,pos)
                if r is None:break
                pos=r[0];num+=1
            print("M358_PARSE_STOP name=%s records=%d stopOffset=%d remaining=%d nextHex=%s"%
                  (name,num,pos,len(blob)-pos,blob[pos:pos+24].hex()))
            print("M358_PAGE_END name=%s verified=0"%name)
            continue
        solution=solutions[0];by_source={}
        for pos,r in solution:
            end,src,flags,obj,target,offsets=r
            for offset in offsets:by_source.setdefault(offset,[]).append((pos,end,src,flags,obj,target))
        print("M358_PARSE_SUMMARY name=%s records=%d sources=%d finalOffset=%d"%
              (name,len(solution),sum(len(r[1][5]) for r in solution),solution[-1][1][0] if solution else 0))
        for i in range(7):
            offset=first+4*i
            matches=by_source.get(offset,[])
            print("M358_ENTRY name=%s state=%d source=%s parsedRecords=%d"%
                  (name,first_state+i,hx(offset),len(matches)))
            for pos,end,src,flags,obj,target in matches:
                print("M358_VERIFIED_RECORD name=%s state=%d recordPos=%d end=%d srcType=%s flags=%s object=%d target=%s bytes=%s"%
                      (name,first_state+i,pos,end,hx(src),hx(flags),obj,hx(target),blob[pos:end].hex()))
        print("M358_PAGE_END name=%s verified=1"%name)
    print("M358_LIMITATIONS internalTargetOnly=1 uncommonSourceFlagsUnsupported=1 externalAndImportFixupsUnsupported=1 noRuntimeExecution=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
