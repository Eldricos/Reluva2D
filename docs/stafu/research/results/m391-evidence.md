# M391 — dos mz segment mapping

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m391-dos-mz-segment-mapping.txt`
- Original bytes: 25234
- SHA256: `4225f1730afa99ddef8d40cac847d77af1e7588e14a3780094f78a30bc623e84`
- Original encoding: `utf-16`
- Original line count: 91
- Curated lines retained: 8

## Selected diagnostic observations

- `M391_MZ declaredBytes=62580 headerBytes=512 loadImageBytes=62068 relocations=78 relocTable=0x1e cs=0x1bd ip=0x2382 ss=0x121 sp=0x340 entryFile=0x4152`
- `M391_MZ_BOUNDARY relocTableFitsHeader=1 declaredFitsFile=1 trailingBeforeLE=167200`
- `M391_RELOC_SUMMARY count=78 invalid=0 uniqueSegments=5 minImageOffset=0x20 maxImageOffset=0xe545`
- `M391_RELOC_REMAINING count=38 notPrinted=1`
- `M391_PTR index=8 name=GETLOADTABLE value=0x008851df segment=0x0088 offset=0x51df mzRelativeFile=0x5c5f mzRelativeInImage=1 rawRealModeLinear=0x5a5f lowWordPlusHeader=0x53df lowWordInImage=1`
- `M391_PTR index=16 name=XXXGET_OBJECT_MAP value=0x00883fe7 segment=0x0088 offset=0x3fe7 mzRelativeFile=0x4a67 mzRelativeInImage=1 rawRealModeLinear=0x4867 lowWordPlusHeader=0x41e7 lowWordInImage=1`
- `M391_RELOC_0088 originalRelocatedWords=0 note=does_not_prove_runtime_selector`
- `M391_LIMITATIONS loadSegmentUnknown=1 selectorDescriptorsUnknown=1 pointerTableSemanticsUnverified=1 noRuntimeExecution=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
