# M364 — le selector fixup crosscheck

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m364-le-selector-fixup-crosscheck.txt`
- Original bytes: 12674
- SHA256: `3c6a2f7f82ba799add25d16b21997fc6b2f7dfc7bf2b8e71f029d6a4c0e4ba9f`
- Original encoding: `utf-16`
- Original line count: 37
- Curated lines retained: 13

## Selected diagnostic observations

- `M364_LAYOUT pages=371 pageSize=4096 objects=2 fixupIndex=231518 fixupRecords=233006`
- `M364_OBJECT number=1 base=0x10000 virtualSize=0xd889a flags=0x2045 firstPage=1 pages=217`
- `M364_OBJECT number=2 base=0xf0000 virtualSize=0x1821b0 flags=0x2043 firstPage=218 pages=154`
- `M364_FORMAT_RULES sourceType07=offset32 targetFlags10=targetOffset32 sourceOffset=pageRelative targetObject=oneBased targetOffset=objectRelative`
- `M364_FORMAT_CAUTION sourceType07AndFlag10AreDifferentFields=1 runtimeSelectorBaseUnresolved=1`
- `M364_CASE_BEGIN name=object_state dispatcherPhysical=566541 dispatcherObject=0x1 dispatcherRva=0xf579 instructionValid=1 displacement=0x12334 fixupLogicalPage=19`
- `M364_SOURCE name=object_state sourcePhysical=0x8d2c8 originalBytes=e089c1f7d18b451421483ceb08837dd82074d8ebdf8b45f80145fce9 candidateCount=7`
- `M364_FIXUP_SET name=object_state sevenUniqueInBounds=1`
- `M364_CASE_END name=object_state`
- `M364_CASE_BEGIN name=global_state dispatcherPhysical=570965 dispatcherObject=0x1 dispatcherRva=0x106c1 instructionValid=1 displacement=0x13488 fixupLogicalPage=20`
- `M364_SOURCE name=global_state sourcePhysical=0x8e41c originalBytes=e883783400750cc745e4ffffffffe9f50300008b45e88b40348b4002 candidateCount=7`
- `M364_FIXUP_SET name=global_state sevenUniqueInBounds=1`
- `M364_CASE_END name=global_state`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
