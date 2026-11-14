#!/usr/bin/env python3
"""STAFU M423: test contiguous 12-byte triples near Object-2 string references.

Read-only. Unlike M422, does not require NUL-delimited string starts.
Reports offset monotonicity, bounded text previews, field distributions,
and candidate boundaries. No decoder or runtime semantics asserted.
"""
import argparse, collections, hashlib, struct
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def preview(b,p,end):
 if p<0 or p>=end:return ("out_of_range","")
 q=b.find(b"\0",p,min(end,p+96))
 if q<0:return ("unterminated","")
 v=b[p:q]
 if not v:return ("empty","")
 if all(32<=x<127 for x in v):return ("printable",v.decode("ascii"))
 return ("binary",v[:32].hex())
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--window",type=lambda x:int(x,0),default=0x1200)
 args=ap.parse_args()
 b=args.ovl.read_bytes()
 print("M423_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M423_ERROR header mismatch")
 table=LE+u32(b,LE+0x40);count=u32(b,LE+0x44);page=u32(b,LE+0x28);data=LE+u32(b,LE+0x80)
 if count<2 or page!=4096:raise SystemExit("M423_ERROR object layout mismatch")
 size,base,flags,first,pages=struct.unpack_from("<IIIII",b,table+24)
 origin=data+(first-1)*page;end=min(len(b),origin+size)
 print("M423_OBJECT2 origin=0x%x end=0x%x base=0x%x assumedLinearPages=1"%(origin,end,base))
 anchors=[("bridge_primary",0x16486c),("bridge_secondary",0x164ed8),("door",0x164f14)]
 for label,anchor in anchors:
  print("M423_ANCHOR label=%s file=0x%x phase12=%d"%(label,anchor,anchor%12))
  lo=max(origin,anchor-args.window);hi=min(end,anchor+args.window)
  phase=anchor%12
  lo+= (phase-lo)%12
  records=[]
  for p in range(lo,hi-11,12):
   v,a,c=struct.unpack_from("<III",b,p)
   kind,s=preview(b,origin+v,end) if v<size else ("out_of_range","")
   records.append((p,v,a,c,kind,s))
  print("M423_WINDOW label=%s start=0x%x end=0x%x records=%d"%(label,lo,hi,len(records)))
  for key,index in (("a",2),("b",3),("kind",4)):
   cnt=collections.Counter(r[index] for r in records)
   print("M423_DISTRIBUTION label=%s field=%s distinct=%d top=%s"%(label,key,len(cnt),repr(cnt.most_common(16))))
  good=[r for r in records if r[4]=="printable"]
  print("M423_PRINTABLE label=%s count=%d percent=%.2f"%(label,len(good),100*len(good)/max(1,len(records))))
  # Runs of plausible offsets, regardless of string-start heuristic.
  runs=[];cur=[]
  for r in records:
   plausible=r[1]<size and r[4] in ("printable","empty")
   if plausible:
    if cur and r[1]<cur[-1][1]:
     runs.append(cur);cur=[]
    cur.append(r)
   elif cur:runs.append(cur);cur=[]
  if cur:runs.append(cur)
  runs.sort(key=len,reverse=True)
  print("M423_MONOTONIC_RUNS label=%s count=%d top=%s"%(
   label,len(runs),repr([(len(run),hex(run[0][0]),hex(run[-1][0]),hex(run[0][1]),hex(run[-1][1])) for run in runs[:12]])))
  # Explicit anchored context, and boundaries of the largest run.
  for r in records:
   if abs(r[0]-anchor)<=120:
    print("M423_NEAR label=%s file=0x%x offset=0x%x a=%d b=%d kind=%s text=%r"%(
     label,r[0],r[1],r[2],r[3],r[4],r[5][:75]))
  for run in runs[:2]:
   if len(run)<8:continue
   print("M423_RUN_DETAIL label=%s first=0x%x last=0x%x length=%d"%(label,run[0][0],run[-1][0],len(run)))
   for r in (run[:8]+run[-8:] if len(run)>16 else run):
    print("M423_RUN_ROW file=0x%x offset=0x%x a=%d b=%d kind=%s text=%r"%(r[0],r[1],r[2],r[3],r[4],r[5][:70]))
   for p in (run[0][0]-24,run[0][0]-12,run[-1][0]+12,run[-1][0]+24):
    if origin<=p and p+12<=end:
     v,a,c=struct.unpack_from("<III",b,p)
     kind,s=preview(b,origin+v,end) if v<size else ("out_of_range","")
     print("M423_BOUNDARY label=%s file=0x%x offset=0x%x a=%d b=%d kind=%s text=%r"%(label,p,v,a,c,kind,s[:70]))
 print("M423_LIMITATIONS objectMappingAssumed=1 textPlausibilityHeuristic=1 tableBoundaryUnverified=1 noConsumerTrace=1 noDecoderIdentified=1")
if __name__=="__main__":main()
