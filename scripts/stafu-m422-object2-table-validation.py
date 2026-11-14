#!/usr/bin/env python3
"""STAFU M422: validate candidate 12-byte Object-2 string-reference table.

Read-only; verifies full-string starts, compares all four 12-byte alignments,
measures contiguous record runs, and reports field distributions. No runtime
or decoder semantics are inferred.
"""
import argparse,collections,hashlib,struct
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def get_string(b,p,end):
 if p<0 or p>=end:return None
 q=b.find(b"\0",p,min(end,p+128))
 if q<0 or q==p:return None
 raw=b[p:q]
 if not all(32<=v<127 for v in raw):return None
 return raw.decode("ascii","replace")
def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument("ovl",type=Path)
 ap.add_argument("--scan-start",type=lambda x:int(x,0),default=0x160000)
 ap.add_argument("--scan-end",type=lambda x:int(x,0),default=0x170000)
 args=ap.parse_args()
 b=args.ovl.read_bytes()
 print("M422_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M422_ERROR unexpected executable header")
 objtab=LE+u32(b,LE+0x40);count=u32(b,LE+0x44);page=u32(b,LE+0x28);data=LE+u32(b,LE+0x80)
 if count<2 or page!=4096:raise SystemExit("M422_ERROR unexpected object/page layout")
 size,base,flags,first,pages=struct.unpack_from("<IIIII",b,objtab+24)
 origin=data+(first-1)*page;end=min(len(b),origin+size)
 lo=max(origin,args.scan_start);hi=min(end,args.scan_end)
 print("M422_OBJECT2 origin=0x%x end=0x%x base=0x%x size=0x%x pageMapAssumedLinear=1"%(origin,end,base,size))
 print("M422_SCAN start=0x%x end=0x%x bytes=%d"%(lo,hi,hi-lo))
 # Enumerate true starts using a NUL delimiter, not arbitrary suffixes.
 starts={}
 for p in range(origin+1,min(end,0x160000)):
  if b[p-1]!=0:continue
  s=get_string(b,p,end)
  if s and len(s)>=3:starts[p-origin]=s
 print("M422_STRING_STARTS count=%d range=[0x%x,0x%x)"%(len(starts),origin,min(end,0x160000)))
 def classify(v):
  if v in starts:return "full"
  p=origin+v
  s=get_string(b,p,end) if 0<=v<size else None
  if s:return "suffix"
  return "none"
 # Only dword-aligned slots; 12-byte stride has three possible phases.
 rows={}
 for p in range((lo+3)//4*4,hi-11,4):
  v,a,c=struct.unpack_from("<III",b,p)
  cls=classify(v)
  rows[p]=(v,a,c,cls,starts.get(v))
 for phase in range(0,12,4):
  sample=[(p,*r) for p,r in rows.items() if p%12==phase]
  kinds=collections.Counter(r[4] for r in sample)
  full=[r for r in sample if r[4]=="full"]
  fields_a=collections.Counter(r[2] for r in full)
  fields_b=collections.Counter(r[3] for r in full)
  print("M422_PHASE phase=%d total=%d full=%d suffix=%d none=%d fullPercent=%.3f"%(
   phase,len(sample),kinds["full"],kinds["suffix"],kinds["none"],100*kinds["full"]/max(1,len(sample))))
  print("M422_FIELD_A phase=%d distinct=%d top=%s"%(phase,len(fields_a),repr(fields_a.most_common(20))))
  print("M422_FIELD_B phase=%d distinct=%d top=%s"%(phase,len(fields_b),repr(fields_b.most_common(20))))
  # Longest contiguous full-string runs, including location and endpoints.
  runs=[];active=[]
  for p,v,a,c,cls,name in sample:
   if cls=="full":
    if active and p!=active[-1][0]+12:runs.append(active);active=[]
    active.append((p,v,a,c,name))
   elif active:runs.append(active);active=[]
  if active:runs.append(active)
  runs.sort(key=len,reverse=True)
  print("M422_RUNS phase=%d count=%d longest=%s"%(
   phase,len(runs),repr([(len(r),hex(r[0][0]),hex(r[-1][0]),r[0][4],r[-1][4]) for r in runs[:15]])))
  for run in runs[:3]:
   if len(run)<5:continue
   print("M422_RUN_DETAIL phase=%d start=0x%x length=%d"%(phase,run[0][0],len(run)))
   for p,v,a,c,name in run[:15]:
    print("M422_RECORD file=0x%x offset=0x%x a=%d b=%d name=%r"%(p,v,a,c,name[:90]))
 # Independently anchor exact M421 records; compare record fields.
 for p,expected in [(0x16486c,"bridge.rm"),(0x164ed8,"bridge.rm"),(0x164f14,"brdgdoor.spr")]:
  v,a,c=struct.unpack_from("<III",b,p)
  name=starts.get(v)
  print("M422_ANCHOR file=0x%x offset=0x%x a=%d b=%d name=%r expected=%r exactStart=%d match=%d"%(
   p,v,a,c,name,expected,int(v in starts),int(name==expected)))
 print("M422_LIMITATIONS stringStartHeuristic=1 scanRangeLimited=1 pageMapAssumedLinear=1 recordBoundariesUnproven=1 noCodeConsumer=1 noDecoderIdentified=1")
if __name__=="__main__":main()
