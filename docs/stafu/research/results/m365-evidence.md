# M365 — relocation source reachability

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m365-relocation-source-reachability.txt`
- Original bytes: 2938
- SHA256: `16f1b4570a1cce07c55bfffdc0142419d7999afced24073768a1b6c2c320c0d6`
- Original encoding: `utf-16`
- Original line count: 15
- Curated lines retained: 10

## Selected diagnostic observations

- `M365_LAYOUT object1Size=0xd889a preferredBase=0x10000 objectPages=217`
- `M365_RAW_BRANCH_SCAN name=object_state windowStart=0x10b34 windowEnd=0x13b50 hits=4`
- `M365_RAW_BRANCH name=object_state source=0x12312 target=0x12349 kind=jmp8`
- `M365_RAW_BRANCH name=object_state source=0x1231d target=0x12341 kind=jmp8`
- `M365_RAW_BRANCH name=object_state source=0x12326 target=0x12349 kind=jmp8`
- `M365_RAW_BRANCH name=object_state source=0x1233f target=0x12349 kind=jmp8`
- `M365_RAW_BRANCH_SCAN name=global_state windowStart=0x11c88 windowEnd=0x14ca4 hits=1`
- `M365_RAW_BRANCH name=global_state source=0x1348d target=0x1349b kind=jcc8`
- `M365_CFG name=object_state seeds=window_start:0x10b34 decodedStarts=100 directEdges=12 inboundDirect=0 overlappingInstructions=0 traversalLimitHit=0`
- `M365_CFG name=global_state seeds=window_start:0x11c88 decodedStarts=210 directEdges=17 inboundDirect=0 overlappingInstructions=0 traversalLimitHit=0`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
