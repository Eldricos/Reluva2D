#!/usr/bin/env python3
"""STAFU M421: inspect candidate resource-name tables in LE object 2.

Read-only. Validates relative pointer-to-string relationships and explores
record stride/alignment; does not claim to identify loader or decoder code.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=0x38194
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def txt(b,p):
 if p<0 or p>=len(b):return None
 q=b.find(b"\0",p,min(len(b),p+100))
 if q<0 or q==p:return None
 s=b[p:q]
 return s.decode("ascii") if all(32<=x<127 for x in s) else None
def main():
 a=argparse.ArgumentParser(description=__doc__);a.add_argument("ovl",type=Path)
 b=a.parse_args().ovl.read_bytes()
 print("M421_INPUT bytes=%d sha256=%s"%(len(b),hashlib.sha256(b).hexdigest()))
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M421_ERROR unexpected LE/MZ layout")
 obj=LE+u32(b,LE+0x40);n=u32(b,LE+0x44);page= u32(b,LE+0x28)
 if n<2 or page!=4096:raise SystemExit("M421_ERROR unexpected object count/page size")
 o1=struct.unpack_from("<IIIII",b,obj)
 o2=struct.unpack_from("<IIIII",b,obj+24)
 data=LE+u32(b,LE+0x80)
 start=data+(o2[3]-1)*page
 end=min(len(b),start+o2[0])
 print("M421_MAPPING object2FileStart=0x%x object2FileEnd=0x%x base=0x%x size=0x%x firstPage=%d data=0x%x assumption=linearPageMapping"%(
 start,end,o2[1],o2[0],o2[3],data))
 # Known literal name anchors from M420; check and derive expected offsets.
 anchors=[(0x15799f,"bridge.rm"),(0x157ce2,"bridge.rm"),(0x157d01,"brdgdoor.spr"),
 (0x157d23,"brdgldor.spr"),(0x157de8,"brdgrikr.spr"),(0x157df5,"brdgpica.spr"),
 (0x157e02,"brdgtroi.spr"),(0x157e0f,"brdgdata.spr")]
 for f,s in anchors:
  actual=txt(b,f)
  print("M421_NAME file=0x%x relative=0x%x linear=0x%x expected=%r actual=%r matched=%d"%(
   f,f-start,o2[1]+f-start,s,actual,int(actual==s)))
 # Probe the table in 4-byte words, printing all 12-byte record hypotheses
 # as well as independent validation of string pointers.
 regions=[("bridge_primary",0x16486c),("bridge_secondary",0x164ed8),("door",0x164f14)]
 for label,center in regions:
  lo=max(start,center-96);hi=min(end,center+160)
  print("M421_REGION label=%s center=0x%x start=0x%x end=0x%x raw=%s"%(label,center,lo,hi,b[lo:hi].hex()))
  for p in range((lo+3)//4*4,hi-3,4):
   val=u32(b,p)
   target=start+val
   name=txt(b,target) if 0<=val<o2[0] else None
   isresource=bool(name and (".spr" in name.lower() or ".rm" in name.lower() or ".img" in name.lower()))
   if isresource:
    print("M421_POINTER label=%s at=0x%x value=0x%x target=0x%x name=%r before=%s after=%s"%(
     label,p,val,target,name,b[max(start,p-16):p].hex(),b[p+4:min(end,p+20)].hex()))
  for stride in (8,12,16,20,24):
   for shift in range(0,stride,4):
    aligned=center-shift
    if aligned<lo or aligned+stride*3>hi:continue
    rows=[]
    for k in range(-2,4):
     p=aligned+k*stride
     if p<lo or p+stride>hi:continue
     words=[u32(b,p+4*i) for i in range(stride//4)]
     matches=[(i,txt(b,start+v)) for i,v in enumerate(words) if v<o2[0] and txt(b,start+v) and any(x in txt(b,start+v).lower() for x in (".spr",".rm",".img"))]
     rows.append((p,words,matches))
    hits=sum(bool(x[2]) for x in rows)
    if hits>=2:
     print("M421_STRIDE_CANDIDATE label=%s stride=%d shift=%d rowsWithResource=%d rows=%s"%(
      label,stride,shift,hits,repr(rows)))
 # Find all 4-byte pointer occurrences to named SPR/RM strings in object 2.
 names=[]
 p=start
 while p<end:
  s=txt(b,p)
  if s and (s.lower().endswith(".spr") or s.lower().endswith(".rm")):
   names.append((p,s))
   p+=len(s)+1
  else:p+=1
 print("M421_NAMES_SCANNED count=%d"%len(names))
 name_by_offset={p-start:s for p,s in names}
 refs=[]
 # scan aligned 4-byte cells in candidate data band, not the full object
 lo=max(start,0x163000);hi=min(end,0x167000)
 for p in range((lo+3)//4*4,hi-3,4):
  val=u32(b,p)
  if val in name_by_offset:refs.append((p,val,name_by_offset[val]))
 print("M421_TABLE_REFS scanStart=0x%x scanEnd=0x%x count=%d"%(lo,hi,len(refs)))
 for p,v,s in refs[:150]:
  print("M421_REF at=0x%x offset=0x%x name=%r prevWords=%s nextWords=%s"%(
   p,v,s,[hex(u32(b,q)) for q in range(p-12,p,4) if q>=start],
   [hex(u32(b,q)) for q in range(p+4,p+16,4) if q+4<=end]))
 if len(refs)>150:print("M421_REF_TRUNCATED count=%d"%(len(refs)-150))
 for stride in (8,12,16,20,24,28,32):
  mods=collections.Counter(p%stride for p,_,_ in refs)
  print("M421_ALIGNMENT stride=%d distribution=%s"%(stride,repr(sorted(mods.items()))))
 print("M421_LIMITATIONS pageMapLinearAssumption=1 recordStrideUnverified=1 codePointersUnverified=1 noRuntimeExecution=1 noDecoderIdentified=1")
if __name__=="__main__":main()
