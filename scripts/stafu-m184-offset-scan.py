#!/usr/bin/env python3
"""Read-only STAFU M184: compare reported byte offsets with actual TITLE.FVF bytes.
Usage: python stafu-m184-offset-scan.py stafu-m181-copy-hex.txt games/STAFU-CD/TITLE.FVF [m183-bin.zip]
No files are modified. Searches exact byte anchors and reports candidate constant deltas.
"""
import argparse, collections, pathlib, re, zipfile, hashlib
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('hex_report',type=pathlib.Path)
p.add_argument('executable',type=pathlib.Path)
p.add_argument('windows_zip',type=pathlib.Path,nargs='?')
a=p.parse_args()
mem={}; regions=[]
for line in a.hex_report.read_text(encoding='utf-8-sig').splitlines():
 m=re.search(r'region=(\w+) file=(\d+) hex=([0-9a-fA-F ]+)',line)
 if not m: continue
 name,off,hexstr=m.groups(); off=int(off); raw=bytes.fromhex(hexstr); regions.append((name,off,raw))
 for i,b in enumerate(raw):
  old=mem.setdefault(off+i,b)
  if old!=b: raise ValueError(f'conflicting reported bytes at {off+i}')
data=a.executable.read_bytes()
print(f'RM_M184_FILE path={a.executable} bytes={len(data)} sha256={hashlib.sha256(data).hexdigest()}')
print(f'RM_M184_REPORT regions={len(regions)} distinctBytes={len(mem)}')
anchors=[('rmEntry',510528,bytes.fromhex('53 56 57 55 83 ec 04 f6 44 24 1c 20')),
 ('rmFlag',510599,bytes.fromhex('f6 c6 01 74 3d f6 c6 80 75 38')),
 ('rmCopy',510698,bytes.fromhex('8b 0c 24 8b 3d 4c 53 01 00 8d 34 2b 29 d9')),
 ('paletteInit',510388,bytes.fromhex('53 56 57 8b 15 b0 96 09 00')),
 ('caller21',1010659,bytes.fromhex('6a 21 b8 f3 c4 00 00')),
 ('caller23',827165,bytes.fromhex('6a 23 b8 3e 6c 00 00'))]
found={}
for name,off,pat in anchors:
 direct=off+len(pat)<=len(data) and data[off:off+len(pat)]==pat
 hits=[]; start=0
 while len(hits)<30:
  pos=data.find(pat,start)
  if pos<0: break
  hits.append(pos);start=pos+1
 more=data.find(pat,start)>=0
 found[name]=hits
 print(f'RM_M184_ANCHOR name={name} reported={off} direct={int(direct)} matches={len(hits)} truncated={int(more)} positions={",".join(map(str,hits)) or "none"} deltas={",".join(str(x-off) for x in hits) or "none"}')
# A single constant delta is supported only if multiple independent anchors agree.
votes=collections.defaultdict(set)
for name,off,_ in anchors:
 for pos in found[name]: votes[pos-off].add(name)
for delta,names in sorted(votes.items(),key=lambda x:(-len(x[1]),abs(x[0])))[:10]:
 print(f'RM_M184_DELTA candidate={delta} anchors={len(names)} names={",".join(sorted(names))}')
if a.windows_zip:
 with zipfile.ZipFile(a.windows_zip) as z:
  for name in sorted(z.namelist()):
   m=re.search(r'-(\d+)\.bin$',name)
   if not m: continue
   off=int(m.group(1));lo=max(0,off-32); sample=z.read(name)
   match=lo+len(sample)<=len(data) and data[lo:lo+len(sample)]==sample
   print(f'RM_M184_WINDOW name={name} bytes={len(sample)} matchesPhysicalFile={int(match)}')
print('RM_M184_LIMITATION noExecutableRemappingAssumed=1 noHelperSemanticsProven=1')
