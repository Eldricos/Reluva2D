# M426 — spr callsite target

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m426-spr-callsite-target.txt`
- Original bytes: 5528
- SHA256: `69897cbc36da152b41e3b2d76243ab6d753536db676202a1e51979824256b401`
- Original encoding: `utf-16`
- Original line count: 19
- Curated lines retained: 9

## Selected diagnostic observations

- `M426_OBJECT index=1 base=0x10000 size=0xd889a fileStart=0x7af94 assumedLinear=1`
- `M426_OBJECT index=2 base=0xf0000 size=0x1821b0 fileStart=0x153f94 assumedLinear=1`
- `M426_SITE file=0x9f03c object1Rel=0x240a8 assumedLinear=0x340a8`
- `M426_PATTERN count=1 positions=['0x9f035']`
- `M426_CALLSITE mov1=0x3e3c mov2=0x3e54 callFile=0x9f041 rel32=-61686 targetFile=0x8ff50 inObject1=1`
- `M426_ARGUMENT name=arg1 object2Rel=0x3e3c file=0x157dd0 string='eplay Conv'`
- `M426_ARGUMENT name=arg2 object2Rel=0x3e54 file=0x157de8 string='brdgrikr.spr'`
- `M426_TARGET_RET_BYTES first=['0x8ffb2', '0x8ffe7', '0x900aa', '0x90104', '0x90133']`
- `M426_LIMITATIONS patternAlignedOnly=1 targetRawCallsNotDisassembled=1 noFixupDecode=1 noRuntimeTrace=1 noResourceLoaderConfirmed=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
