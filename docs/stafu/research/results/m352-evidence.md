# M352 — handler address validation

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m352-handler-address-validation.txt`
- Original bytes: 308142
- SHA256: `856361660d54c2ab7609628591d874bc3b02ec8eb142976145a87b021784e231`
- Original encoding: `utf-16`
- Original line count: 1019
- Curated lines retained: 35

## Selected diagnostic observations

- `M352_OBJECT index=1 base=0x10000 size=0xd889a first=1 pages=217`
- `M352_OBJECT index=2 base=0xf0000 size=0x1821b0 first=218 pages=154`
- `M352_TABLE_BEGIN origin=headerPlusField name=object_state tableRva=0x12334`
- `M352_TABLE_LOCATION interpretation=objectRelative rva=0x12334 physical=0x8d2c8 rawHex=e089c1f7d18b451421483ceb08837dd82074d8ebdf8b45f80145fce9`
- `M352_TABLE_LOCATION interpretation=absoluteMinusBase rva=0x2334 physical=0x7d2c8 rawHex=3a0f87f2feffff2eff248d2051000050b8f095090050b8ae0c000050`
- `M352_TARGET origin=headerPlusField table=object_state state=1 relocationValue=0x1239b interpretation=objectRelative rva=0x1239b physical=0x8d32f insnCount=0 coveredBytes=0 firstBytes=feff83c40ceb338b45f48b40088945d4`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=-8 physical=578343 instructions=17 coveredBytes=48 first=add byte ptr [eax], al`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=-4 physical=578347 instructions=3 coveredBytes=4 first=add al, ch`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=-2 physical=578349 instructions=2 coveredBytes=2 first=lahf `
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=-1 physical=578350 instructions=1 coveredBytes=1 first=out dx, eax`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=0 physical=578351 instructions=0 coveredBytes=0 first=none`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=1 physical=578352 instructions=15 coveredBytes=47 first=inc dword ptr [ebx + 0x33eb0cc4]`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=2 physical=578353 instructions=17 coveredBytes=48 first=add esp, 0xc`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=4 physical=578355 instructions=14 coveredBytes=46 first=or al, 0xeb`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=objectRelative delta=8 physical=578359 instructions=18 coveredBytes=48 first=inc ebp`
- `M352_TARGET origin=headerPlusField table=object_state state=1 relocationValue=0x1239b interpretation=absoluteMinusBase rva=0x239b physical=0x7d32f insnCount=15 coveredBytes=47 firstBytes=b8090083c4105bc35068f095090068d1`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=-8 physical=512807 instructions=17 coveredBytes=44 first=enter 0xc, 0`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=-4 physical=512811 instructions=18 coveredBytes=46 first=push eax`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=-2 physical=512813 instructions=16 coveredBytes=44 first=call 0x118ba7`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=-1 physical=512814 instructions=18 coveredBytes=48 first=jne 0x7d2e8`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=0 physical=512815 instructions=15 coveredBytes=47 first=mov eax, 0xc4830009`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=1 physical=512816 instructions=17 coveredBytes=46 first=or dword ptr [eax], eax`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=2 physical=512817 instructions=15 coveredBytes=48 first=add byte ptr [ebx - 0x3ca4ef3c], al`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=4 physical=512819 instructions=19 coveredBytes=48 first=les edx, ptr [eax]`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=1 interpretation=absoluteMinusBase delta=8 physical=512823 instructions=17 coveredBytes=45 first=push eax`
- `M352_TARGET origin=headerPlusField table=object_state state=2 relocationValue=0x1238e interpretation=objectRelative rva=0x1238e physical=0x8d322 insnCount=17 coveredBytes=46 firstBytes=ff50b87e1e0000506a00e89feffeff83`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=-8 physical=578330 instructions=1 coveredBytes=6 first=add byte ptr [ebx - 0x1e17f33c], al`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=-4 physical=578334 instructions=16 coveredBytes=47 first=call 0x7af04`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=-2 physical=578336 instructions=0 coveredBytes=0 first=none`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=-1 physical=578337 instructions=0 coveredBytes=0 first=none`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=0 physical=578338 instructions=17 coveredBytes=46 first=call dword ptr [eax - 0x48]`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=1 physical=578339 instructions=16 coveredBytes=45 first=push eax`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=2 physical=578340 instructions=15 coveredBytes=44 first=mov eax, 0x1e7e`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=4 physical=578342 instructions=17 coveredBytes=47 first=push ds`
- `M352_NEIGHBOR origin=headerPlusField table=object_state state=2 interpretation=objectRelative delta=8 physical=578346 instructions=16 coveredBytes=47 first=push 0`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
