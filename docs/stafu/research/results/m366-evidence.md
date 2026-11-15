# M366 — local call seeded cfg

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m366-local-call-seeded-cfg.txt`
- Original bytes: 19420
- SHA256: `cadf4a5fd45feb7c43cc355f6a905eb505e8b27bfd57d505185058ff98580733`
- Original encoding: `utf-16`
- Original line count: 99
- Curated lines retained: 15

## Selected diagnostic observations

- `M366_LAYOUT object1Size=0xd889a radius=0x2000`
- `M366_CALL_SEEDS name=object_state linearSweepCalls=21 distinctTargets=10 prologueTargets=0`
- `M366_RAW_SOURCE name=object_state source=0x12312 standalone=jmp_0x12349 target=0x12349`
- `M366_RAW_SOURCE name=object_state source=0x1231d standalone=jmp_0x12341 target=0x12341`
- `M366_RAW_SOURCE name=object_state source=0x12326 standalone=jmp_0x12349 target=0x12349`
- `M366_RAW_SOURCE name=object_state source=0x1233f standalone=jmp_0x12349 target=0x12349`
- `M366_CFG name=object_state seedClass=prologue_call_targets seeds=0 visited=0 directEdges=0 inbound=0 overlaps=0 indirect=0 limitHit=0`
- `M366_CFG name=object_state seedClass=all_direct_call_targets seeds=10 visited=999 directEdges=119 inbound=0 overlaps=0 indirect=2 limitHit=0`
- `M366_REGION_END name=object_state`
- `M366_CALL_SEEDS name=global_state linearSweepCalls=41 distinctTargets=15 prologueTargets=0`
- `M366_RAW_SOURCE name=global_state source=0x1348d standalone=jne_0x1349b target=0x1349b`
- `M366_CFG name=global_state seedClass=prologue_call_targets seeds=0 visited=0 directEdges=0 inbound=0 overlaps=0 indirect=0 limitHit=0`
- `M366_CFG name=global_state seedClass=all_direct_call_targets seeds=15 visited=836 directEdges=97 inbound=0 overlaps=0 indirect=1 limitHit=0`
- `M366_REGION_END name=global_state`
- `M366_LIMITATIONS callSeedsDerivedFromLinearSweep=1 noVerifiedRuntimeFunctionEntries=1 noIndirectTargetResolution=1 noRuntimeReachabilityProof=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
