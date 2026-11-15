# M382 — fixup target range audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m382-fixup-target-range-audit.txt`
- Original bytes: 6246
- SHA256: `afefface8a4d9a218a3694856fc8a0a5087403aba8f25a25774a26edf000a4b9`
- Original encoding: `utf-16`
- Original line count: 41
- Curated lines retained: 30

## Selected diagnostic observations

- `M382_HEADER pages=371 pageSize=4096 objects=2 recordBytes=258657`
- `M382_OBJECT number=1 size=0xd889a preferredBase=0x10000 firstPage=1 pageCount=217`
- `M382_OBJECT number=2 size=0x1821b0 preferredBase=0xf0000 firstPage=218 pageCount=154`
- `M382_PAGE_OBJECT page=18 encodedObject=1 count=7 inBounds=7 targetMin=0x134ec targetMax=0x1378c width16=0 width32=7`
- `M382_PAGE_OBJECT page=18 encodedObject=2 count=180 inBounds=180 targetMin=0x1376 targetMax=0x11f3d8 width16=68 width32=112`
- `M382_HYPOTHESIS page=18 model=one_based_object_shift valid=7 invalid=180`
- `M382_HYPOTHESIS page=18 model=target_linear_absolute valid=119 invalid=68`
- `M382_HYPOTHESIS page=18 model=target_linear_preferred_subtract valid=16 invalid=171`
- `M382_WIDTH page=18 width=2 count=68 encodedObjects={2: 68}`
- `M382_WIDTH page=18 width=4 count=119 encodedObjects={1: 7, 2: 112}`
- `M382_PAGE_OBJECT page=19 encodedObject=1 count=10 inBounds=10 targetMin=0x12334 targetMax=0x25e15 width16=0 width32=10`
- `M382_PAGE_OBJECT page=19 encodedObject=2 count=123 inBounds=123 targetMin=0x179e targetMax=0x11f018 width16=19 width32=104`
- `M382_HYPOTHESIS page=19 model=one_based_object_shift valid=10 invalid=123`
- `M382_HYPOTHESIS page=19 model=target_linear_absolute valid=114 invalid=19`
- `M382_HYPOTHESIS page=19 model=target_linear_preferred_subtract valid=34 invalid=99`
- `M382_WIDTH page=19 width=2 count=19 encodedObjects={2: 19}`
- `M382_WIDTH page=19 width=4 count=114 encodedObjects={1: 10, 2: 104}`
- `M382_PAGE_OBJECT page=20 encodedObject=1 count=16 inBounds=16 targetMin=0x13488 targetMax=0x16358 width16=0 width32=16`
- `M382_PAGE_OBJECT page=20 encodedObject=2 count=142 inBounds=142 targetMin=0x1940 targetMax=0x11fa18 width16=12 width32=130`
- `M382_HYPOTHESIS page=20 model=one_based_object_shift valid=16 invalid=142`
- `M382_HYPOTHESIS page=20 model=target_linear_absolute valid=146 invalid=12`
- `M382_HYPOTHESIS page=20 model=target_linear_preferred_subtract valid=82 invalid=76`
- `M382_WIDTH page=20 width=2 count=12 encodedObjects={2: 12}`
- `M382_WIDTH page=20 width=4 count=146 encodedObjects={1: 16, 2: 130}`
- `M382_PAGE_OBJECT page=21 encodedObject=2 count=25 inBounds=25 targetMin=0x1cde targetMax=0x11fa18 width16=14 width32=11`
- `M382_HYPOTHESIS page=21 model=one_based_object_shift valid=0 invalid=25`
- `M382_HYPOTHESIS page=21 model=target_linear_absolute valid=11 invalid=14`
- `M382_HYPOTHESIS page=21 model=target_linear_preferred_subtract valid=10 invalid=15`
- `M382_WIDTH page=21 width=2 count=14 encodedObjects={2: 14}`
- `M382_WIDTH page=21 width=4 count=11 encodedObjects={2: 11}`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
