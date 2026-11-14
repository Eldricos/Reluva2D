#!/usr/bin/env python3
"""STAFU M411: interleaved argument pushes and segment setup around loader calls.

Read-only, standard library. Reuses M409's partial decoder. No runtime or
reachability claims. Candidate instruction boundaries remain hypotheses.
"""
import argparse
import hashlib
import importlib.util
from pathlib import Path

TARGETS = (
    ("DVX_Global_Seg_Alloc",0x28A70,8),
    ("DVX_Global_Seg_Alloc",0x2CBC7,8),
    ("DVX_Global_Seg_Broadcast",0x2CC8B,12),
    ("DVX_Global_Seg_Broadcast",0x2CD19,12),
    ("VMFILEIO",0x2CE6D,12),
)
PREFIXES={0x26:"es",0x2e:"cs",0x36:"ss",0x3e:"ds"}
LIMIT=0x38194

def load_decoder():
    path=Path(__file__).with_name("stafu-m409-corrected-indirect-call-decoder.py")
    if not path.is_file():
        raise SystemExit("M411_ERROR missing sibling script: "+str(path))
    spec=importlib.util.spec_from_file_location("stafu_m409_decoder",path)
    mod=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod.decode

def candidate_sequences(data,decode,call_start,lookback=96):
    results=[]
    for start in range(max(0,call_start-lookback),call_start):
        cursor=start; seq=[]
        while cursor<call_start:
            decoded=decode(data,cursor)
            if not decoded or decoded[0]<=0 or cursor+decoded[0]>call_start:
                break
            length,ins,kind,barrier=decoded
            seq.append((cursor,length,ins,kind,barrier))
            cursor+=length
        if cursor==call_start and seq:
            results.append((start,seq))
    return results

def stack_effect(ins,kind):
    if kind=="push":return 2
    if kind=="pop":return -2
    if kind=="cleanup" and ins.startswith("add sp,"):
        try:return -int(ins.split(",",1)[1],0)
        except ValueError:return None
    if ins.startswith("sub sp,"):
        try:return int(ins.split(",",1)[1],0)
        except ValueError:return None
    if ins in ("mov sp,bp","leave","ret","retf") or ins.startswith(("ret ","retf ")):
        return None
    if ins.startswith(("call ","call far","call near")):
        return 0  # net stack effect unknown: call may use callee cleanup
    if ins.startswith(("jmp ","branch ")):
        return 0  # only a linear accounting hypothesis
    if ins.startswith(("mov sp,","lea sp,","xchg sp,")):
        return None
    return 0

def summarize(seq,expected):
    # Scan backwards to last stack reset or control-flow barrier; then compute
    # candidate outstanding pushes while permitting intervening register setup.
    suffix=[]
    for row in reversed(seq):
        addr,length,ins,kind,barrier=row
        effect=stack_effect(ins,kind)
        if effect is None or barrier:break
        suffix.append((row,effect))
    suffix.reverse()
    net=sum(effect for _,effect in suffix)
    pushes=[r for r,e in suffix if e==2 and r[3]=="push"]
    pops=[r for r,e in suffix if e==-2]
    nonpush=[r for r,e in suffix if e==0 and r[3]!="push"]
    segments=[r for r in seq if r[3]=="segment" or "es:" in r[2] or "ds:" in r[2] or "ss:" in r[2]]
    branches=[r for r in seq if r[3]=="flow"]
    return {
        "net":net,"pushes":pushes,"pops":pops,"interleaved":nonpush,
        "segments":segments,"branches":branches,
        "match":net==expected,
        "last_push":pushes[-1][0] if pushes else -1,
    }

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ovl",type=Path)
    args=ap.parse_args()
    data=args.ovl.read_bytes()
    print("M411_INPUT bytes=%d sha256=%s"%(len(data),hashlib.sha256(data).hexdigest()))
    if len(data)<LIMIT+2 or data[:2]!=b"MZ" or data[LIMIT:LIMIT+2]!=b"LE":
        raise SystemExit("M411_ERROR unexpected MZ/LE layout")
    decode=load_decoder()
    for name,opcode,cleanup in TARGETS:
        start=opcode
        prefixes=[]
        while start>0 and data[start-1] in PREFIXES and len(prefixes)<4:
            start-=1
            prefixes.insert(0,data[start])
        raw=data[start:opcode+4]
        print("M411_TARGET name=%s opcode=0x%x instructionStart=0x%x prefix=%s expectedCleanup=%d bytes=%s"%(
            name,opcode,start,",".join(PREFIXES[x] for x in prefixes) or "none",cleanup,raw.hex()))
        paths=candidate_sequences(data,decode,start)
        scored=[]
        for anchor,seq in paths:
            s=summarize(seq,cleanup)
            scored.append((anchor,seq,s))
        matches=[x for x in scored if x[2]["match"]]
        # Rank paths by matching stack balance, more pushes, fewer branches,
        # shorter arbitrary prefix. None of these proves actual code boundaries.
        ranked=sorted(scored,key=lambda x:(
            not x[2]["match"],
            -len(x[2]["pushes"]),
            len(x[2]["branches"]),
            -x[0]
        ))
        print("M411_PATHS opcode=0x%x candidates=%d matchingNet=%d"%(opcode,len(scored),len(matches)))
        if not ranked:
            print("M411_NO_PATH opcode=0x%x preceding=%s"%(opcode,data[max(0,start-64):start].hex()))
            continue
        for rank,(anchor,seq,s) in enumerate(ranked[:3],1):
            print("M411_CANDIDATE opcode=0x%x rank=%d anchor=0x%x instructions=%d netStack=%d expected=%d matches=%d pushes=%d pops=%d intervening=%d branches=%d"%(
                opcode,rank,anchor,len(seq),s["net"],cleanup,int(s["match"]),
                len(s["pushes"]),len(s["pops"]),len(s["interleaved"]),len(s["branches"])))
            for addr,length,ins,kind,barrier in seq[-26:]:
                print("M411_INSN opcode=0x%x rank=%d file=0x%x len=%d kind=%s barrier=%d text=%s"%(
                    opcode,rank,addr,length,kind,int(barrier),ins))
            print("M411_SEGMENTS opcode=0x%x rank=%d observations=%s"%(
                opcode,rank,";".join("0x%x:%s"%(r[0],r[2]) for r in s["segments"]) or "none"))
        print("M411_AFTER opcode=0x%x bytes=%s"%(opcode,data[opcode+4:opcode+16].hex()))
    print("M411_LIMITATIONS partialM409Decoder=1 arbitraryAnchors=1 branchesNotFollowed=1 callCalleeCleanupUnknown=1 runtimeSegmentsUnknown=1 stackDeltaNotSignature=1")
if __name__=="__main__":
    main()
