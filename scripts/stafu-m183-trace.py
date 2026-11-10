#!/usr/bin/env python3
"""STAFU M183 read-only byte checks and extraction of helper windows.
Usage: python stafu-m183-trace.py stafu-m181-copy-hex.txt [TITLE.FVF]
File offsets are offsets in TITLE.FVF, NOT LE virtual addresses.
"""
import re, sys, pathlib, unittest
p=pathlib.Path(sys.argv[1]) if len(sys.argv)>1 else pathlib.Path('stafu-m181-copy-hex.txt')
source=p.read_text(encoding='utf-8-sig')
mem={}; regions=[]
for line in source.splitlines():
 m=re.search(r'region=(\w+) file=(\d+) hex=([0-9a-fA-F ]+)',line)
 if not m: continue
 name,start,hexstr=m.groups(); start=int(start); bs=bytes.fromhex(hexstr); regions.append((name,start,len(bs)))
 for i,b in enumerate(bs):
  if start+i in mem and mem[start+i]!=b: raise ValueError(f'conflicting byte at {start+i}')
  mem[start+i]=b

def got(offset,pattern):
 bs=bytes.fromhex(pattern)
 return all(mem.get(offset+i)==b for i,b in enumerate(bs))

class Evidence(unittest.TestCase):
 def test_entry_local_allocation(self): self.assertTrue(got(510528,'53 56 57 55 83 ec 04'))
 def test_stack_pointer_capture(self): self.assertTrue(got(510555,'89 e3 53'))
 def test_prior_helper_call(self): self.assertTrue(got(510563,'e8 48 cc 00 00'))
 def test_ebp_from_eax(self): self.assertTrue(got(510575,'89 c5'))
 def test_precopy_helper_call(self): self.assertTrue(got(510648,'e8 5b fe ff ff'))
 def test_post_helper_ebx(self): self.assertTrue(got(510653,'89 d9 c1 e3 02 83 c4 10 29 cb'))
 def test_copy_length_stack(self): self.assertTrue(got(510698,'8b 0c 24 8b 3d 4c 53 01 00 8d 34 2b 29 d9'))
 def test_copy_f2_prefixes(self): self.assertTrue(got(510715,'c1 e9 02 f2 a5 8a c8 80 e1 03 f2 a4'))
 def test_flag21_caller(self): self.assertTrue(got(1010659,'6a 21'))
 def test_flag23_caller(self): self.assertTrue(got(827165,'6a 23'))

print('RM_M183_INPUT regions=%d distinctBytes=%d'%(len(regions),len(mem)))
result=unittest.TextTestRunner(verbosity=1).run(unittest.defaultTestLoader.loadTestsFromTestCase(Evidence))
print('RM_M183_TESTS passed=%d failed=%d errors=%d'%(result.testsRun-len(result.failures)-len(result.errors),len(result.failures),len(result.errors)))
print('RM_M183_STACK localAllocation=4 capturedStackAddress=observed helperWrites=unverified')
print('RM_M183_EBX postHelperArithmetic=3_times_returned_EBX helperPreservesEBX=unverified')
print('RM_M183_COPY source=EBP_plus_EBX length=stackLocal_minus_EBX destination=globalPointer_0x1534C')
print('RM_M183_LIMITATION helperBodies=notInHex runtimeRegisters=unverified paletteProvenance=unverified')
if len(sys.argv)>2:
 data=pathlib.Path(sys.argv[2]).read_bytes()
 # Include both identified call targets and nearby control-flow for independent disassembly.
 windows=[('priorHelper',510232),('paletteInitializer',510388),('paletteSetup',510550),('preCopyHelper',510648),('postCopyCleanup',562937),('rmLoader',510528)]
 for name,off in windows:
  if off>=len(data): print(f'RM_M183_WINDOW name={name} offset={off} available=0');continue
  lo=max(0,off-32); hi=min(len(data),off+160)
  out=pathlib.Path(f'm183-{name}-{off}.bin');out.write_bytes(data[lo:hi])
  print(f'RM_M183_WINDOW name={name} offset={off} from={lo} to={hi} file={out}')
if not result.wasSuccessful(): sys.exit(1)
