#!/usr/bin/env python3
"""STAFU M205: read-only palette callsite extraction and candidate resource comparison."""
import argparse, hashlib, pathlib, struct, sys, zipfile, zlib
SHA='65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
WINDOWS=[('palette-init',510320,510535),('palette-loader',510528,511200),('palette-helper',510232,510388),('wrapper',562864,563100),('inner-loader',513376,513964),('lookup',516148,516800),('bridge-caller-a',1010450,1010800),('bridge-caller-b',1012100,1012430),('callsite-a',509800,510030),('palette-dac',638078,638480)]
CALLS=[(510412,562864),(510563,562864),(510648,510232),(562901,513376),(510348,638078),(510376,638078)]
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('ovl',type=pathlib.Path);p.add_argument('--installed',type=pathlib.Path,default=pathlib.Path('games/STTNG'));p.add_argument('--cd',type=pathlib.Path,default=pathlib.Path('games/STAFU-CD'));p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('m205-output'));a=p.parse_args()
 d=a.ovl.read_bytes();ok=len(d)==2008455 and sha(d)==SHA and d[229780:229782]==b'LE';print(f'RM_M205_INPUT bytes={len(d)} sha256={sha(d)} knownMatch={int(ok)}')
 if not ok:return 2
 for site,target in CALLS:
  raw=d[site:site+5];found=site+5+struct.unpack_from('<i',d,site+1)[0] if raw[0]==0xe8 else None;valid=found==target;print(f'RM_M205_CALL site={site} target={found} expected={target} match={int(valid)}');
  if not valid:return 2
 a.out.mkdir(parents=True,exist_ok=True);manifest=[]
 for name,start,end in WINDOWS:
  b=d[start:end];fn=f'm205-{name}-{start}.bin';(a.out/fn).write_bytes(b);manifest.append(f'{fn} physical={start}:{end} sha256={sha(b)}');print(f'RM_M205_WINDOW name={name} start={start} end={end} bytes={len(b)} sha256={sha(b)}')
 # Keep physical, object-relative, and runtime virtual addresses separate.
 print('RM_M205_ARGUMENT callsite=510412 pushedFirst=0x00000c08 meaning=unverified')
 # Inspect named resources; no assertion that any one is the resource referenced by 0xC08.
 wanted={'standard.pal','bridge.rm','viewscr.rm'};entries={}
 for name in wanted:
  matches=list(a.installed.rglob(name)) if a.installed.exists() else []
  if not matches and a.installed.exists():matches=[x for x in a.installed.rglob('*') if x.is_file() and x.name.lower()==name]
  for f in matches[:3]:
   b=f.read_bytes();entries.setdefault(name,[]).append((str(f),b));print(f'RM_M205_INSTALLED name={name} path={str(f)!r} bytes={len(b)} sha256={sha(b)} first24={b[:24].hex()}')
 zpath=a.cd/'STTNG.ZIP'
 if zpath.is_file():
  with zipfile.ZipFile(zpath) as z:
   for info in z.infolist():
    name=info.filename.replace('\\','/').split('/')[-1].lower()
    if name in wanted:
     b=z.read(info);entries.setdefault(name,[]).append((f'{zpath}:{info.filename}',b));print(f'RM_M205_ARCHIVE name={name} method={info.compress_type} compressed={info.compress_size} bytes={len(b)} crc32={zlib.crc32(b)&0xffffffff:08x} sha256={sha(b)} first24={b[:24].hex()}')
 else:print(f'RM_M205_ARCHIVE_MISSING path={zpath}')
 # Compare exact 384-byte RGB candidates; this is a data comparison, not proof of runtime selection.
 bridge=entries.get('bridge.rm',[]);standard=entries.get('standard.pal',[])
 for bp,bb in bridge[:2]:
  for sp,sb in standard[:2]:
   if len(bb)>=384 and len(sb)>=384:
    same=sum(bb[i]==sb[i] for i in range(384));rgb=sum(bb[i:i+3]==sb[i:i+3] for i in range(0,384,3));print(f'RM_M205_COMPARE bridge={bp!r} palette={sp!r} prefixBytesEqual={same}/384 rgbTripletsEqual={rgb}/128')
 (a.out/'m205-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
 print(f'RM_M205_OUTPUT directory={a.out} files={len(WINDOWS)+1}')
 print('RM_M205_LIMITATION staticOnly=1 resourceArgument0xC08Unresolved=1 bridgeRuntimeFlagsUnverified=1 archiveMethodNotRuntimeMode=1')
 return 0
if __name__=='__main__':sys.exit(main())
