# M349 — le object fixup mapping

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m349-le-object-fixup-mapping.txt`
- Original bytes: 14322
- SHA256: `a1b8b3e7d75afebe0af6d95add354cf7447c3d5b0e8d991f002a91228238e86d`
- Original encoding: `utf-16`
- Original line count: 61
- Curated lines retained: 18

## Selected diagnostic observations

- `M349_HEADER field=pageCount relative=0x14 value=0x173`
- `M349_HEADER field=entryObject relative=0x18 value=0x1`
- `M349_HEADER field=objectTable relative=0x40 value=0xc4`
- `M349_HEADER field=objectCount relative=0x44 value=0x2`
- `M349_HEADER field=objectPageTable relative=0x48 value=0xf4`
- `M349_HEADER field=fixupPageTable relative=0x68 value=0x6ca`
- `M349_HEADER field=fixupRecordTable relative=0x6c value=0xc9a`
- `M349_OBJECT index=1 tablePhysical=229976 size=0xd889a base=0x10000 flags=0x2045 firstPage=1 pageCount=217 reserved=0x0`
- `M349_OBJECT index=2 tablePhysical=230000 size=0x1821b0 base=0xf0000 flags=0x2043 firstPage=218 pageCount=154 reserved=0x0`
- `M349_LAYOUT pageTablePhysical=230024 dataStartPhysical=503700 pageSize=4096 note=LE_page_map_entry_is_4_bytes`
- `M349_PAGE_SUMMARY object=1 mapped=217`
- `M349_PAGE_SUMMARY object=2 mapped=154`
- `M349_TABLE_BEGIN runtimeAddress=0x12334`
- `M349_TABLE_END runtimeAddress=0x12334`
- `M349_TABLE_BEGIN runtimeAddress=0x13488`
- `M349_TABLE_END runtimeAddress=0x13488`
- `M349_FIXUP_TABLE pageTablePhysical=231518 recordTablePhysical=233006 entries=372 monotonic=1`
- `M349_LIMITATIONS fixupRecordsNotDecoded=1 relocationTargetsUnapplied=1 runtimeLoadBaseUnknown=1 LEPageFlagsMayRequireSpecialHandling=1 tableCandidateNotProof=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
