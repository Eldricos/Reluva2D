#!/usr/bin/env python3
"""M362: audit LE fixup source/target semantics without modifying the OVL.

Reports raw record fields, source-page offsets, target-object bounds, selector
interpretations, and whether relocation writes overlap instruction-like bytes.
The LE interpretation is a hypothesis; no loader execution is claimed.
"""
import argparse,hashlib,struct
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
TABLES=(("object_state",19,0x334,1,0x12334),("global_state",20,0x488,0,0x13488))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def hx(x):return "0x%x"%x
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
    b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M362_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M362_ERROR unexpected executable")
    count=u32(b,LE+0x14);pagesize=u32(b,LE+0x28)
    objtab=LE+u32(b,LE+0x40);objcount=u32(b,LE+0x44)
    mapoff=LE+u32(b,LE+0x48);idxoff=LE+u32(b,LE+0x68)
    recoff=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
    if count!=371 or pagesize!=4096 or objcount!=2:raise SystemExit("M362_ERROR unexpected layout")
    objects=[]
    for i in range(objcount):
        p=objtab+i*24
        objects.append((u32(b,p),u32(b,p+4),u32(b,p+12),u32(b,p+16)))
    indexes=[u32(b,idxoff+4*i) for i in range(count+1)]
    def physical_page(logical):
        if not 1<=logical<=count:return None
        entry=b[mapoff+4*(logical-1):mapoff+4*logical]
        n=int.from_bytes(entry[:3],"big")
        if not n or entry[3]:return None
        return origin+(n-1)*pagesize
    def map_rva(obj,rva):
        size,base,first,n=objects[obj-1]
        if rva>=size or rva<0 or rva//pagesize>=n:return None
        p=physical_page(first+rva//pagesize)
        return None if p is None else p+rva%pagesize
    print("M362_LAYOUT pageSize=%d pages=%d objectCount=%d fixupIndex=%d fixupRecords=%d"%(pagesize,count,objcount,idxoff,recoff))
    for i,(size,base,first,n) in enumerate(objects,1):
        print("M362_OBJECT number=%d size=%s preferredBase=%s firstLogicalPage=%d pageCount=%d"%(i,hx(size),hx(base),first,n))
    print("M362_INTERPRETATION sourceType07=32bitOffsetCandidate targetFlags10=32bitTargetOffsetCandidate sourceOffsetsPageRelativeCandidate=1 targetOffsetsObjectRelativeCandidate=1 selectorBaseUnknown=1")
    for name,logical,offset,state0,disp in TABLES:
        lo,hi=indexes[logical-1:logical+1]
        stream=b[recoff+lo:recoff+hi]
        print("M362_TABLE_BEGIN name=%s logicalPage=%d recordBytes=%d tableDisplacement=%s"%(name,logical,len(stream),hx(disp)))
        found={}
        for pos in range(len(stream)-8):
            if stream[pos:pos+2]!=b"\x07\x10":continue
            source=u16(stream,pos+2)
            if offset<=source<offset+28 and (source-offset)%4==0:
                found.setdefault((source-offset)//4,[]).append((pos,source,stream[pos+4],u32(stream,pos+5),stream[pos:pos+9].hex()))
        for i in range(7):
            candidates=found.get(i,[])
            print("M362_SLOT name=%s state=%d candidateCount=%d"%(name,state0+i,len(candidates)))
            for pos,source,obj,target,raw in candidates:
                sourcep=physical_page(logical)
                sourcep=None if sourcep is None else sourcep+source
                targetp=map_rva(obj,target) if 1<=obj<=objcount else None
                targetbase=objects[obj-1][1] if 1<=obj<=objcount else None
                targetsize=objects[obj-1][0] if 1<=obj<=objcount else None
                rawsource=b[sourcep:sourcep+4].hex() if sourcep is not None else "none"
                patched=struct.pack("<I",(targetbase+target)&0xffffffff).hex() if targetbase is not None else "none"
                print("M362_RECORD name=%s state=%d streamOffset=%d raw=%s sourceType=%s flags=%s sourcePageOffset=%s sourcePhysical=%s sourceBytes=%s targetObject=%d targetOffset=%s targetObjectSize=%s targetInBounds=%d targetPhysical=%s preferredLinear=%s patchedBytes=%s"%
                      (name,state0+i,pos,raw,hx(7),hx(16),hx(source),hx(sourcep) if sourcep is not None else "none",rawsource,obj,hx(target),
                       hx(targetsize) if targetsize is not None else "none",int(targetsize is not None and target<targetsize),
                       hx(targetp) if targetp is not None else "none",hx(targetbase+target) if targetbase is not None else "none",patched))
        p=physical_page(logical)
        if p is not None:
            p+=offset
            print("M362_SOURCE_WINDOW name=%s sourcePhysical=%d before16=%s after16=%s"%
                  (name,p,b[p-16:p].hex(),b[p+28:p+44].hex()))
        # Compare displacement with object-relative and preferred-linear readings.
        for obj,(size,base,first,n) in enumerate(objects,1):
            for interpretation,rva in (("object_relative",disp),("preferred_linear_minus_base",disp-base)):
                p=map_rva(obj,rva)
                print("M362_DISPLACEMENT name=%s object=%d interpretation=%s candidateRva=%s mappedPhysical=%s candidateBytes=%s"%
                      (name,obj,interpretation,hx(rva),hx(p) if p is not None else "none",b[p:p+28].hex() if p is not None else "none"))
        print("M362_TABLE_END name=%s"%name)
    print("M362_LIMITATIONS fixupSemanticNamesHypotheses=1 exactSourceRecordsFromPrefixSearch=1 runtimeSelectorsNotResolved=1 noInstructionCFG=1 noLoaderEmulation=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
