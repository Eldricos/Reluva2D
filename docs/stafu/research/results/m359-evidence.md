# M359 — le page address consistency

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m359-le-page-address-consistency.txt`
- Original bytes: 11338
- SHA256: `e082b8e9e22e89d0754af340c8dc172ac8738753ffe1aeed839dc7dfd9f20f6e`
- Original encoding: `utf-16`
- Original line count: 39
- Curated lines retained: 10

## Selected diagnostic observations

- `M359_LAYOUT leHeader=229780 pageSize=4096 totalPages=371 objects=2 pageMapPhysical=230024 dataRelative=273920 dataOrigin=503700`
- `M359_OBJECT number=1 base=0x10000 size=0xd889a flags=0x2045 firstPage=1 pageCount=217 lastPage=217`
- `M359_OBJECT number=2 base=0xf0000 size=0x1821b0 flags=0x2043 firstPage=218 pageCount=154 lastPage=371`
- `M359_PAGE_SUMMARY flags=0x0:371 zeroPageNumbers=0 uniquePhysicalPages=371 duplicates=0 minPhysical=0x1 maxPhysical=0x173`
- `M359_DISPATCH_BEGIN name=object_state sitePhysical=566541 displacement=0x12334`
- `M359_SITE_MAP name=object_state object=1 rva=0xf579 virtual=0x1f579 logical=16 physicalPage=16 pageOffset=0x579`
- `M359_DISPATCH_END name=object_state`
- `M359_DISPATCH_BEGIN name=global_state sitePhysical=570965 displacement=0x13488`
- `M359_SITE_MAP name=global_state object=1 rva=0x106c1 virtual=0x206c1 logical=17 physicalPage=17 pageOffset=0x6c1`
- `M359_DISPATCH_END name=global_state`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
