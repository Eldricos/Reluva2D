# M385 — dos extender loader inventory

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m385-dos-extender-loader-inventory.txt`
- Original bytes: 17886
- SHA256: `664a3df1aa051e657ce88d5ec132eeb629bffe414201a8a6b709496c219db6ec`
- Original encoding: `utf-16`
- Original line count: 107
- Curated lines retained: 25

## Selected diagnostic observations

- `M385_MZ lastPageBytes=0x74 pages=0x7b relocations=0x4e headerParagraphs=0x20 minAlloc=0x1 maxAlloc=0xffff initialSS=0x121 initialSP=0x340 initialIP=0x2382 initialCS=0x1bd relocationTable=0x1e overlayNumber=0x0 newHeaderPointer=0x9b40000`
- `M385_SIGNATURE needle=LE countAtLeast=24 offsets=0x1bae2,0x1c18d,0x1c3f4,0x1c401,0x1c973,0x1c99c,0x1dc0b,0x2a61b,0x3023e,0x31802,0x32744,0x32959,0x32981,0x329b9,0x329cd,0x32b51,0x32c25,0x3350e,0x33869,0x3476f,0x38194,0x151488,0x151501,0x15157c`
- `M385_SIGNATURE needle=LX countAtLeast=6 offsets=0x1baf0,0x2a622,0x2c9b2,0x3024c,0x336a7,0x33719`
- `M385_SIGNATURE needle=BW countAtLeast=9 offsets=0x79b0,0xf474,0x1e0c4,0x11c37b,0x11c41c,0x11c4ee,0x11c590,0x15632b,0x1ccd41`
- `M385_SIGNATURE needle=DOS/4G countAtLeast=8 offsets=0x25c,0x1c2bd,0x1f620,0x3162e,0x31985,0x31cff,0x3367e,0x16380d`
- `M385_SIGNATURE needle=DOS4GW countAtLeast=1 offsets=0x3379a`
- `M385_SIGNATURE needle=RATIONAL countAtLeast=4 offsets=0x1f617,0x158c23,0x158c66,0x163804`
- `M385_SIGNATURE needle=DPMI countAtLeast=7 offsets=0x10a6,0x1a36,0x1c09,0x1c36,0x1c4d,0x1c7e,0x1c9f`
- `M385_SIGNATURE needle=Phar Lap countAtLeast=1 offsets=0x31c8e`
- `M385_SIGNATURE needle=VCPI countAtLeast=5 offsets=0x10a1,0x1a2e,0x1b18,0x1b2a,0x1b60`
- `M385_LE_HEADER offset=0x38194 magic=LE cpu=0x2 os=0x1 moduleFlags=0x200 pages=371 eipObject=1 eip=0xa2554 espObject=2 esp=0x1821b0 autoDataObject=1737`
- `M385_TEXT keyword='rational' offset=0x153379 text='Current status of this unit is: Fully Operational.  '`
- `M385_TEXT keyword='dpmi' offset=0x1c10 text='protected mode already in use in this DPMI virtual machine'`
- `M385_TEXT keyword='vcpi' offset=0x1b0a text='no memory for VCPI page table'`
- `M385_TEXT keyword='vcpi' offset=0x1b2a text='VCPI page table address incorrect'`
- `M385_TEXT keyword='phar lap' offset=0x31c5f text='Note: this program may have been built for the Phar Lap extender.'`
- `M385_TEXT keyword='linear executable' offset=0x32851 text='invalid linear executable format'`
- `M385_TEXT keyword='protected mode' offset=0x1c10 text='protected mode already in use in this DPMI virtual machine'`
- `M385_TEXT keyword='stub' offset=0x37deb text='Stub exec failed:'`
- `M385_STRINGS totalPrintableRuns5=5404`
- `M385_MZ lastPageBytes=0x12c pages=0x31 relocations=0xcd headerParagraphs=0x36 minAlloc=0x83 maxAlloc=0xffff initialSS=0x5df initialSP=0x800 initialIP=0x10f6 initialCS=0x0 relocationTable=0x20 overlayNumber=0x0 newHeaderPointer=0x3ddb`
- `M385_SIGNATURE needle=LE countAtLeast=2 offsets=0x5af2,0x5b06`
- `M385_SIGNATURE needle=DPMI countAtLeast=2 offsets=0x5c35,0x5c52`
- `M385_STRINGS totalPrintableRuns5=108`
- `M385_LIMITATIONS staticIdentificationOnly=1 loaderNotExecuted=1 runtimeSelectorsUnobserved=1 signatureHitsNotProof=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
