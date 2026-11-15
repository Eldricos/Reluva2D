# M399 — loader name array xrefs

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m399-loader-name-array-xrefs.txt`
- Original bytes: 9016
- SHA256: `16a603907d71a0255cdaf891be4b82c3a46d6695af4b77757245713c5869102c`
- Original encoding: `utf-16`
- Original line count: 43
- Curated lines retained: 22

## Selected diagnostic observations

- `M399_ARRAY name=process_manager file=0x31d64 affine=0x00a811f0 count=5`
- `M399_ABSOLUTE_REFS name=process_manager kind=linear32 value=0x00a811f0 count=0 offsets=`
- `M399_ABSOLUTE_REFS name=process_manager kind=file32 value=0x00031d64 count=0 offsets=`
- `M399_REL32_CANDIDATES name=process_manager target=0x31d64 count=0`
- `M399_REL32_CANDIDATES name=process_manager target=0x31d68 count=0`
- `M399_REL32_CANDIDATES name=process_manager target=0x31d6c count=0`
- `M399_REL32_CANDIDATES name=process_manager target=0x31d70 count=0`
- `M399_REL32_CANDIDATES name=process_manager target=0x31d74 count=0`
- `M399_LOW16 name=process_manager value=0x11f0 count=10 first=0x4611,0x463d,0x46ab,0x5d00,0x5d7a,0x61b8,0x6243,0x24c41,0x24c54,0x24c76`
- `M399_ELEMENT_REFS name=process_manager index=0 value=0x00a81135 count=1 offsets=0x31d64`
- `M399_ELEMENT_REFS name=process_manager index=4 value=0x00a8116b count=1 offsets=0x31d74`
- `M399_ARRAY name=loader_names file=0x338fe affine=0x00a82d8a count=4`
- `M399_ABSOLUTE_REFS name=loader_names kind=linear32 value=0x00a82d8a count=0 offsets=`
- `M399_ABSOLUTE_REFS name=loader_names kind=file32 value=0x000338fe count=0 offsets=`
- `M399_REL32_CANDIDATES name=loader_names target=0x338fe count=0`
- `M399_REL32_CANDIDATES name=loader_names target=0x33902 count=0`
- `M399_REL32_CANDIDATES name=loader_names target=0x33906 count=0`
- `M399_REL32_CANDIDATES name=loader_names target=0x3390a count=0`
- `M399_LOW16 name=loader_names value=0x2d8a count=1 first=0x2d6bb`
- `M399_ELEMENT_REFS name=loader_names index=0 value=0x00a82d36 count=1 offsets=0x338fe`
- `M399_ELEMENT_REFS name=loader_names index=3 value=0x00a82d57 count=1 offsets=0x3390a`
- `M399_LIMITATIONS staticBytesOnly=1 noInstructionBoundaryProof=1 rel32FileOffsetHeuristic=1 runtimeAddressingUnknown=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
