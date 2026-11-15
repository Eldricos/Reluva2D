# M377 — le fixup grammar audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m377-le-fixup-grammar-audit.txt`
- Original bytes: 14112
- SHA256: `056d47a46ddaafb1b23be2d0594a81c4ff3d00a2a7f3494626294842d905cd60`
- Original encoding: `utf-16`
- Original line count: 63
- Curated lines retained: 27

## Selected diagnostic observations

- `M377_HEADER pages=371 pageSize=4096 fixupBytes=258657 objects=2 indexMonotonic=1`
- `M377_OBJECT id=1 base=0x10000 size=0xd889a first=1 pages=217`
- `M377_OBJECT id=2 base=0xf0000 size=0x1821b0 first=218 pages=154`
- `M377_GRAMMAR logical=18 mode=source32 consumed=9/1547 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=18 mode=target32_always consumed=11/1547 records=1 complete=0 error=unknown_c2_0f`
- `M377_GRAMMAR logical=18 mode=target16_always consumed=23/1547 records=3 complete=0 error=unknown_09_00`
- `M377_GRAMMAR logical=18 mode=source8 consumed=6/1547 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=18 mode=target_is_additive consumed=13/1547 records=1 complete=0 error=unknown_02_76`
- `M377_COUNTS logical=18 sourceTypes={7: 187} targetFlags={0: 68, 16: 119} recordSizes={7: 68, 9: 119} sourceUnique=187 records=187`
- `M377_GRAMMAR logical=19 mode=source32 consumed=11/1159 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=19 mode=target32_always consumed=279/1159 records=30 complete=0 error=target_out_of_object`
- `M377_GRAMMAR logical=19 mode=target16_always consumed=9/1159 records=1 complete=0 error=unknown_09_00`
- `M377_GRAMMAR logical=19 mode=source8 consumed=8/1159 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=19 mode=target_is_additive consumed=15/1159 records=1 complete=0 error=unknown_02_7c`
- `M377_COUNTS logical=19 sourceTypes={7: 133} targetFlags={16: 114, 0: 19} recordSizes={9: 114, 7: 19} sourceUnique=133 records=133`
- `M377_GRAMMAR logical=20 mode=source32 consumed=11/1398 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=20 mode=target32_always consumed=38/1398 records=4 complete=0 error=unknown_7f_0a`
- `M377_GRAMMAR logical=20 mode=target16_always consumed=9/1398 records=1 complete=0 error=unknown_11_00`
- `M377_GRAMMAR logical=20 mode=source8 consumed=8/1398 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=20 mode=target_is_additive consumed=15/1398 records=1 complete=0 error=unknown_02_e0`
- `M377_COUNTS logical=20 sourceTypes={7: 158} targetFlags={16: 146, 0: 12} recordSizes={9: 146, 7: 12} sourceUnique=158 records=158`
- `M377_GRAMMAR logical=21 mode=source32 consumed=11/197 records=0 complete=0 error=invalid_object`
- `M377_GRAMMAR logical=21 mode=target32_always consumed=29/197 records=3 complete=0 error=unknown_84_01`
- `M377_GRAMMAR logical=21 mode=target16_always consumed=9/197 records=1 complete=0 error=unknown_11_00`
- `M377_GRAMMAR logical=21 mode=source8 consumed=8/197 records=0 complete=0 error=target_out_of_object`
- `M377_GRAMMAR logical=21 mode=target_is_additive consumed=15/197 records=1 complete=0 error=unknown_02_14`
- `M377_COUNTS logical=21 sourceTypes={7: 25} targetFlags={16: 11, 0: 14} recordSizes={9: 11, 7: 14} sourceUnique=25 records=25`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
