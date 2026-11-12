#!/usr/bin/env python3
"""M370: independently audit CS-relative jump operands and LE fixup tables.

Read-only. Compares displacement-as-offset and linear-address interpretations
against fixup source ranges, reconstructs entries and checks physical mapping.
No claim that a selector base is known at runtime.
"""
import argparse,hashlib,struct
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
LE=229780
SHA="65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697"
CASES=(("object_state",566541,0x12334,19,1),("global_state",570965,0x13488,20,0))
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();sha=hashlib.sha256(b).hexdigest()
 print("M370_INPUT bytes=%d sha256=%s"%(len(b),sha))
 if len(b)!=2008455 or sha!=SHA or b[LE:LE+2]!=b"LE":raise SystemExit("M370_ERROR unexpected OVL")
 ps=u32(b,LE+0x28);count=u32(b,LE+0x14)
 ot=LE+u32(b,LE+0x40);pm=LE+u32(b,LE+0x48)
 fi=LE+u32(b,LE+0x68);fr=LE+u32(b,LE+0x6c);origin=LE+u32(b,LE+0x80)
 objs=[]
 for i in range(u32(b,LE+0x44)):
  p=ot+i*24;objs.append((u32(b,p),u32(b,p+4),u32(b,p+8),u32(b,p+12),u32(b,p+16)))
 if ps!=4096 or count!=371 or len(objs)!=2:raise SystemExit("M370_ERROR unexpected layout")
 starts=[u32(b,fi+i*4) for i in range(count+1)]
 def locate(rva):
  for oi,(size,base,flags,first,pages) in enumerate(objs,1):
   if 0<=rva<size:
    logical=first+rva//ps
    if logical>=first+pages:continue
    e=b[pm+4*(logical-1):pm+4*logical];num=int.from_bytes(e[:3],"big")
    if not num or e[3]:raise ValueError("unsupported page map")
    return oi,logical,origin+(num-1)*ps+rva%ps
  return None
 def fixups(logical):
  blob=b[fr+starts[logical-1]:fr+starts[logical]];pos=0;out={}
  while pos<len(blob):
   rec=pos
   if pos+7>len(blob):raise ValueError("truncated fixup")
   src,flags=blob[pos:pos+2];pos+=2
   if src!=7 or flags not in (0,0x10):raise ValueError("unsupported record at %d: %02x %02x"%(rec,src,flags))
   off=u16(blob,pos);pos+=2;obj=blob[pos];pos+=1
   width=4 if flags&0x10 else 2
   if pos+width>len(blob):raise ValueError("truncated target")
   target=u32(blob,pos) if width==4 else u16(blob,pos);pos+=width
   if off in out:raise ValueError("duplicate source")
   out[off]=(rec,obj,target,flags)
  return out,len(blob)
 cs=Cs(CS_ARCH_X86,CS_MODE_32)
 for name,physical,disp,logical,state0 in CASES:
  # Locate dispatcher using physical page address, not guessed RVA.
  physical_page=origin+((physical-origin)//ps)*ps
  found=[]
  for oi,(size,base,flags,first,pages) in enumerate(objs,1):
   for l in range(first,first+pages):
    e=b[pm+4*(l-1):pm+4*l];num=int.from_bytes(e[:3],"big")
    if origin+(num-1)*ps<=physical<origin+num*ps:
     rva=(l-first)*ps+(physical-(origin+(num-1)*ps))
     found.append((oi,l,rva))
  if len(found)!=1:raise SystemExit("M370_ERROR dispatcher physical mapping")
  oi,dl,drva=found[0]
  code=b[physical:physical+12]
  instructions=list(cs.disasm(code,drva,count=2))
  print("M370_DISPATCH name=%s physical=0x%x object=%d logical=%d rva=0x%x bytes=%s"%(name,physical,oi,dl,drva,code.hex()))
  for ins in instructions:
   print("M370_INSN name=%s rva=0x%x bytes=%s mnemonic=%s operands=%s"%(name,ins.address,ins.bytes.hex(),ins.mnemonic,ins.op_str.replace(" ","_")))
  # CS prefix + FF 24 85 + 32-bit displacement, verify exact.
  expected=b"\x2e\xff\x24\x85"+struct.pack("<I",disp)
  matches=b[physical:physical+8]==expected
  print("M370_ENCODING name=%s expected=%s actual=%s exact=%d"%(name,expected.hex(),b[physical:physical+8].hex(),int(matches)))
  records,nbytes=fixups(logical)
  print("M370_FIXUP_PAGE name=%s logical=%d records=%d bytes=%d"%(name,logical,len(records),nbytes))
  for hypothesis,linear in (("CS_base_object1",objs[0][1]+disp),("CS_base_zero",disp),("displacement_is_linear",disp)):
   rva=linear-objs[0][1]
   mapped=locate(rva)
   source_count=0
   if mapped:
    _,l,_=mapped
    f,_=fixups(l)
    off=rva%ps
    source_count=sum(off+4*i in f for i in range(7))
   print("M370_ADDRESS name=%s hypothesis=%s linear=0x%x object1Rva=0x%x mapped=%s tableFixupMatches=%d"%
    (name,hypothesis,linear,rva,("obj%d:logical%d:physical0x%x"%mapped) if mapped else "none",source_count))
  off=disp%ps
  values=[]
  for i in range(7):
   src=off+4*i
   if src not in records:raise SystemExit("M370_ERROR missing expected fixup")
   rec,obj,target,flags=records[src]
   linear=objs[obj-1][1]+target
   targetmap=locate(target) if obj==1 else None
   values.append(linear)
   print("M370_ENTRY name=%s state=%d source=0x%x record=%d flags=0x%x targetObject=%d targetRva=0x%x preferredLinear=0x%x targetMapping=%s"%
    (name,state0+i,src,rec,flags,obj,target,linear,("obj%d:logical%d:physical0x%x"%targetmap) if targetmap else "none"))
  print("M370_TABLE name=%s entries=%d uniqueTargets=%d exactJumpEncoding=%d"%(name,len(values),len(set(values)),int(matches)))
 print("M370_LIMITATIONS CSSelectorBaseUnknown=1 runtimeAddressingUnverified=1 LEFixupSemanticsAssumed=1 noReachabilityProof=1 SPRDecoderUnverified=1")
if __name__=="__main__":main()
