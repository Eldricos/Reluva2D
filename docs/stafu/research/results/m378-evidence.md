# M378 — le object address model

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m378-le-object-address-model.txt`
- Original bytes: 13484
- SHA256: `374cff16885c84d9263b7e3ce2f81f9f0f28340d06612acf4289d3b7875afe5f`
- Original encoding: `utf-16`
- Original line count: 39
- Curated lines retained: 11

## Selected diagnostic observations

- `M378_HEADER pageCount=371 pageSize=4096 dataOrigin=0x7af94 fixupIndexMonotonic=1`
- `M378_OBJECT id=1 base=0x10000 size=0xd889a flags=0x2045 firstPage=1 pages=217 linearEndExclusive=0xe889a`
- `M378_OBJECT id=2 base=0xf0000 size=0x1821b0 flags=0x2043 firstPage=218 pages=154 linearEndExclusive=0x2721b0`
- `M378_JUMP name=object_state object=1 rva=0xf579 preferredLinear=0x1f579 logicalPage=16 physical=0x8a50d exactSignature=1`
- `M378_SELECTOR name=object_state csBaseHypothesis=0x0 effectiveLinear=0x12334 mapped=obj1:rva0x2334 matchesProposedTable=0 observedRuntimeCS=0`
- `M378_SELECTOR name=object_state csBaseHypothesis=0x10000 effectiveLinear=0x22334 mapped=obj1:rva0x12334 matchesProposedTable=1 observedRuntimeCS=0`
- `M378_SELECTOR name=object_state csBaseHypothesis=0xf0000 effectiveLinear=0x102334 mapped=obj2:rva0x12334 matchesProposedTable=0 observedRuntimeCS=0`
- `M378_JUMP name=global_state object=1 rva=0x106c1 preferredLinear=0x206c1 logicalPage=17 physical=0x8b655 exactSignature=1`
- `M378_SELECTOR name=global_state csBaseHypothesis=0x0 effectiveLinear=0x13488 mapped=obj1:rva0x3488 matchesProposedTable=0 observedRuntimeCS=0`
- `M378_SELECTOR name=global_state csBaseHypothesis=0x10000 effectiveLinear=0x23488 mapped=obj1:rva0x13488 matchesProposedTable=1 observedRuntimeCS=0`
- `M378_SELECTOR name=global_state csBaseHypothesis=0xf0000 effectiveLinear=0x103488 mapped=obj2:rva0x13488 matchesProposedTable=0 observedRuntimeCS=0`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
