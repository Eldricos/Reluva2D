#!/usr/bin/env python3
"""M364: LE selector/fixup cross-check (read-only, no loader emulation).

Separates raw LE observations from hypotheses. In LE fixup terminology,
source type 7 is conventionally a 32-bit offset; target flag 0x10 is the
32-bit target-offset field-width flag. The target's object number and offset
are read separately. The CS selector base remains unknown at runtime.
"""
import argparse, hashlib, struct
from pathlib import Path

LE = 229780
SHA = "65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES = (("object_state", 566541, 19, 0x334, 0x12334, 1),
         ("global_state", 570965, 20, 0x488, 0x13488, 0))

def u16(b,p): return struct.unpack_from("<H",b,p)[0]
def u32(b,p): return struct.unpack_from("<I",b,p)[0]
def h(x): return "none" if x is None else "0x%x" % x

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    a=ap.parse_args()
    b=a.ovl.read_bytes()
    digest=hashlib.sha256(b).hexdigest()
    print("M364_INPUT bytes=%d sha256=%s"%(len(b),digest))
    if len(b)!=2008455 or digest!=SHA or b[LE:LE+2]!=b"LE":
        raise SystemExit("M364_ERROR unexpected STTNG.OVL")
    count=u32(b,LE+0x14); size=u32(b,LE+0x28)
    objtab=LE+u32(b,LE+0x40); objcount=u32(b,LE+0x44)
    pagemap=LE+u32(b,LE+0x48)
    fixindex=LE+u32(b,LE+0x68)
    fixrecords=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    if count!=371 or size!=4096 or objcount!=2:
        raise SystemExit("M364_ERROR unexpected LE layout")
    objs=[]
    for i in range(objcount):
        p=objtab+24*i
        objs.append(dict(number=i+1,virtual=u32(b,p),base=u32(b,p+4),
                         flags=u32(b,p+8),first=u32(b,p+12),pages=u32(b,p+16)))
    indexes=[u32(b,fixindex+4*i) for i in range(count+1)]
    if any(x>y for x,y in zip(indexes,indexes[1:])):
        raise SystemExit("M364_ERROR nonmonotonic fixup index")
    def page(logical):
        if not 1<=logical<=count:return None
        ent=b[pagemap+4*(logical-1):pagemap+4*logical]
        pn=int.from_bytes(ent[:3],"big")
        if not pn or ent[3]!=0:return None
        p=origin+(pn-1)*size
        return p if p+size<=len(b) else None
    def mapr(obj,rva):
        if not 1<=obj<=len(objs):return None
        o=objs[obj-1]
        if not 0<=rva<o["virtual"] or rva//size>=o["pages"]:return None
        p=page(o["first"]+rva//size)
        return None if p is None else p+rva%size
    def reverse(p):
        for o in objs:
            for i in range(o["pages"]):
                q=page(o["first"]+i)
                if q is not None and q<=p<q+size:
                    r=i*size+p-q
                    if r<o["virtual"]:return o["number"],r
        return None,None
    print("M364_LAYOUT pages=%d pageSize=%d objects=%d fixupIndex=%d fixupRecords=%d"%(count,size,objcount,fixindex,fixrecords))
    for o in objs:
        print("M364_OBJECT number=%d base=%s virtualSize=%s flags=%s firstPage=%d pages=%d"%
              (o["number"],h(o["base"]),h(o["virtual"]),h(o["flags"]),o["first"],o["pages"]))
    print("M364_FORMAT_RULES sourceType07=offset32 targetFlags10=targetOffset32 sourceOffset=pageRelative targetObject=oneBased targetOffset=objectRelative")
    print("M364_FORMAT_CAUTION sourceType07AndFlag10AreDifferentFields=1 runtimeSelectorBaseUnresolved=1")
    for name,site,logical,firstslot,disp,state0 in CASES:
        opcode=b[site:site+8]
        expected=b"\x2e\xff\x24\x85"+struct.pack("<I",disp)
        sourceobj,sourcerva=reverse(site)
        lo,hi=indexes[logical-1],indexes[logical]
        stream=b[fixrecords+lo:fixrecords+hi]
        print("M364_CASE_BEGIN name=%s dispatcherPhysical=%d dispatcherObject=%s dispatcherRva=%s instructionValid=%d displacement=%s fixupLogicalPage=%d"%
              (name,site,h(sourceobj),h(sourcerva),int(opcode==expected),h(disp),logical))
        found={}
        for pos in range(len(stream)-8):
            if stream[pos:pos+2]!=b"\x07\x10":continue
            src=u16(stream,pos+2)
            if firstslot<=src<firstslot+28 and (src-firstslot)%4==0:
                slot=(src-firstslot)//4
                found.setdefault(slot,[]).append((pos,src,stream[pos+4],u32(stream,pos+5)))
        physical=page(logical)
        if physical is None:raise SystemExit("M364_ERROR missing source page")
        sourcephysical=physical+firstslot
        sourcebytes=b[sourcephysical:sourcephysical+28]
        print("M364_SOURCE name=%s sourcePhysical=%s originalBytes=%s candidateCount=%d"%
              (name,h(sourcephysical),sourcebytes.hex(),sum(map(len,found.values()))))
        allvalid=True
        for i in range(7):
            entries=found.get(i,[])
            if len(entries)!=1:
                allvalid=False
                print("M364_SLOT name=%s state=%d candidates=%d"%(name,state0+i,len(entries)))
                continue
            pos,src,obj,target=entries[0]
            targetphysical=mapr(obj,target)
            if targetphysical is None:allvalid=False
            targetbase=objs[obj-1]["base"] if 1<=obj<=len(objs) else None
            print("M364_FIXUP name=%s state=%d recordAt=%d sourceType=0x07 flags=0x10 sourceOffset=%s targetObject=%d targetOffset=%s targetPhysical=%s preferredLinear=%s sourceOriginal=%s"%
                  (name,state0+i,pos,h(src),obj,h(target),h(targetphysical),
                   h(targetbase+target) if targetbase is not None else "none",
                   b[physical+src:physical+src+4].hex()))
        print("M364_FIXUP_SET name=%s sevenUniqueInBounds=%d"%(name,int(allvalid)))
        # The encoded x86 operand is an effective OFFSET in CS, not a linear
        # address. We can compute required CS descriptor bases for candidate
        # table locations; static LE metadata does not choose among them.
        for o in objs:
            for label,rva in (("fixup_source_rva",((logical-o["first"])*size+firstslot)),
                              ("displacement_as_rva",disp),
                              ("displacement_minus_preferred_base",disp-o["base"])):
                p=mapr(o["number"],rva)
                if p is None:continue
                linear=o["base"]+rva
                required_cs_base=linear-disp
                print("M364_CS_HYPOTHESIS name=%s object=%d location=%s rva=%s physical=%s preferredLinear=%s requiredCSBase=%s matchesFixupSource=%d bytes=%s"%
                      (name,o["number"],label,h(rva),h(p),h(linear),h(required_cs_base),
                       int(p==sourcephysical),b[p:p+28].hex()))
        print("M364_CASE_END name=%s"%name)
    print("M364_LIMITATIONS staticSelectorBaseUnknown=1 formatRuleNotValidatedAgainstRuntimeLoader=1 recordParserPrefixFiltered=1 codeDataReachabilityUnknown=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
