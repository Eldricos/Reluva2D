# M424 — object2 table consumers

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m424-object2-table-consumers.txt`
- Original bytes: 8476
- SHA256: `4cd25e7f5ad5e7b12591331382cbb53d003a44adaf14bb8ad718692c9fe9b356`
- Original encoding: `utf-16`
- Original line count: 42
- Curated lines retained: 19

## Selected diagnostic observations

- `M424_OBJECT index=1 base=0x10000 size=0xd889a firstPage=1 pages=217 fileStart=0x7af94 fileEnd=0x15382e assumedLinear=1`
- `M424_OBJECT index=2 base=0xf0000 size=0x1821b0 firstPage=218 pages=154 fileStart=0x153f94 fileEnd=0x1ea587 assumedLinear=1`
- `M424_FIXUP_SECTIONS pageTable=0x3885e recordTable=0x38e2e headerOffsets=0x6ca/0xc9a`
- `M424_TARGET name=table_start file=0x164554 object=2 rel=0x105c0 linear=0x1005c0`
- `M424_RAW32 target=table_start encoding=object_relative value=0x105c0 count=1`
- `M424_RAW32_HIT target=table_start encoding=object_relative at=0x100a17 object1Rel=0x85a83 context=008b85e8fdffff8945c0817dc0050100007d0b8b45c08985e4fdffff`
- `M424_RAW32 target=table_start encoding=linear_address value=0x1005c0 count=0`
- `M424_TARGET name=table_end file=0x16501c object=2 rel=0x11088 linear=0x101088`
- `M424_RAW32 target=table_end encoding=object_relative value=0x11088 count=0`
- `M424_RAW32 target=table_end encoding=linear_address value=0x101088 count=0`
- `M424_TARGET name=bridge_primary file=0x16486c object=2 rel=0x108d8 linear=0x1008d8`
- `M424_RAW32 target=bridge_primary encoding=object_relative value=0x108d8 count=0`
- `M424_RAW32 target=bridge_primary encoding=linear_address value=0x1008d8 count=0`
- `M424_TARGET name=bridge_secondary file=0x164ed8 object=2 rel=0x10f44 linear=0x100f44`
- `M424_RAW32 target=bridge_secondary encoding=object_relative value=0x10f44 count=0`
- `M424_RAW32 target=bridge_secondary encoding=linear_address value=0x100f44 count=0`
- `M424_TARGET name=door file=0x164f14 object=2 rel=0x10f80 linear=0x100f80`
- `M424_RAW32 target=door encoding=object_relative value=0x10f80 count=0`
- `M424_RAW32 target=door encoding=linear_address value=0x100f80 count=0`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
