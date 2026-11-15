# M363 — dispatcher selector address space

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m363-dispatcher-selector-address-space.txt`
- Original bytes: 7702
- SHA256: `acff2fdc0e4fd877ab049d99b476be237836a872b50288beb7d874c395f96bb2`
- Original encoding: `utf-16`
- Original line count: 34
- Curated lines retained: 12

## Selected diagnostic observations

- `M363_LAYOUT pages=371 pageSize=4096 objectCount=2 dataOrigin=503700`
- `M363_OBJECT number=1 size=0xd889a preferredBase=0x10000 flags=0x2045 firstPage=1 pageCount=217`
- `M363_OBJECT number=2 size=0x1821b0 preferredBase=0xf0000 flags=0x2043 firstPage=218 pageCount=154`
- `M363_REFERENCE_SCAN name=object_state kind=disp32 value=0x12334 count=1 samples=rva=0xf57d/physical=0x8a511`
- `M363_REFERENCE_SCAN name=object_state kind=linearPreferred value=0x22334 count=0 samples=none`
- `M363_REFERENCE_SCAN name=object_state kind=dispMinusBase value=0x2334 count=0 samples=none`
- `M363_DISPATCH_END name=object_state`
- `M363_REFERENCE_SCAN name=global_state kind=disp32 value=0x13488 count=1 samples=rva=0x106c5/physical=0x8b659`
- `M363_REFERENCE_SCAN name=global_state kind=linearPreferred value=0x23488 count=0 samples=none`
- `M363_REFERENCE_SCAN name=global_state kind=dispMinusBase value=0x3488 count=0 samples=none`
- `M363_DISPATCH_END name=global_state`
- `M363_LIMITATIONS CSSelectorBaseNotStaticallyRecovered=1 rawDwordReferenceScanNotControlFlow=1 objectFlagsUninterpreted=1 fixupApplicationNotEmulated=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
