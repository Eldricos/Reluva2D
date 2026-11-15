# M380 — function anchored cfg

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m380-function-anchored-cfg.txt`
- Original bytes: 19128
- SHA256: `d9b144ffd353dd94753166c2a46ca29d02d4518bcb42dbe4491f3a1505c28d5d`
- Original encoding: `utf-16`
- Original line count: 76
- Curated lines retained: 12

## Selected diagnostic observations

- `M380_REACH name=object_state anchoredCandidatesReachingJump=2`
- `M380_ANCHORED_PATH name=object_state anchor=0xf460 kind=push_ebx_push_esi instructions=44 branches=7 jumpDecoded=1`
- `M380_ANCHORED_PATH name=object_state anchor=0xf461 kind=push_esi_push_edi instructions=43 branches=7 jumpDecoded=1`
- `M380_TABLE_ANCHORS name=object_state candidates=1 nearest=0x1213c`
- `M380_TABLE_REACH name=object_state anchoredCandidatesEnteringTable=0`
- `M380_WARNING name=object_state noFunctionPrologueAnchorReachesTable; this does not prove table is data`
- `M380_REACH name=global_state anchoredCandidatesReachingJump=2`
- `M380_ANCHORED_PATH name=global_state anchor=0x105b4 kind=push_ebx_push_esi instructions=41 branches=5 jumpDecoded=1`
- `M380_ANCHORED_PATH name=global_state anchor=0x105b5 kind=push_esi_push_edi instructions=40 branches=5 jumpDecoded=1`
- `M380_TABLE_ANCHORS name=global_state candidates=3 nearest=0x132b2,0x132cf,0x1346b`
- `M380_TABLE_REACH name=global_state anchoredCandidatesEnteringTable=1`
- `M380_TABLE_PATH name=global_state anchor=0x1346b kind=push_ebp_mov_ebp_esp decodedTableStarts=0x13489,0x1348d,0x1348f,0x13496,0x1349b,0x1349e,0x134a1 inbound=0x1348d->0x1349b:jne`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
