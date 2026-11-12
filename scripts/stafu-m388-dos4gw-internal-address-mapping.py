#!/usr/bin/env python3
"""M388: read-only analysis of DOS/4GW internal name/function address pairs.

Tests affine address-to-file mappings against actual printable strings and
possible code bytes. Candidate mappings are not assumed to be runtime truth.
"""
import argparse, collections, hashlib, struct
from pathlib import Path

TABLE=0x329f0
COUNT=17
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def ascii_z(b,p,limit=96):
    if not 0<=p<len(b):return None
    q=b.find(b"\0",p,min(len(b),p+limit))
    if q<0 or q==p:return None
    t=b[p:q]
    return t.decode("ascii") if all(32<=v<127 for v in t) else None
def runs(b,lo,hi,minlen=4):
    result=[]
    p=lo
    while p<hi:
        if 32<=b[p]<127:
            q=p
            while q<hi and 32<=b[q]<127:q+=1
            if q-p>=minlen and q<hi and b[q]==0:
                result.append((p,b[p:q].decode("ascii")))
            p=q+1
        else:p+=1
    return result
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    b=args.ovl.read_bytes()
    print(f"M388_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
    if b[LE:LE+2]!=b"LE":raise SystemExit("M388_ERROR expected LE header")
    pairs=[(u32(b,TABLE+8*i),u32(b,TABLE+8*i+4)) for i in range(COUNT)]
    for i,(a,c) in enumerate(pairs):
        print(f"M388_ENTRY index={i} file=0x{TABLE+8*i:x} nameValue=0x{a:08x} codeValue=0x{c:08x}")
    names=runs(b,0,LE)
    print(f"M388_CANDIDATE_STRINGS count={len(names)}")
    # For each proposed string address, search all known NUL-terminated
    # loader strings for a shared additive mapping: file = address - base.
    candidates=collections.defaultdict(set)
    for i,(a,_) in enumerate(pairs):
        for p,_ in names:
            candidates[a-p].add(i)
    ranked=[]
    for base in candidates:
        mapped=[]
        for i,(a,_) in enumerate(pairs):
            p=a-base
            s=ascii_z(b,p) if 0<=p<LE else None
            if s:mapped.append((i,p,s))
        if len(mapped)>=2:ranked.append((len(mapped),base,mapped))
    ranked.sort(key=lambda x:(-x[0],abs(x[1])))
    for score,base,mapped in ranked[:12]:
        print(f"M388_NAME_MAPPING base=0x{base:x} validStrings={score}/{COUNT} distinctNames={len(set(x[2] for x in mapped))}")
        for i,p,s in mapped:
            print(f"M388_NAME_MATCH base=0x{base:x} index={i} file=0x{p:x} text={s!r}")
    if not ranked:print("M388_NAME_MAPPING noneWithTwoValidStrings=1")
    # Inspect code-address mapping hypotheses, especially the base inferred
    # from the names. Code bytes alone cannot verify an instruction boundary.
    bases=[0,0x800000,0x880000,0xa00000,0xa80000]
    bases.extend(base for _,base,_ in ranked[:5])
    seen=set()
    for base in bases:
        if base in seen:continue
        seen.add(base)
        valid=[(i,c-base) for i,(_,c) in enumerate(pairs) if 0<=c-base<LE]
        print(f"M388_CODE_MAPPING base=0x{base:x} mappedLoaderPrefix={len(valid)}/{COUNT}")
        for i,p in valid[:17]:
            print(f"M388_CODE_BYTES base=0x{base:x} index={i} file=0x{p:x} bytes={b[p:p+16].hex()} asciiString={ascii_z(b,p)!r}")
    # Independent consistency: second column clustering and differences.
    for label,col in (("name",[x[0] for x in pairs]),("code",[x[1] for x in pairs])):
        high=collections.Counter(v>>16 for v in col)
        print(f"M388_CLUSTER column={label} upper16={','.join(f'0x{k:04x}:{v}' for k,v in sorted(high.items()))}")
        print(f"M388_DELTAS column={label} values={','.join(str(col[i+1]-col[i]) for i in range(len(col)-1))}")
    # Detect repeated 8-byte pair signatures in the embedded loader prefix.
    for i in (0,1,8,16):
        pat=b[TABLE+8*i:TABLE+8*i+8]
        hits=[];p=0
        while True:
            x=b.find(pat,p,LE)
            if x<0:break
            hits.append(x);p=x+1
        print(f"M388_PAIR_REPEAT index={i} count={len(hits)} offsets={','.join(hex(x) for x in hits[:20])}")
    print("M388_LIMITATIONS affineMappingHypothesisOnly=1 stringsCanBeCoincidental=1 codeBoundariesNotDisassembled=1 runtimeLoaderNotObserved=1")
if __name__=="__main__":main()
