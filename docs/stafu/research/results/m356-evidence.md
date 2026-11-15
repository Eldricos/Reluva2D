# M356 — le segment address resolution

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m356-le-segment-address-resolution.txt`
- Original bytes: 26134
- SHA256: `0cc607d1383a752a654a6ed259268ec044078e08705ecdb8b73fd6b8b6815765`
- Original encoding: `utf-16`
- Original line count: 107
- Curated lines retained: 34

## Selected diagnostic observations

- `M356_LAYOUT pages=371 pageSize=4096 dataOrigin=503700 fixupRecords=233006`
- `M356_OBJECT number=1 base=0x10000 size=0xd889a flags=0x2045 firstPage=1 count=217`
- `M356_OBJECT number=2 base=0xf0000 size=0x1821b0 flags=0x2043 firstPage=218 count=154`
- `M356_DISPATCH_BEGIN name=object_state physical=566541 displacement=0x12334`
- `M356_JUMP_MAP object=1 rva=0xf579 virtual=0x1f579`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=zero csBase=0x0 linear=0x12334 targetObject=1 targetRva=0x2334 targetPhysical=0x7d2c8`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=zero csBase=0x0 linear=0x12334 targetObject=2 targetRva=0x-ddccc targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=own_object_base csBase=0x10000 linear=0x22334 targetObject=1 targetRva=0x12334 targetPhysical=0x8d2c8`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=own_object_base csBase=0x10000 linear=0x22334 targetObject=2 targetRva=0x-cdccc targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=object1_base csBase=0x10000 linear=0x22334 targetObject=1 targetRva=0x12334 targetPhysical=0x8d2c8`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=object1_base csBase=0x10000 linear=0x22334 targetObject=2 targetRva=0x-cdccc targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=object2_base csBase=0xf0000 linear=0x102334 targetObject=1 targetRva=0xf2334 targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=object_state codeObject=1 csBaseMode=object2_base csBase=0xf0000 linear=0x102334 targetObject=2 targetRva=0x12334 targetPhysical=0x1662c8`
- `M356_TABLE_LOCATION name=object_state object=1 mode=object_relative rva=0x12334 physical=0x8d2c8`
- `M356_TABLE_LOCATION name=object_state object=1 mode=virtual_minus_base rva=0x2334 physical=0x7d2c8`
- `M356_TABLE_LOCATION name=object_state object=2 mode=object_relative rva=0x12334 physical=0x1662c8`
- `M356_TABLE_LOCATION name=object_state object=2 mode=virtual_minus_base rva=0x-ddccc physical=none`
- `M356_DISPATCH_END name=object_state`
- `M356_DISPATCH_BEGIN name=global_state physical=570965 displacement=0x13488`
- `M356_JUMP_MAP object=1 rva=0x106c1 virtual=0x206c1`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=zero csBase=0x0 linear=0x13488 targetObject=1 targetRva=0x3488 targetPhysical=0x7e41c`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=zero csBase=0x0 linear=0x13488 targetObject=2 targetRva=0x-dcb78 targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=own_object_base csBase=0x10000 linear=0x23488 targetObject=1 targetRva=0x13488 targetPhysical=0x8e41c`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=own_object_base csBase=0x10000 linear=0x23488 targetObject=2 targetRva=0x-ccb78 targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=object1_base csBase=0x10000 linear=0x23488 targetObject=1 targetRva=0x13488 targetPhysical=0x8e41c`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=object1_base csBase=0x10000 linear=0x23488 targetObject=2 targetRva=0x-ccb78 targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=object2_base csBase=0xf0000 linear=0x103488 targetObject=1 targetRva=0xf3488 targetPhysical=none`
- `M356_SEGMENT_HYPOTHESIS name=global_state codeObject=1 csBaseMode=object2_base csBase=0xf0000 linear=0x103488 targetObject=2 targetRva=0x13488 targetPhysical=0x16741c`
- `M356_TABLE_LOCATION name=global_state object=1 mode=object_relative rva=0x13488 physical=0x8e41c`
- `M356_TABLE_LOCATION name=global_state object=1 mode=virtual_minus_base rva=0x3488 physical=0x7e41c`
- `M356_TABLE_LOCATION name=global_state object=2 mode=object_relative rva=0x13488 physical=0x16741c`
- `M356_TABLE_LOCATION name=global_state object=2 mode=virtual_minus_base rva=0x-dcb78 physical=none`
- `M356_DISPATCH_END name=global_state`
- `M356_LIMITATIONS csBaseNotKnown=1 selectorDescriptorsNotDecoded=1 completeFixupGrammarNotParsed=1 noRuntimeExecution=1 candidateTargetsNotValidated=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
