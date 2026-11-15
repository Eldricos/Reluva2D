# M343 — resource io wrappers

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m343-resource-io-wrappers.txt`
- Original bytes: 72152
- SHA256: `27c9e54fd6d88d35f7268969e48f3d4ea6d47a154dfcb1f4c8d373f7c87197c6`
- Original encoding: `utf-16`
- Original line count: 740
- Curated lines retained: 23

## Selected diagnostic observations

- `M343_REGION name=delegate_517688 entry=517688 boundary=518400 decoded=29 edges=4 undecodable=0`
- `M343_REGION name=candidate_read_563062 entry=563062 boundary=563123 decoded=27 edges=0 undecodable=0`
- `M343_MEM region=candidate_read_563062 site=563074 operands=op1:ebp:-:+0x1c:size4`
- `M343_MEM region=candidate_read_563062 site=563078 operands=op1:ebp:-:+0x18:size4`
- `M343_MEM region=candidate_read_563062 site=563082 operands=op1:ebp:-:+0x14:size4`
- `M343_CALL region=candidate_read_563062 site=563092 target=517764`
- `M343_MEM region=candidate_read_563062 site=563100 operands=op0:ebp:-:-0x4:size4`
- `M343_MEM region=candidate_read_563062 site=563103 operands=op0:ebp:-:-0x4:size4`
- `M343_MEM region=candidate_read_563062 site=563107 operands=op1:ebp:-:-0x4:size4`
- `M343_MEM region=candidate_read_563062 site=563110 operands=op0:ebp:-:-0x8:size4`
- `M343_MEM region=candidate_read_563062 site=563113 operands=op1:ebp:-:-0x8:size4`
- `M343_REGION_END name=candidate_read_563062`
- `M343_REGION name=post_seek_570696 entry=570696 boundary=571200 decoded=41 edges=5 undecodable=0`
- `M343_CALL region=post_seek_570696 site=570723 target=678926`
- `M343_REGION name=seek_wrapper_563123 entry=563123 boundary=563166 decoded=21 edges=0 undecodable=0`
- `M343_CALL region=seek_wrapper_563123 site=563153 target=517688`
- `M343_ANCHOR start=519076 end=519115 note=known-instruction-boundary`
- `M343_ANCHOR start=519142 end=519180 note=known-instruction-boundary`
- `M343_ANCHOR start=566382 end=566415 note=known-instruction-boundary`
- `M343_RAW_CALLERS target=517688 count=1 sites=563153`
- `M343_RAW_CALLERS target=563062 count=24 sites=519037,519100,519166,519249,519311,519373,519435,519497,519693,519819,565466,565684,568294,568454,570032,757398,757416,757495,784366,784408,784426,784518,1059361,1059620`
- `M343_RAW_CALLERS target=570696 count=38 sites=509317,509588,513749,513864,514047,514359,514411,517780,518097,541572,541793,542401,546379,549721,552020,558778,566405,567477,567482,567999,572396,587704,670930,742411,816789,839442,839809,839855,840075,844416,844592,857865,859009,907101,1040612,1040793,1040828,1091712`
- `M343_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 functionBoundariesHeuristic=1 indirectBranchesUnresolved=1 rawCallersMayBeData=1 readSeekClassificationUnverified=1 decoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
