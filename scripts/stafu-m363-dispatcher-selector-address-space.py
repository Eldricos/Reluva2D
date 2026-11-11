#!/usr/bin/env python3
"""M363: compare LE dispatcher address spaces and selector-base hypotheses.

Read-only diagnostic; no original executable is modified. The CS selector base
is not recovered from static bytes, so mappings are explicitly hypothetical.
"""
import argparse,hashlib,struct
from pathlib import Path
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
SITES=(("object_state",566541,0x12334),("global_state",570965,0x13488))
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def signed32(x):return x if x<0x80000000 else x-0x100000000
def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path)
    a=ap.parse_args();b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
    print("M363_INPUT bytes=%d sha256=%s"%(len(b),sha))
    if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M363_ERROR unexpected executable")
    n=u32(b,LE+0x14);ps=u32(b,LE+0x28)
    ot=LE+u32(b,LE+0x40);oc=u32(b,LE+0x44)
    pm=LE+u32(b,LE+0x48);origin=LE+u32(b,LE+0x80)
    if n!=371 or ps!=4096 or oc!=2:raise SystemExit("M363_ERROR unexpected LE layout")
    objs=[]
    for i in range(oc):
        p=ot+i*24
        objs.append(dict(number=i+1,size=u32(b,p),base=u32(b,p+4),flags=u32(b,p+8),first=u32(b,p+12),count=u32(b,p+16)))
    def page(logical):
        if not 1<=logical<=n:return None
        raw=b[pm+4*(logical-1):pm+4*logical]
        index=int.from_bytes(raw[:3],"big")
        if not index or raw[3]:return None
        p=origin+(index-1)*ps
        return p if p+ps<=len(b) else None
    def mapr(obj,rva):
        if rva<0 or rva>=obj["size"] or rva//ps>=obj["count"]:return None
        p=page(obj["first"]+rva//ps)
        return None if p is None else p+rva%ps
    def reverse(p):
        for o in objs:
            for i in range(o["count"]):
                q=page(o["first"]+i)
                if q is not None and q<=p<q+ps:
                    rva=i*ps+p-q
                    if rva<o["size"]:return o,rva
        return None,None
    print("M363_LAYOUT pages=%d pageSize=%d objectCount=%d dataOrigin=%d"%(n,ps,oc,origin))
    for o in objs:
        print("M363_OBJECT number=%d size=0x%x preferredBase=0x%x flags=0x%x firstPage=%d pageCount=%d"%
              (o["number"],o["size"],o["base"],o["flags"],o["first"],o["count"]))
    # Header fields relevant to entry and selector resolution.
    for label,off in (("eipObject",0x18),("eip",0x1c),("espObject",0x20),("esp",0x24),
                      ("autoDataObject",0x94),("preloadPages",0x84),("instanceDemandPages",0x88)):
        print("M363_HEADER_FIELD name=%s offset=0x%x value=0x%x"%(label,off,u32(b,LE+off)))
    for name,site,disp in SITES:
        o,rva=reverse(site)
        anchor=b[site:site+8]
        print("M363_DISPATCH name=%s physical=%d object=%s instructionRva=%s preferredLinear=%s raw=%s opcodeValid=%d displacement=0x%x"%
              (name,site,o["number"] if o else "none","0x%x"%rva if rva is not None else "none",
               "0x%x"%(o["base"]+rva) if o else "none",anchor.hex(),
               int(anchor==b"\x2e\xff\x24\x85"+struct.pack("<I",disp)),disp))
        for obj in objs:
            # These are alternative models, not claims about the actual CS descriptor.
            hypotheses=(("CS_base_zero",disp),
                        ("CS_base_object_preferred",disp-obj["base"]),
                        ("CS_base_object_rva_zero",disp))
            for hypothesis,candidate in hypotheses:
                p=mapr(obj,candidate)
                data=b[p:p+28] if p is not None else b""
                print("M363_ADDRESS name=%s object=%d model=%s candidateRva=%s physical=%s bytes=%s"%
                      (name,obj["number"],hypothesis,
                       "0x%x"%candidate if candidate>=0 else "negative",
                       "0x%x"%p if p is not None else "none",data.hex() if data else "none"))
        # Distinguish raw references in object1 pages from references to linear address.
        obj=objs[0]
        needle_variants=(("disp32",disp),("linearPreferred",obj["base"]+disp),
                         ("dispMinusBase",disp-obj["base"]))
        for label,value in needle_variants:
            if value<0 or value>0xffffffff:continue
            needle=struct.pack("<I",value)
            hits=[]
            for i in range(obj["count"]):
                start=page(obj["first"]+i)
                if start is None:continue
                chunk=b[start:start+min(ps,max(0,obj["size"]-i*ps))]
                at=chunk.find(needle)
                while at!=-1:
                    hits.append((i*ps+at,start+at))
                    at=chunk.find(needle,at+1)
            print("M363_REFERENCE_SCAN name=%s kind=%s value=0x%x count=%d samples=%s"%
                  (name,label,value,len(hits),
                   ",".join("rva=0x%x/physical=0x%x"%(r,p) for r,p in hits[:24]) or "none"))
        print("M363_DISPATCH_END name=%s"%name)
    print("M363_LIMITATIONS CSSelectorBaseNotStaticallyRecovered=1 rawDwordReferenceScanNotControlFlow=1 objectFlagsUninterpreted=1 fixupApplicationNotEmulated=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
