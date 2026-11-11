#!/usr/bin/env python3
"""M361: independently compare LE fixup page-index conventions.

Read-only. Parse candidate nine-byte internal offset32 records in the streams
associated with neighboring logical pages, then compare where their source
offsets would land under competing page-index conventions. No executable
bytes are modified; relocation simulation uses private bytearrays.
"""
import argparse, hashlib, struct
from pathlib import Path

LE=229780
EXPECTED_SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",19,0x334,1,0x12334),
        ("global_state",20,0x488,0,0x13488))
def u32(data,at): return struct.unpack_from("<I",data,at)[0]
def u16(data,at): return struct.unpack_from("<H",data,at)[0]
def fmt(x): return "none" if x is None else "0x%x"%x

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("ovl",type=Path)
    args=parser.parse_args()
    data=args.ovl.read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    print("M361_INPUT bytes=%d sha256=%s"%(len(data),digest))
    if len(data)!=2008455 or digest!=EXPECTED_SHA or data[LE:LE+2]!=b"LE":
        raise SystemExit("M361_ERROR unexpected STTNG.OVL")
    total=u32(data,LE+0x14)
    size=u32(data,LE+0x28)
    objtab=LE+u32(data,LE+0x40)
    pagemap=LE+u32(data,LE+0x48)
    fixidx=LE+u32(data,LE+0x68)
    fixrec=LE+u32(data,LE+0x6c)
    origin=LE+u32(data,LE+0x80)
    first=u32(data,objtab+12)
    if total!=371 or size!=4096 or first!=1:
        raise SystemExit("M361_ERROR unexpected LE layout")
    indexes=[u32(data,fixidx+i*4) for i in range(total+1)]
    if any(a>b for a,b in zip(indexes,indexes[1:])) or fixrec+indexes[-1]>len(data):
        raise SystemExit("M361_ERROR invalid fixup index")
    def page_start(logical):
        if not 1<=logical<=total:return None
        raw=data[pagemap+4*(logical-1):pagemap+4*logical]
        page=int.from_bytes(raw[:3],"big")
        if not page or raw[3]!=0:return None
        start=origin+(page-1)*size
        return start if start+size<=len(data) else None
    def stream_for(logical,mode):
        # mode=zero_based: page 1 uses table[0]..table[1].
        # mode=one_based: page 1 uses table[1]..table[2].
        index=logical-1 if mode=="zero_based" else logical
        if not 0<=index<total:return None
        return indexes[index],indexes[index+1],index
    def records(blob,offset):
        result=[]
        for pos in range(len(blob)-8):
            if blob[pos:pos+2]!=b"\x07\x10":continue
            src=u16(blob,pos+2)
            if offset<=src<offset+28 and (src-offset)%4==0:
                obj=blob[pos+4]
                target=u32(blob,pos+5)
                result.append((pos,(src-offset)//4,obj,target,blob[pos:pos+9].hex()))
        return result
    print("M361_LAYOUT totalPages=%d pageSize=%d fixupIndexPhysical=%d fixupRecordPhysical=%d dataOrigin=%d"%
          (total,size,fixidx,fixrec,origin))
    for logical in range(16,23):
        print("M361_INDEX logical=%d tableIndexZero=%d startZero=%d endZero=%d bytesZero=%d tableIndexOne=%d startOne=%d endOne=%d bytesOne=%d"%
              (logical,logical-1,indexes[logical-1],indexes[logical],indexes[logical]-indexes[logical-1],
               logical,indexes[logical],indexes[logical+1],indexes[logical+1]-indexes[logical]))
    for name,logical,offset,state0,table in TABLES:
        print("M361_TABLE_BEGIN name=%s expectedPage=%d offset=%s tableRva=%s"%(name,logical,fmt(offset),fmt(table)))
        for mode in ("zero_based","one_based"):
            lo,hi,index=stream_for(logical,mode)
            blob=data[fixrec+lo:fixrec+hi]
            found=records(blob,offset)
            matched={i:[] for i in range(7)}
            for rec in found:matched[rec[1]].append(rec)
            print("M361_INDEX_MODE name=%s mode=%s streamIndex=%d start=%d end=%d bytes=%d candidates=%d uniqueSlots=%d"%
                  (name,mode,index,lo,hi,hi-lo,len(found),sum(len(matched[i])==1 for i in range(7))))
            for i in range(7):
                print("M361_INDEX_SLOT name=%s mode=%s state=%d matches=%d details=%s"%
                      (name,mode,state0+i,len(matched[i]),
                       ";".join("%d:%d:%s:%s"%(pos,obj,fmt(target),raw)
                                for pos,_,obj,target,raw in matched[i]) or "none"))
            if any(len(matched[i])!=1 for i in range(7)):
                print("M361_RECONSTRUCTION name=%s mode=%s complete=0"% (name,mode))
                continue
            for page_shift in (-2,-1,0,1,2):
                target_logical=logical+page_shift
                p=page_start(target_logical)
                if p is None:continue
                p+=offset
                before=bytearray(data[p:p+28])
                after=bytearray(before)
                for i in range(7):
                    _,_,obj,target,_=matched[i][0]
                    if obj not in (1,2):continue
                    base=u32(data,objtab+(obj-1)*24+4)
                    struct.pack_into("<I",after,i*4,base+target)
                print("M361_RECONSTRUCTION name=%s mode=%s pageShift=%d targetLogical=%d sourcePhysical=%d complete=1 originalHex=%s patchedHex=%s changedBytes=%d"%
                      (name,mode,page_shift,target_logical,p,before.hex(),after.hex(),
                       sum(x!=y for x,y in zip(before,after))))
        # Evaluate whether each page's *own* fixup stream carries the
        # same seven source offsets. A match printed for another page is
        # not attributed to the expected page.
        for source_page in range(logical-2,logical+3):
            for mode in ("zero_based","one_based"):
                pair=stream_for(source_page,mode)
                if pair is None:continue
                lo,hi,index=pair
                found=records(data[fixrec+lo:fixrec+hi],offset)
                print("M361_NEIGHBOR_STREAM name=%s logical=%d mode=%s streamIndex=%d matches=%d slots=%s"%
                      (name,source_page,mode,index,len(found),
                       ",".join(str(v[1]) for v in found) if found else "none"))
        print("M361_TABLE_END name=%s"%name)
    print("M361_LIMITATIONS prefixRecordSearchNotFullGrammar=1 alternativeIndexingHypothetical=1 codeReachabilityNotProven=1 inMemoryOnly=1 runtimeLoaderUnverified=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
