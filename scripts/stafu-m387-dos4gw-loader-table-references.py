#!/usr/bin/env python3
"""STAFU M387: inspect DOS/4GW loader-name tables and candidate references.

Read-only; does not execute copyrighted binaries. File-offset pointers are
hypotheses until validated against the loader's actual address space.
"""
import argparse, hashlib, struct
from pathlib import Path

MARKERS = [b"LINEXE_LOADER", b"LINEXE_GETFLATREGS", b"DOS4GMAINLOADERVECT",
           b"XXXGET_OBJECT_MAP", b"SEGEXE_LOADER", b"EXP_LOADER"]
def u32(data, at):
    return struct.unpack_from("<I", data, at)[0]
def printable(data):
    return "".join(chr(x) if 32 <= x < 127 else "." for x in data)
def dump(data, start, end, label):
    start=max(0,start); end=min(len(data),end)
    print(f"M387_REGION label={label} start=0x{start:x} end=0x{end:x}")
    for p in range(start,end,16):
        chunk=data[p:min(p+16,end)]
        print(f"M387_HEX offset=0x{p:x} hex={chunk.hex()} ascii={printable(chunk)!r}")
def string_at(data, p):
    if not (0<=p<len(data)): return None
    end=data.find(b"\0",p,min(len(data),p+96))
    if end<0 or end==p:return None
    chunk=data[p:end]
    if all(32<=x<127 for x in chunk):return chunk.decode("ascii")
    return None
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    a=ap.parse_args()
    b=a.ovl.read_bytes()
    sha=hashlib.sha256(b).hexdigest()
    print(f"M387_INPUT bytes={len(b)} sha256={sha}")
    if len(b)<0x38200 or b[0x38194:0x38196]!=b"LE":
        raise SystemExit("M387_ERROR expected LE header at 0x38194")
    # Scan only embedded extender prefix, not the LE payload.
    prefix=b[:0x38194]
    for name in MARKERS:
        pos=0; matches=[]
        while True:
            k=prefix.find(name,pos)
            if k<0:break
            matches.append(k);pos=k+1
        print(f"M387_MARKER name={name.decode()} count={len(matches)} offsets={','.join(hex(x) for x in matches)}")
        for k in matches[:8]:
            # Search for exact little-endian file-offset constants referencing
            # strings, and candidate relocatable offsets relative to two
            # observed loader string/table regions. No base is assumed valid.
            for base in (0,0x1da00,0x31c00,0x32900):
                value=k-base
                if value<0:continue
                pattern=struct.pack("<I",value)
                refs=[];p=0
                while True:
                    x=prefix.find(pattern,p)
                    if x<0:break
                    refs.append(x);p=x+1
                    if len(refs)>=24:break
                if refs:
                    print(f"M387_POINTER_CANDIDATE marker=0x{k:x} base=0x{base:x} encoded=0x{value:x} matches={','.join(hex(x) for x in refs)} meaning=unverified")
    # The table after the LINEXE_LOADER string includes paired 32-bit
    # values. Inspect as pairs without claiming address semantics.
    for start,end,label in ((0x329e2,0x32b42,"linexe-name-and-pairs"),
                            (0x31d88,0x31e58,"loader-name-table"),
                            (0x33880,0x33990,"loader-module-names"),
                            (0x317d0,0x318a0,"loader-vector-names")):
        dump(prefix,start,end,label)
    for start,end,label in ((0x329f0,0x32af0,"linexe-pairs"),
                            (0x31de8,0x31e48,"loader-pairs"),
                            (0x338f0,0x33970,"module-pairs")):
        print(f"M387_DWORDS label={label} start=0x{start:x}")
        for p in range(start,min(end,len(prefix)-7),8):
            x,y=u32(prefix,p),u32(prefix,p+4)
            # Values near 0x800000 are common in this region, but are
            # not automatically physical or linear addresses.
            print(f"M387_PAIR offset=0x{p:x} first=0x{x:08x} second=0x{y:08x} firstFileCandidate={int(x<len(b))} secondFileCandidate={int(y<len(b))}")
    # Explicitly inspect potential string-pointer references in the
    # loader prefix without using a guessed relocation base.
    for k in (0x329e2,0x31da8,0x317e1,0x338cb):
        for delta in (0,0x800000,0x880000,0xa80000):
            target=k+delta
            if target>0xffffffff:continue
            pattern=struct.pack("<I",target)
            found=[];p=0
            while len(found)<12:
                x=prefix.find(pattern,p)
                if x<0:break
                found.append(x);p=x+1
            print(f"M387_ABSOLUTE_REFERENCE marker=0x{k:x} encoding=0x{target:x} countFirst12={len(found)} offsets={','.join(hex(x) for x in found)}")
    print("M387_LIMITATIONS tableLayoutUnverified=1 pointerBaseUnverified=1 executableReferencesUnverified=1 runtimeLoaderNotObserved=1")
if __name__=="__main__":
    main()
