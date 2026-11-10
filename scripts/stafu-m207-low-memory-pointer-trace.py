#!/usr/bin/env python3
"""STAFU M207: read-only LE runtime-address provenance and resource-name contexts.

This does not resolve DOS-extender low-memory pointers without a runtime dump.
"""
import argparse,hashlib,pathlib,struct,re,sys
SHA='65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697'
H=229780
VALUES={'default-upper':0xc08,'screen-a-upper':0xc4e6,'screen-a-resource':0xc4f3,'screen-b-upper':0xc582,'screen-a-other':0xc4c4,'screen-a-other2':0xc4fd}
STRINGS=['STANDARD.PAL','bridge.rm','viewscr.rm','holodeck.rm','endgame.rm']
WINDOWS=[('bridge-a',1010350,1010860),('bridge-b',1012080,1012480),('palette-init',510350,510680),('lookup',515970,516380),('resource-inner',513320,513740)]
def u32(b,p):return struct.unpack_from('<I',b,p)[0]
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('ovl',type=pathlib.Path);p.add_argument('--exe',type=pathlib.Path,default=pathlib.Path('games/STTNG/STTNG.EXE'));p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('m207-output'));a=p.parse_args()
 b=a.ovl.read_bytes();ok=len(b)==2008455 and sha(b)==SHA and b[H:H+2]==b'LE'
 print(f'RM_M207_INPUT bytes={len(b)} sha256={sha(b)} knownMatch={int(ok)}')
 if not ok:return 2
 ps=u32(b,H+0x28);obrel=u32(b,H+0x40);cnt=u32(b,H+0x44);mprel=u32(b,H+0x48);dataoff=u32(b,H+0x80)
 if ps!=4096 or cnt!=2 or dataoff!=273920:print('RM_M207_ERROR unexpected LE layout');return 2
 objects=[];page_maps=[]
 for i in range(cnt):
  size,base,flags,first,npage=struct.unpack_from('<IIIII',b,H+obrel+i*24)
  objects.append((size,base,first,npage));print(f'RM_M207_OBJECT index={i+1} size={size} base=0x{base:x} firstPage={first} pages={npage} range=0x{base:x}-0x{base+size-1:x}')
  for j in range(npage):
   raw=b[H+mprel+(first-1+j)*4:H+mprel+(first+j)*4]
   page_maps.append((i+1,j,int.from_bytes(raw[:3],'big'),raw[3]))
 def map_phys(pos):
  if pos<dataoff:return []
  page=(pos-dataoff)//ps+1;off=(pos-dataoff)%ps
  return [(oi,objects[oi-1][1]+j*ps+off) for oi,j,pg,fl in page_maps if pg==page and fl==0 and j*ps+off<objects[oi-1][0]]
 def map_linear(addr):
  result=[]
  for oi,(size,base,first,npage) in enumerate(objects,1):
   if base<=addr<base+size:
    j,off=divmod(addr-base,ps)
    raw=b[H+mprel+(first-1+j)*4:H+mprel+(first+j)*4];pg=int.from_bytes(raw[:3],'big');fl=raw[3]
    result.append((oi,dataoff+(pg-1)*ps+off if pg and fl==0 else None,fl))
  return result
 for name,value in VALUES.items():
  mappings=map_linear(value)
  print(f'RM_M207_POINTER name={name} value=0x{value:x} leObjectMapping={mappings} belowLowestLEBase={int(value<min(x[1] for x in objects))}')
  for label,off in [('raw-ovl-physical',value),('raw-exe-physical',value)]:
   src=b if label=='raw-ovl-physical' else (a.exe.read_bytes() if a.exe.is_file() else b'')
   if off<len(src):
    chunk=src[max(0,off-24):min(len(src),off+96)]
    print(f'RM_M207_RAW_PROBE name={name} source={label} offset={off} first32={src[off:off+32].hex()} ascii={repr(src[off:off+48].decode("ascii","replace"))}')
 a.out.mkdir(parents=True,exist_ok=True);manifest=[]
 for label,start,end in WINDOWS:
  fn=f'm207-{label}-{start}.bin';part=b[start:end];(a.out/fn).write_bytes(part);manifest.append(f'{fn} physical={start}:{end} sha256={sha(part)}')
  print(f'RM_M207_WINDOW name={label} start={start} end={end} bytes={len(part)} sha256={sha(part)}')
 for target in STRINGS:
  needle=target.encode('ascii');hits=[m.start() for m in re.finditer(re.escape(needle),b,re.I)]
  for idx,pos in enumerate(hits):
   start=max(0,pos-96);end=min(len(b),pos+128);fn=f'm207-string-{target.replace(".","_")}-{idx}-{pos}.bin';part=b[start:end];(a.out/fn).write_bytes(part);manifest.append(f'{fn} stringAt={pos} mapped={map_phys(pos)}')
   print(f'RM_M207_STRING text={target} physical={pos} runtimeCandidates={map_phys(pos)} contextFile={fn}')
 # Export header/loader metadata for independent investigation.
 for label,start,end in [('mz-header',0,128),('le-header',H,H+196),('le-object-table',H+obrel,H+obrel+cnt*24),('le-page-map',H+mprel,H+mprel+sum(o[3] for o in objects)*4)]:
  fn=f'm207-{label}.bin';(a.out/fn).write_bytes(b[start:end]);manifest.append(f'{fn} physical={start}:{end}')
  print(f'RM_M207_METADATA name={label} start={start} end={end} bytes={end-start}')
 if a.exe.is_file():
  exe=a.exe.read_bytes();(a.out/'m207-sttng-exe-header.bin').write_bytes(exe[:256]);manifest.append('m207-sttng-exe-header.bin')
  print(f'RM_M207_EXE bytes={len(exe)} sha256={sha(exe)} mz={int(exe[:2]==b"MZ")}')
 (a.out/'m207-manifest.txt').write_text('\n'.join(manifest)+'\n',encoding='utf-8')
 print(f'RM_M207_OUTPUT directory={a.out} files={len(list(a.out.iterdir()))}')
 print('RM_M207_LIMITATION staticOnly=1 lowMemoryNotDumped=1 rawOffsetProbesNotPointerResolution=1 relocationUnapplied=1 nameMappingUnverified=1')
 return 0
if __name__=='__main__':sys.exit(main())
