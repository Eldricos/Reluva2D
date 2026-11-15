# M348 — resource state dispatcher

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m348-resource-state-dispatcher.txt`
- Original bytes: 37614
- SHA256: `8e6f8aab3e3a67ba75ee6e6b4c19303c6bca8f52f030b0dbd33ef76f9d602f60`
- Original encoding: `utf-16`
- Original line count: 262
- Curated lines retained: 18

## Selected diagnostic observations

- `M348_LE_HEADER offset=229780 signature=4c45 objectTableOffset=196 objectCount=2 fixupPageTableOffset=1738 fixupRecordTableOffset=3226`
- `M348_CFG name=object_state_dispatch entry=566260 lower=566260 upper=566552 decoded=44 edges=5 failures=0 bounded=1`
- `M348_CALL name=object_state_dispatch site=566309 target=589648`
- `M348_CALL name=object_state_dispatch site=566397 target=563123`
- `M348_CALL name=object_state_dispatch site=566405 target=570696`
- `M348_CFG_END name=object_state_dispatch`
- `M348_CFG name=global_resource_dispatch entry=570696 lower=570696 upper=571450 decoded=41 edges=5 failures=0 bounded=1`
- `M348_CALL name=global_resource_dispatch site=570723 target=678926`
- `M348_CFG_END name=global_resource_dispatch`
- `M348_ANCHOR name=object_position_block start=566382 end=566415 entryAlignmentFromPriorTrace=1`
- `M348_ANCHOR_END name=object_position_block`
- `M348_ANCHOR name=object_reset_block start=566415 end=566470 entryAlignmentFromPriorTrace=1`
- `M348_ANCHOR_END name=object_reset_block`
- `M348_ANCHOR name=object_mode_dispatch start=566514 end=566552 entryAlignmentFromPriorTrace=1`
- `M348_ANCHOR_END name=object_mode_dispatch`
- `M348_ANCHOR name=global_state_loop start=570927 end=571110 entryAlignmentFromPriorTrace=1`
- `M348_ANCHOR_END name=global_state_loop`
- `M348_LIMITATIONS static=1 physicalOffsets=1 LEFixupsUnapplied=1 jumpTableTargetsUnverified=1 candidateMappingsHypothetical=1 indirectCFGUnresolved=1 boundsHeuristic=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
