# M362 — le fixup semantics audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m362-le-fixup-semantics-audit.txt`
- Original bytes: 15996
- SHA256: `f6f48ea58676addd3fdba2ea0fa9726cec343a990577a2ec086e7bc5bf116bf3`
- Original encoding: `utf-16`
- Original line count: 48
- Curated lines retained: 16

## Selected diagnostic observations

- `M362_LAYOUT pageSize=4096 pages=371 objectCount=2 fixupIndex=231518 fixupRecords=233006`
- `M362_OBJECT number=1 size=0xd889a preferredBase=0x10000 firstLogicalPage=1 pageCount=217`
- `M362_OBJECT number=2 size=0x1821b0 preferredBase=0xf0000 firstLogicalPage=218 pageCount=154`
- `M362_INTERPRETATION sourceType07=32bitOffsetCandidate targetFlags10=32bitTargetOffsetCandidate sourceOffsetsPageRelativeCandidate=1 targetOffsetsObjectRelativeCandidate=1 selectorBaseUnknown=1`
- `M362_TABLE_BEGIN name=object_state logicalPage=19 recordBytes=1159 tableDisplacement=0x12334`
- `M362_DISPLACEMENT name=object_state object=1 interpretation=object_relative candidateRva=0x12334 mappedPhysical=0x8d2c8 candidateBytes=e089c1f7d18b451421483ceb08837dd82074d8ebdf8b45f80145fce9`
- `M362_DISPLACEMENT name=object_state object=1 interpretation=preferred_linear_minus_base candidateRva=0x2334 mappedPhysical=0x7d2c8 candidateBytes=3a0f87f2feffff2eff248d2051000050b8f095090050b8ae0c000050`
- `M362_DISPLACEMENT name=object_state object=2 interpretation=object_relative candidateRva=0x12334 mappedPhysical=0x1662c8 candidateBytes=0b08241e1e21241e1f1f25341f3f341f3f341f3f000603000e070015`
- `M362_DISPLACEMENT name=object_state object=2 interpretation=preferred_linear_minus_base candidateRva=0x-ddccc mappedPhysical=none candidateBytes=none`
- `M362_TABLE_END name=object_state`
- `M362_TABLE_BEGIN name=global_state logicalPage=20 recordBytes=1398 tableDisplacement=0x13488`
- `M362_DISPLACEMENT name=global_state object=1 interpretation=object_relative candidateRva=0x13488 mappedPhysical=0x8e41c candidateBytes=e883783400750cc745e4ffffffffe9f50300008b45e88b40348b4002`
- `M362_DISPLACEMENT name=global_state object=1 interpretation=preferred_linear_minus_base candidateRva=0x3488 mappedPhysical=0x7e41c candidateBytes=24a400000089e831db83b8a840000000751783fb1e751d31db4383fb`
- `M362_DISPLACEMENT name=global_state object=2 interpretation=object_relative candidateRva=0x13488 mappedPhysical=0x16741c candidateBytes=00000000000000000000000000000000000000000000000000000000`
- `M362_DISPLACEMENT name=global_state object=2 interpretation=preferred_linear_minus_base candidateRva=0x-dcb78 mappedPhysical=none candidateBytes=none`
- `M362_TABLE_END name=global_state`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
