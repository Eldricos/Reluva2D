#!/usr/bin/env python3
"""STAFU M393: read-only loader module descriptor and adjacent record audit.

Inspect the bytes following the verified 17-pair LINEXE_LOADER export table,
compare neighboring loader names, and test structural hypotheses without
assigning unverified runtime meanings.
"""
import argparse,hashlib,struct
from pathlib import Path
TABLE=0x329f0;COUNT=17;LE=0x38194
def u16(b,p):return struct.unpack_from("<H",b,p)[0]
def u32(b,p):return struct.unpack_from("<I",b,p)[0]
def hits(b,needle,start=0,end=LE,limit=100):
 out=[];pos=start
 while len(out)<limit:
  pos=b.find(needle,pos,end)
  if pos<0:break
  out.append(pos);pos+=1
 return out
def hx(xs):return ",".join(f"0x{x:x}" for x in xs)
def ascii_at(b,p):
 if not 0<=p<LE:return None
 e=b.find(b"\0",p,min(LE,p+128))
 if e<0 or e==p:return None
 raw=b[p:e]
 if not all(32<=c<=126 for c in raw):return None
 return raw.decode("ascii")
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument("ovl",type=Path);args=ap.parse_args()
 b=args.ovl.read_bytes()
 print(f"M393_INPUT bytes={len(b)} sha256={hashlib.sha256(b).hexdigest()}")
 if b[:2]!=b"MZ" or b[LE:LE+2]!=b"LE":raise SystemExit("M393_ERROR invalid OVL signatures")
 end=TABLE+COUNT*8
 print(f"M393_TABLE file=0x{TABLE:x} count={COUNT} endExclusive=0x{end:x} bytes={COUNT*8}")
 print(f"M393_NEIGHBOR before64={b[TABLE-64:TABLE].hex()} after256={b[end:end+256].hex()}")
 # Print each 4-byte field and possible adjacent 16-bit fields in the
 # 192 bytes after the known table; avoid treating arbitrary bytes as pointers.
 for p in range(end,end+192,4):
  v=u32(b,p)
  nameCandidate=ascii_at(b,v-0xA4F48C)
  codeOffset=v&65535
  print(f"M393_POST_DWORD file=0x{p:x} value=0x{v:08x} low16=0x{v&65535:04x} high16=0x{v>>16:04x} nameViaKnownBase={nameCandidate!r} codeHigh88={int(v>>16==0x88)}")
 for p in range(end,end+96,2):
  print(f"M393_POST_WORD file=0x{p:x} value=0x{u16(b,p):04x}")
 # Verify known string pointer base and look for any nearby additional
 # NUL-terminated names, including after table and in parallel string runs.
 base=0xA4F48C
 for p in range(0x328d0,0x32b80):
  if p>=LE:break
  if (p==0 or b[p-1]==0) and 32<=b[p]<=126:
   val=ascii_at(b,p)
   if val and len(val)>=3:
    print(f"M393_STRING file=0x{p:x} value={val!r} affinePointer=0x{base+p:08x}")
 for name in ("LINEXE_LOADER","SEGEXE_LOADER","EXP_LOADER","LINEXE_MOD_INFO_SEG","DVX_Which_Loader","DVX_Process_Manager","DOS4GMAINLOADERVECT"):
  needle=name.encode()+b"\0"
  positions=hits(b,needle)
  print(f"M393_MARKER name={name!r} count={len(positions)} files={hx(positions)}")
  for p in positions[:12]:
   print(f"M393_MARKER_CONTEXT name={name!r} file=0x{p:x} before32={b[max(0,p-32):p].hex()} after64={b[p+len(needle):p+len(needle)+64].hex()}")
 # Search for stored pointers to named module strings using established
 # affine mapping. Occurrences may be data, code immediates, or coincidence.
 for name in ("LINEXE_LOADER","SEGEXE_LOADER","EXP_LOADER","LINEXE_MOD_INFO_SEG"):
  for p in hits(b,name.encode()+b"\0"):
   ptr=base+p
   pp=hits(b,struct.pack("<I",ptr),0,LE,100)
   print(f"M393_NAME_POINTER name={name!r} stringFile=0x{p:x} candidatePointer=0x{ptr:08x} references={hx(pp)}")
 # Check known 17-entry table and its neighboring records for other
 # repeating pair layouts; report scores rather than asserting descriptors.
 for start in range(TABLE-64,end+96,4):
  pairs=[]
  for i in range(4):
   p=start+i*8
   if p+8>LE:break
   a,c=u32(b,p),u32(b,p+4)
   text=ascii_at(b,a-base)
   pairs.append((text,c>>16))
  names=sum(t is not None for t,_ in pairs)
  code88=sum(hi==0x88 for _,hi in pairs)
  if names>=2 or code88>=3:
   print(f"M393_PAIR_CANDIDATE start=0x{start:x} nameMatches={names} codeHigh88={code88} entries={pairs!r}")
 print("M393_LIMITATIONS affineStringMappingNotRuntimeBase=1 descriptorLayoutUnknown=1 staticReferencesNotCalls=1 noExecution=1")
if __name__=="__main__":main()
