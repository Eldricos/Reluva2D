#!/usr/bin/env python3
"""STAFU M206: read-only provenance survey of palette resource arguments.

This diagnostic does NOT assume that small immediates are pointers or IDs.
It exports the surrounding code, scans exact references, and inventories
possible string data in the original OVL and optional installed EXE.
"""
import argparse, hashlib, pathlib, struct, sys, re
EXPECTED='65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
VALUES=[0xc08,0xc4e6,0xc4f3,0xc582,0xc4c4,0xc4fd]
WINDOWS=[('palette-init',510200,510740),('resource-wrapper',562760,563240),('resource-lookup',515800,517050),('resource-inner',513200,514000),('bridge-a',1010000,1011350),('bridge-b',1011700,1013000),('palette-dac',637900,638500)]
SITES=[(510412,562864),(510563,562864),(510648,510232),(562901,513376)]
def sha(x):return hashlib.sha256(x).hexdigest()
def ascii_candidates(b,low=4):
 for m in re.finditer(rb'[\x20-\x7e]{'+str(low).encode()+rb',}',b):
  s=m.group();
  if b'.' in s or b'PAL' in s.upper() or b'BRID' in s.upper() or b'RM' in s.upper():yield m.start(),s

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('ovl',type=pathlib.Path);p.add_argument('--exe',type=pathlib.Path,default=pathlib.Path('games/STTNG/STTNG.EXE'))
 p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('m206-output'))
 a=p.parse_args();d=a.ovl.read_bytes();valid=len(d)==2008455 and sha(d)==EXPECTED and d[229780:229782]==b'LE'
 print(f'RM_M206_INPUT bytes={len(d)} sha256={sha(d)} knownMatch={int(valid)}')
 if not valid:return 2
 for site,target in SITES:
  op=d[site:site+5];found=site+5+struct.unpack_from('<i',d,site+1)[0] if op[0]==0xe8 else None
  print(f'RM_M206_CALL site={site} expected={target} actual={found} match={int(found==target)}')
  if found!=target:return 2
 a.out.mkdir(parents=True,exist_ok=True);manifest=[]
 for name,start,end in WINDOWS:
  data=d[start:end];filename=f'm206-{name}-{start}.bin';(a.out/filename).write_bytes(data)
  manifest.append(f'{filename} physical={start}:{end} sha256={sha(data)}')
  print(f'RM_M206_WINDOW name={name} start={start} end={end} bytes={len(data)} sha256={sha(data)}')
 for value in VALUES:
  pat=struct.pack('<I',value);hits=[m.start() for m in re.finditer(re.escape(pat),d)]
  print(f'RM_M206_IMMEDIATE value=0x{value:x} rawDwordMatches={len(hits)} sites={",".join(map(str,hits[:70]))} truncated={max(0,len(hits)-70)}')
  # Export each distinct vicinity; arbitrary matches are not instruction boundaries.
  for i,site in enumerate(hits[:35]):
   start=max(0,site-48);end=min(len(d),site+56)
   fn=f'm206-imm-{value:x}-{i:02d}-{start}.bin';(a.out/fn).write_bytes(d[start:end]);manifest.append(f'{fn} match={site} physical={start}:{end}')
 for label,b in [('OVL',d)]+([('EXE',a.exe.read_bytes())] if a.exe.is_file() else []):
  print(f'RM_M206_FILE label={label} bytes={len(b)} sha256={sha(b)}')
  strings=list(ascii_candidates(b));interesting=[(pos,s) for pos,s in strings if any(x in s.upper() for x in (b'BRIDGE',b'STANDARD.PAL',b'VIEWSCR',b'.PAL',b'.RM'))]
  for pos,s in interesting[:90]:print(f'RM_M206_STRING file={label} physical={pos} text={s[:120].decode("ascii",errors="replace")!r}')
  print(f'RM_M206_STRING_COUNT file={label} relevant={len(interesting)} truncated={max(0,len(interesting)-90)}')
  (a.out/f'm206-{label.lower()}-strings.txt').write_text('\n'.join(f'{pos}: {s[:200].decode("ascii",errors="replace")}' for pos,s in strings)+'\n',encoding='utf-8')
  for value in VALUES:
   for width in (2,4):
    pat=value.to_bytes(width,'little');hits=[m.start() for m in re.finditer(re.escape(pat),b)]
    if label=='EXE':print(f'RM_M206_EXE_VALUE value=0x{value:x} width={width} matches={len(hits)} first={hits[:30]}')
 (a.out/'m206-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
 print(f'RM_M206_OUTPUT directory={a.out} files={len(list(a.out.iterdir()))}')
 print('RM_M206_LIMITATION staticOnly=1 immediatesUnresolved=1 stringProximityNotProof=1 fixupsUnapplied=1')
 return 0
if __name__=='__main__':sys.exit(main())
