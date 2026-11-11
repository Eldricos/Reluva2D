#!/usr/bin/env python3
"""M367: read-only full-stream LE fixup reconstruction for pages 19 and 20.

Sequentially parses every internal fixup record. Applies only unambiguous
offset32 (source kind 7) internal fixups to copied pages; other source kinds
are counted and left unchanged. This is a partial relocation simulation,
not an emulation of the original loader.
"""
import argparse,hashlib,struct
from collections import Counter
from pathlib import Path

LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",19,0x334,1),("global_state",20,0x488,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def parse(blob,pos):
    start=pos
    if pos+2>len(blob):raise ValueError("truncated header")
    src,flags=blob[pos:pos+2];pos+=2
    kind=src&15
    if kind not in (0,2,3,5,6,7,8) or src&0xd0 or flags&0x8b:
        raise ValueError("unsupported header at %d: %02x %02x"%(start,src,flags))
    if flags&3:raise ValueError("non-internal target at %d"%start)
    if src&0x20:
        if pos>=len(blob):raise ValueError("missing list count")
        count=blob[pos];pos+=1
        if not 1<=count<=64:raise ValueError("invalid source count")
        offsets=[]
    else:
        count=1
        if pos+2>len(blob):raise ValueError("missing source")
        offsets=[u16(blob,pos)];pos+=2
    objwide=bool(flags&0x40)
    if pos+(2 if objwide else 1)>len(blob):raise ValueError("missing object")
    obj=u16(blob,pos) if objwide else blob[pos];pos+=2 if objwide else 1
    wide=bool(flags&0x10)
    if pos+(4 if wide else 2)>len(blob):raise ValueError("missing target")
    target=u32(blob,pos) if wide else u16(blob,pos);pos+=4 if wide else 2
    additive=None
    if flags&4:
        addwide=bool(flags&0x20)
        if pos+(4 if addwide else 2)>len(blob):raise ValueError("missing additive")
        additive=u32(blob,pos) if addwide else u16(blob,pos)
        pos+=4 if addwide else 2
    elif flags&0x20:raise ValueError("additive width without additive")
    if src&0x20:
        if pos+count*2>len(blob):raise ValueError("missing source list")
        offsets=[u16(blob,pos+2*i) for i in range(count)];pos+=count*2
    if obj not in (1,2) or any(x>=4096 for x in offsets):
        raise ValueError("invalid object/source at %d"%start)
    return dict(start=start,end=pos,kind=kind,src=src,flags=flags,obj=obj,
                target=target,additive=additive,offsets=offsets)
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args();b=args.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M367_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M367_ERROR unexpected OVL")
    total=u32(b,LE+0x14);ps=u32(b,LE+0x28)
    ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
    fixidx=LE+u32(b,LE+0x68);fixrec=LE+u32(b,LE+0x6c)
    origin=LE+u32(b,LE+0x80)
    objects=[]
    for i in range(u32(b,LE+0x44)):
        off=ot+24*i
        objects.append(dict(size=u32(b,off),base=u32(b,off+4),first=u32(b,off+12),count=u32(b,off+16)))
    if total!=371 or ps!=4096 or len(objects)!=2 or objects[0]["first"]!=1:
        raise SystemExit("M367_ERROR unexpected layout")
    starts=[u32(b,fixidx+4*i) for i in range(total+1)]
    if any(a>c for a,c in zip(starts,starts[1:])):raise SystemExit("M367_ERROR fixup index")
    def page(logical):
        e=b[pm+4*(logical-1):pm+4*logical]
        number=int.from_bytes(e[:3],"big")
        if not number or e[3]:raise ValueError("unsupported page map")
        p=origin+(number-1)*ps
        if p+ps>len(b):raise ValueError("page beyond file")
        return bytearray(b[p:p+ps]),p
    print("M367_LAYOUT pages=%d pageSize=%d objects=%s"%
          (total,ps,",".join("%d:base0x%x:size0x%x"%(i+1,o["base"],o["size"]) for i,o in enumerate(objects))))
    for name,logical,first,state0 in CASES:
        raw,physical=page(logical)
        stream=b[fixrec+starts[logical-1]:fixrec+starts[logical]]
        records=[];pos=0
        while pos<len(stream):
            try:r=parse(stream,pos)
            except ValueError as e:raise SystemExit("M367_PARSE_ERROR name=%s pos=%d reason=%s"%(name,pos,e))
            records.append(r);pos=r["end"]
        counts=Counter(r["kind"] for r in records)
        sources=sum(len(r["offsets"]) for r in records)
        print("M367_PAGE name=%s logical=%d physical=0x%x fixupBytes=%d records=%d sources=%d complete=%d"%
              (name,logical,physical,len(stream),len(records),sources,int(pos==len(stream))))
        print("M367_SOURCE_TYPES name=%s %s"%(name," ".join("kind%d=%d"%(k,counts[k]) for k in sorted(counts))))
        relocated=bytearray(raw);applied=[];skipped=[];collisions=[]
        written={}
        for r in records:
            obj=objects[r["obj"]-1]
            for source in r["offsets"]:
                # Offset32 internal target is the only source format whose
                # write width and base semantics are simulated here.
                if r["kind"]!=7 or r["additive"] is not None:
                    skipped.append((r,source,"source_kind_or_additive_unsupported"));continue
                if source+4>ps:
                    skipped.append((r,source,"cross_page_source"));continue
                if r["target"]>=obj["size"]:
                    skipped.append((r,source,"target_out_of_bounds"));continue
                value=obj["base"]+r["target"]
                if value>0xffffffff:
                    skipped.append((r,source,"value_overflow"));continue
                if any(x in written for x in range(source,source+4)):
                    collisions.append((r,source));continue
                before=bytes(relocated[source:source+4])
                relocated[source:source+4]=struct.pack("<I",value)
                for x in range(source,source+4):written[x]=r["start"]
                applied.append((r,source,value,before))
        print("M367_APPLY_SUMMARY name=%s appliedSources=%d skippedSources=%d collisions=%d modifiedBytes=%d"%
              (name,len(applied),len(skipped),len(collisions),sum(a!=c for a,c in zip(raw,relocated))))
        for kind,count in sorted(Counter((r["kind"],reason) for r,_,reason in skipped).items()):
            print("M367_SKIPPED_SUMMARY name=%s kind=%d reason=%s count=%d"%(name,kind[0],kind[1],count))
        for r,source in collisions[:20]:
            print("M367_COLLISION name=%s record=%d source=0x%x"%(name,r["start"],source))
        print("M367_TABLE name=%s original=%s reconstructed=%s"%
              (name,raw[first:first+28].hex(),relocated[first:first+28].hex()))
        for i in range(7):
            source=first+4*i
            matches=[(r,value,before) for r,s,value,before in applied if s==source]
            if len(matches)==1:
                r,value,before=matches[0]
                print("M367_ENTRY name=%s state=%d source=0x%x targetObject=%d targetRva=0x%x linear=0x%x original=%s relocated=%s record=%d"%
                      (name,state0+i,source,r["obj"],r["target"],value,before.hex(),relocated[source:source+4].hex(),r["start"]))
                target_obj=objects[r["obj"]-1]
                target_logical=target_obj["first"]+r["target"]//ps
                target_page,target_physical=page(target_logical)
                t=r["target"]%ps
                print("M367_TARGET name=%s state=%d logical=%d physical=0x%x targetOriginalContext=%s"%
                      (name,state0+i,target_logical,target_physical+t,
                       bytes(target_page[max(0,t-8):min(ps,t+24)]).hex()))
            else:
                print("M367_ENTRY name=%s state=%d source=0x%x appliedMatches=%d"%(name,state0+i,source,len(matches)))
        print("M367_PAGE_END name=%s"%name)
    print("M367_LIMITATIONS offset32Only=1 additiveUnsupported=1 otherSourceKindsNotApplied=1 crossPageNotApplied=1 preferredBasesAssumed=1 targetContextUnrelocated=1 noRuntimeLoaderValidation=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
