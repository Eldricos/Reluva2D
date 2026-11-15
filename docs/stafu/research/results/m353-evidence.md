# M353 — le fixup audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m353-le-fixup-audit.txt`
- Original bytes: 17392
- SHA256: `48fbf5afdb08c057c916c64cb7ceec18e4a2124021a66f9a5ed34ce5531fc06e`
- Original encoding: `utf-16`
- Original line count: 64
- Curated lines retained: 17

## Selected diagnostic observations

- `M353_OBJECT number=1 size=0xd889a base=0x10000 flags=0x2045 first=1 pages=217`
- `M353_OBJECT number=2 size=0x1821b0 base=0xf0000 flags=0x2043 first=218 pages=154`
- `M353_LAYOUT pageSize=4096 pageMap=230024 fixupIndex=231518 fixupRecords=233006 dataOrigin=503700 totalPages=371`
- `M353_FIXUP_INDEX entries=372 totalBytes=258657 monotonic=1`
- `M353_TABLE_BEGIN name=object_state displacement=0x12334 firstState=1`
- `M353_INTERPRETATION name=object_state obj=1 mode=object_relative rva=0x12334 physical=0x8d2c8`
- `M353_INTERPRETATION name=object_state obj=1 mode=virtual_minus_base rva=0x2334 physical=0x7d2c8`
- `M353_INTERPRETATION name=object_state obj=2 mode=object_relative rva=0x12334 physical=0x1662c8`
- `M353_INTERPRETATION name=object_state obj=2 mode=virtual_minus_base rva=0x-ddccc physical=none`
- `M353_TABLE_END name=object_state`
- `M353_TABLE_BEGIN name=global_state displacement=0x13488 firstState=0`
- `M353_INTERPRETATION name=global_state obj=1 mode=object_relative rva=0x13488 physical=0x8e41c`
- `M353_INTERPRETATION name=global_state obj=1 mode=virtual_minus_base rva=0x3488 physical=0x7e41c`
- `M353_INTERPRETATION name=global_state obj=2 mode=object_relative rva=0x13488 physical=0x16741c`
- `M353_INTERPRETATION name=global_state obj=2 mode=virtual_minus_base rva=0x-dcb78 physical=none`
- `M353_TABLE_END name=global_state`
- `M353_LIMITATIONS partialFixupParser=1 sourceRecordBoundariesNotFullyProven=1 codeValidityNotEstablished=1 displacementSegmentSemanticsUnverified=1 noHandlersConfirmed=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
