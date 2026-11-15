# M351 — jump table relocations

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m351-jump-table-relocations.txt`
- Original bytes: 50204
- SHA256: `dede88820a69428b5ed0552fe6b07e3a1303a1fe12dd5f4093b101cc88f1af1c`
- Original encoding: `utf-16`
- Original line count: 219
- Curated lines retained: 35

## Selected diagnostic observations

- `M351_LAYOUT objects=2 pages=371 pageSize=4096 dataStart=503700 fixupPage=231518 fixupRecords=233006`
- `M351_OBJECT index=1 base=0x10000 size=0xd889a firstPage=1 pages=217 flags=0x2045`
- `M351_OBJECT index=2 base=0xf0000 size=0x1821b0 firstPage=218 pages=154 flags=0x2043`
- `M351_TABLE name=object_state_dispatch object=1 rva=0x12334 physical=0x8d2c8 logicalPage=19 offsetInPage=0x334 fixupBytes=1159`
- `M351_CHAIN_SUMMARY table=object_state_dispatch completeChains=1 candidateIndices=[0, 1, 2, 3, 4, 5, 6]`
- `M351_STATE_BEGIN table=object_state_dispatch index=0 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=0`
- `M351_STATE_BEGIN table=object_state_dispatch index=1 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=1`
- `M351_STATE_BEGIN table=object_state_dispatch index=2 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=2`
- `M351_STATE_BEGIN table=object_state_dispatch index=3 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=3`
- `M351_STATE_BEGIN table=object_state_dispatch index=4 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=4`
- `M351_STATE_BEGIN table=object_state_dispatch index=5 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=5`
- `M351_STATE_BEGIN table=object_state_dispatch index=6 candidates=1`
- `M351_STATE_END table=object_state_dispatch index=6`
- `M351_TABLE_END name=object_state_dispatch`
- `M351_TABLE name=global_resource_dispatch object=1 rva=0x13488 physical=0x8e41c logicalPage=20 offsetInPage=0x488 fixupBytes=1398`
- `M351_CHAIN_SUMMARY table=global_resource_dispatch completeChains=1 candidateIndices=[0, 1, 2, 3, 4, 5, 6]`
- `M351_STATE_BEGIN table=global_resource_dispatch index=0 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=0`
- `M351_STATE_BEGIN table=global_resource_dispatch index=1 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=1`
- `M351_STATE_BEGIN table=global_resource_dispatch index=2 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=2`
- `M351_STATE_BEGIN table=global_resource_dispatch index=3 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=3`
- `M351_STATE_BEGIN table=global_resource_dispatch index=4 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=4`
- `M351_STATE_BEGIN table=global_resource_dispatch index=5 candidates=1`
- `M351_STATE_END table=global_resource_dispatch index=5`
- `M351_STATE_BEGIN table=global_resource_dispatch index=6 candidates=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
