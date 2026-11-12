#!/usr/bin/env python3
"""M391: read-only DOS MZ segment and relocation mapping audit.

Distinguishes file-relative image offsets, real-mode segment arithmetic and
unobserved protected-mode selector interpretation. Does not execute the game.
"""
import argparse,hashlib,struct,collections
from pathlib import Path
LE=0x38194;TABLE=0x329f0;N=17
NAMES=("INIT","DEINIT","LOADER","UNLOAD","FREEMAP","REL","UNREL","CANLOAD","GETLOADTABLE","GETLOADNAME","GETERRSTR","LINEXE_LOADMODULE","LINEXE_GETPROCADDR","LINEXE_GETFLATREGS","LINEXE_FREEMODULE","LINEXE_GETMODHANDLE","XXXGET_OBJECT_MAP")
def w(b,p):return struct.unpack_from("<H",b,p)[0]
def d(b,p):return struct.unpack_from("<I",b,p)[0]
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);a=ap.parse_args()
 b=a.ovl.read_bytes();print(f"M391_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M391_ERROR expected MZ stub and LE image")
 last,pages,relocs,paras=w(b,2),w(b,4),w(b,6),w(b,8)
 image_size=(pages-1)*512+(last or 512);header=paras*16
 ss,sp,ip,cs=w(b,14),w(b,16),w(b,20),w(b,22)
 reloc_off=w(b,24)
 print(f"M391_MZ declaredBytes={image_size} headerBytes={header} loadImageBytes={image_size-header} relocations={relocs} relocTable=0x{reloc_off:x} cs=0x{cs:x} ip=0x{ip:x} ss=0x{ss:x} sp=0x{sp:x} entryFile=0x{header+cs*16+ip:x}")
 print(f"M391_MZ_BOUNDARY relocTableFitsHeader={int(reloc_off+relocs*4<=header)} declaredFitsFile={int(image_size<=len(b))} trailingBeforeLE={LE-image_size}")
 reloc_entries=[];invalid=[]
 for i in range(relocs):
  p=reloc_off+i*4
  if p+4>len(b):invalid.append(i);break
  off,seg=w(b,p),w(b,p+2)
  image_off=seg*16+off
  file_off=header+image_off
  if file_off+2>image_size:invalid.append(i)
  reloc_entries.append((i,seg,off,image_off,file_off))
 print(f"M391_RELOC_SUMMARY count={len(reloc_entries)} invalid={len(invalid)} uniqueSegments={len(set(x[1] for x in reloc_entries))} minImageOffset={min(x[3] for x in reloc_entries):#x} maxImageOffset={max(x[3] for x in reloc_entries):#x}")
 for i,seg,off,im,fo in reloc_entries[:40]:
  print(f"M391_RELOC index={i} segment=0x{seg:04x} offset=0x{off:04x} imageOffset=0x{im:x} file=0x{fo:x} originalWord=0x{w(b,fo):04x}")
 if len(reloc_entries)>40:print(f"M391_RELOC_REMAINING count={len(reloc_entries)-40} notPrinted=1")
 print("M391_MODEL realModeImageFile=headerBytes+16*relativeSegment+offset; loadedLinear=16*loadSegment+16*relativeSegment+offset")
 print("M391_MODEL protectedModeSelectorRequiresDescriptorBase=1 selectorValueAloneNotLinearAddress=1")
 # Segment 0x0088 as a *relative* segment in the MZ image.
 # Also test whether 0x0088 is a code segment selector whose descriptor
 # is unknown, and whether its low word could refer to another segment.
 for i,name in enumerate(NAMES):
  value=d(b,TABLE+i*8+4);seg=value>>16;off=value&0xffff
  as_relative=header+seg*16+off
  as_absolute=seg*16+off
  low_only=header+off
  print(f"M391_PTR index={i} name={name} value=0x{value:08x} segment=0x{seg:04x} offset=0x{off:04x} mzRelativeFile=0x{as_relative:x} mzRelativeInImage={int(header<=as_relative<image_size)} rawRealModeLinear=0x{as_absolute:x} lowWordPlusHeader=0x{low_only:x} lowWordInImage={int(header<=low_only<image_size)}")
 # The MZ load image includes offsets only through its declared boundary.
 # Compare code-looking bytes at both mapping hypotheses, without claiming
 # that the table actually holds far pointers.
 try:
  from capstone import Cs,CS_ARCH_X86,CS_MODE_16
  md=Cs(CS_ARCH_X86,CS_MODE_16)
  print("M391_DISASM capstone=available")
 except ImportError:
  md=None;print("M391_DISASM capstone=unavailable")
 for name in ("REL","UNREL","LINEXE_LOADMODULE","INIT","DEINIT"):
  i=NAMES.index(name);v=d(b,TABLE+i*8+4);seg=v>>16;off=v&65535
  for label,p in (("mzRelative",header+seg*16+off),("lowWordImage",header+off),("rawLowWord",off),("rawRealMode",seg*16+off)):
   valid=header<=p<image_size
   instructions=[]
   if md and valid:
    instructions=[f"{x.mnemonic} {x.op_str}" for x in md.disasm(b[p:min(p+32,image_size)],p,count=6)]
   print(f"M391_TARGET name={name} model={label} file=0x{p:x} inMZImage={int(valid)} bytes={b[p:p+16].hex() if valid else ''} instructions={' ; '.join(instructions)!r}")
 # Scan original relocation target words: their original contents are
 # link-time segment constants, not final runtime selectors.
 values=collections.Counter(w(b,fo) for _,_,_,_,fo in reloc_entries if fo+2<=image_size)
 for v,count in values.most_common(15):
  print(f"M391_RELOC_WORD value=0x{v:04x} count={count}")
 print(f"M391_RELOC_0088 originalRelocatedWords={values[0x88]} note=does_not_prove_runtime_selector")
 print("M391_LIMITATIONS loadSegmentUnknown=1 selectorDescriptorsUnknown=1 pointerTableSemanticsUnverified=1 noRuntimeExecution=1")
if __name__=="__main__":main()
