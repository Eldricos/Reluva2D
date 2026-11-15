# M350 — corrected le page mapping

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m350-corrected-le-page-mapping.txt`
- Original bytes: 30846
- SHA256: `e0680a93d5edabe902d022089ea4aef23dd1cd4835fd98da0a0866168cd4500a`
- Original encoding: `utf-16`
- Original line count: 117
- Curated lines retained: 16

## Selected diagnostic observations

- `M350_LAYOUT objectCount=2 pageSize=4096 pageTablePhysical=230024 dataPagesHeaderValue=0x42e00 dataStartPhysical=503700 fixupPagePhysical=231518 fixupRecordPhysical=233006 totalPages=371`
- `M350_OBJECT index=1 size=0xd889a base=0x10000 flags=0x2045 firstPage=1 pageCount=217`
- `M350_OBJECT index=2 size=0x1821b0 base=0xf0000 flags=0x2043 firstPage=218 pageCount=154`
- `M350_TABLE_BEGIN address=0x12334`
- `M350_TABLE_LOCATION address=0x12334 object=1 mode=absolute_minus_base rva=0x2334 physical=0x7d2c8`
- `M350_TABLE_LOCATION address=0x12334 object=1 mode=object_relative rva=0x12334 physical=0x8d2c8`
- `M350_TABLE_LOCATION address=0x12334 object=2 mode=absolute_minus_base rva=0x-ddccc physical=none`
- `M350_TABLE_LOCATION address=0x12334 object=2 mode=object_relative rva=0x12334 physical=0x1662c8`
- `M350_TABLE_END address=0x12334`
- `M350_TABLE_BEGIN address=0x13488`
- `M350_TABLE_LOCATION address=0x13488 object=1 mode=absolute_minus_base rva=0x3488 physical=0x7e41c`
- `M350_TABLE_LOCATION address=0x13488 object=1 mode=object_relative rva=0x13488 physical=0x8e41c`
- `M350_TABLE_LOCATION address=0x13488 object=2 mode=absolute_minus_base rva=0x-dcb78 physical=none`
- `M350_TABLE_LOCATION address=0x13488 object=2 mode=object_relative rva=0x13488 physical=0x16741c`
- `M350_TABLE_END address=0x13488`
- `M350_LIMITATIONS noFixupsApplied=1 fixupCandidateBoundariesUnverified=1 runtimeAddressAssumptionsUnverified=1 rawTargetNotProof=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
