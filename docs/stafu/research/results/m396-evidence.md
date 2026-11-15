# M396 — export group boundaries

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m396-export-group-boundaries.txt`
- Original bytes: 14946
- SHA256: `04e51f74d6464c280c07705e8786ee752c20ac8b9143946d214b71688b5d23dd`
- Original encoding: `utf-16`
- Original line count: 78
- Curated lines retained: 16

## Selected diagnostic observations

- `M396_MODULE name=LINEXE_LOADER descriptor=0x32a78 expectedTable=0x329f0`
- `M396_POINTER module=LINEXE_LOADER file=0x32a7c value=0x00a81e6e target=0x329e2 string='LINEXE_LOADER' entryIndex=-`
- `M396_POINTER module=LINEXE_LOADER file=0x32a84 value=0x00a81e7c target=0x329f0 string=None entryIndex=0`
- `M396_POINTER module=LINEXE_LOADER file=0x32a8a value=0x00a81e8c target=0x32a00 string=None entryIndex=2`
- `M396_HYPOTHESIS module=LINEXE_LOADER name='LINEXE_LOADER' namePointer=0x00a81e6e versions=1,2 group0=0x329f0 group1=0x32a00 countField=15 gapBytes=16 firstGroupCount=2 inferredTotal=17`
- `M396_EXPORTS module=LINEXE_LOADER start=0x329f0 end=0x32a78 count=17`
- `M396_EXPORT module=LINEXE_LOADER index=8 file=0x32a30 namePtr=0x00a81ddb nameFile=0x3294f name='GETLOADTABLE' codePtr=0x008851df codeHigh16=0x0088`
- `M396_EXPORT module=LINEXE_LOADER index=16 file=0x32a70 namePtr=0x00a81e5c nameFile=0x329d0 name='XXXGET_OBJECT_MAP' codePtr=0x00883fe7 codeHigh16=0x0088`
- `M396_MODULE name=D32_INT31 descriptor=0x323ec expectedTable=0x323d4`
- `M396_POINTER module=D32_INT31 file=0x323f0 value=0x00a8184c target=0x323c0 string='D32_INT31' entryIndex=-`
- `M396_POINTER module=D32_INT31 file=0x323f8 value=0x00a81860 target=0x323d4 string=None entryIndex=0`
- `M396_POINTER module=D32_INT31 file=0x323fe value=0x00a81870 target=0x323e4 string=None entryIndex=2`
- `M396_HYPOTHESIS module=D32_INT31 name='D32_INT31' namePointer=0x00a8184c versions=1,2 group0=0x323d4 group1=0x323e4 countField=1 gapBytes=16 firstGroupCount=2 inferredTotal=3`
- `M396_EXPORTS module=D32_INT31 start=0x323d4 end=0x323ec count=3`
- `M396_RUN region=linexe start=0x329f0 end=0x32a78 count=17 names=['INIT', 'DEINIT', 'LOADER', 'UNLOAD', 'FREEMAP', 'REL', 'UNREL', 'CANLOAD', 'GETLOADTABLE', 'GETLOADNAME', 'GETERRSTR', 'LINEXE_LOADMODULE', 'LINEXE_GETPROCADDR', 'LINEXE_GETFLATREGS', 'LINEXE_FREEMODULE', 'LINEXE_GETMODHANDLE', 'XXXGET_OBJECT_MAP']`
- `M396_RUN region=int31 start=0x323d4 end=0x323ec count=3 names=['INIT', 'DEINIT', 'INT31']`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
