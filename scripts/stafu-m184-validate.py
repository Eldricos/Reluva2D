#!/usr/bin/env python3
"""Read-only validation of M183 windows against known M181 bytes."""
import argparse, zipfile
from pathlib import Path
EXPECTED = {
 'm183-rmLoader-510528.bin': (32, '53 56 57 55 83 ec 04'),
 'm183-paletteInitializer-510388.bin': (32, '53 56 57 8b 15 b0 96 09 00'),
 'm183-paletteSetup-510550.bin': (32, 'e8 59 ff ff ff'),
 'm183-preCopyHelper-510648.bin': (32, 'e8 5b fe ff ff'),
}
def main():
 p=argparse.ArgumentParser();p.add_argument('zipfile',type=Path);a=p.parse_args()
 with zipfile.ZipFile(a.zipfile) as z:
  for name,(index,hexstr) in EXPECTED.items():
   data=z.read(name); expected=bytes.fromhex(hexstr); actual=data[index:index+len(expected)]
   print(f'{name}: {"PASS" if actual==expected else "MISMATCH"} at center+0 ({index}) expected={expected.hex(" ")} actual={actual.hex(" ")}')
  print('No original executable code is executed or modified.')
if __name__=='__main__': main()
