#!/usr/bin/env python3
"""STAFU M420: locate SPR-name references and nearby LE call candidates.

Read-only, conservative static reconnaissance. No decoder semantics asserted.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=0x38194
DATA=0x7af94
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def s32(v):return v-0x100000000 if v&0x80000000 else v
def strings(b,start,end):
 out=[]
 i=start
 while i<end:
  if 32<=b[i]<127:
   j=i
   while j<end and 32<=b[j]<127:j+=1
   if j-i>=5 and (j==end or b[j]==0):out.append((i,b[i:j].decode("ascii","replace")))
   i=max(j+1,i+1)
  else:i+=1
 return out
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument("ovl",type=Path)
 p.add_argument("--window",type=int,default=96,help="context radius around candidate references")
 args=p.parse_args();b=args.ovl.read_bytes()
 print("M420_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M420_ERROR unexpected OVL layout")
 # LE object records are 24 bytes each; verify offsets from the file itself.
 objtab=LE+u32(b,LE+0x40);nobj=u32(b,LE+0x44)
 pagemap=LE+u32(b,LE+0x48);pagesize=u32(b,LE+0x28)
 dataoff=LE+u32(b,LE+0x80)
 print("M420_LE objectTable=0x%x objectCount=%d pageMap=0x%x pageSize=%d dataOffsetCandidate=0x%x"%(objtab,nobj,pagemap,pagesize,dataoff))
 if not 1<=nobj<=32:raise SystemExit("M420_ERROR implausible object count")
 objs=[]
 for i in range(nobj):
  q=objtab+24*i;size,base,flags,pageidx,pagecount=struct.unpack_from("<IIIII",b,q)
  objs.append((size,base,flags,pageidx,pagecount))
  print("M420_OBJECT index=%d size=0x%x base=0x%x flags=0x%x firstPage=%d pageCount=%d"%(i+1,size,base,flags,pageidx,pagecount))
 # This project has observed an uncompressed LE page sequence at 0x7AF94.
 # Check page map before relying on the linear mapping.
 print("M420_PAGE_MAP_FIRST bytes=%s"%b[pagemap:pagemap+48].hex())
 print("M420_MAPPING assumption=linearLEPages candidateDataStart=0x%x verified=0"%DATA)
 # Scan readable embedded resource names across the OVL, not user assets.
 names=[(off,s) for off,s in strings(b,0,len(b)) if ".SPR" in s.upper() or s.upper().endswith(".RM") or ".IMG" in s.upper()]
 print("M420_RESOURCE_STRINGS count=%d"%len(names))
 for off,s in names[:150]:
  print("M420_NAME file=0x%x text=%r"%(off,s[:120]))
 if len(names)>150:print("M420_NAME_TRUNCATED omitted=%d"%(len(names)-150))
 # For LE object data, scan 32-bit immediates equal to resource string
 # object-relative or linear addresses. Matches are candidates only.
 for index,(size,base,flags,pageidx,pagecount) in enumerate(objs,1):
  if pageidx<1 or not pagecount:continue
  start=DATA+(pageidx-1)*4096
  end=min(len(b),start+size)
  print("M420_OBJECT_SCAN index=%d fileStart=0x%x fileEnd=0x%x"%(index,start,end))
  if start>=end:continue
  # direct near E8 calls are instruction-boundary unverified; list only
  # calls near literal name references rather than the whole image.
  for noff,name in names[:150]:
   if not(start<=noff<end):continue
   rel=noff-start
   addresses=[base+rel,rel]
   for val in dict.fromkeys(addresses):
    needle=struct.pack("<I",val)
    found=[];pos=start
    while len(found)<30:
     pos=b.find(needle,pos,end)
     if pos<0:break
     found.append(pos);pos+=1
    for at in found:
     nearstart=max(start,at-args.window);nearend=min(end,at+args.window)
     calls=[]
     for cp in range(nearstart,nearend-4):
      if b[cp]==0xe8:
       disp=s32(u32(b,cp+1));target=cp+5+disp
       if start<=target<end:calls.append((cp,target))
     print("M420_XREF_CANDIDATE nameFile=0x%x name=%r object=%d pointer=0x%x operandFile=0x%x context=%s nearbyCalls=%s boundariesVerified=0"%(
      noff,name[:48],index,val,at,b[max(start,at-16):min(end,at+20)].hex(),
      ",".join("0x%x:0x%x"%(c,t) for c,t in calls[:12]) or "none"))
 # Search strings inside DOS4GW prefix separately; do not treat as LE code.
 prefix=[(p,s) for p,s in names if p<LE]
 print("M420_DOS_PREFIX_NAMES count=%d"%len(prefix))
 print("M420_LIMITATIONS noGameAssets=1 noRuntimeTrace=1 pageMappingAssumed=1 rawImmediateXrefs=1 instructionBoundariesUnverified=1 decoderNotIdentified=1")
if __name__=="__main__":main()
