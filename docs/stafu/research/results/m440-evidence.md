# M440 — dos read caller provenance

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m440-dos-read-caller-provenance.txt`
- Original bytes: 141596
- SHA256: `ba9eb67516092b42705dabd51cac57354739ad457dcc7cc2a0c4a48c23dc214e`
- Original encoding: `utf-16`
- Original line count: 1063
- Curated lines retained: 35

## Selected diagnostic observations

- `M440_READ_TARGET file=0x11acd0 rawCallers=23`
- `M440_NEARBY_DOS_CALLS site=0x7cfa8 radius=512 count=1`
- `M440_NEARBY site=0x7cfa8 callsite=0x7cfa8 target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY_DOS_CALLS site=0x7d61e radius=512 count=3`
- `M440_NEARBY site=0x7d61e callsite=0x7d61e target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY site=0x7d61e callsite=0x7d6e7 target=0x11ae03 name=dos_seek delta=+201`
- `M440_NEARBY site=0x7d61e callsite=0x7d7f3 target=0x11acd0 name=dos_read delta=+469`
- `M440_NEARBY_DOS_CALLS site=0x7d7f3 radius=512 count=5`
- `M440_NEARBY site=0x7d7f3 callsite=0x7d61e target=0x11acd0 name=dos_read delta=-469`
- `M440_NEARBY site=0x7d7f3 callsite=0x7d6e7 target=0x11ae03 name=dos_seek delta=-268`
- `M440_NEARBY site=0x7d7f3 callsite=0x7d7f3 target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY site=0x7d7f3 callsite=0x7d897 target=0x11acd0 name=dos_read delta=+164`
- `M440_NEARBY site=0x7d7f3 callsite=0x7d951 target=0x11acd0 name=dos_read delta=+350`
- `M440_NEARBY_DOS_CALLS site=0x7d897 radius=512 count=6`
- `M440_NEARBY site=0x7d897 callsite=0x7d6e7 target=0x11ae03 name=dos_seek delta=-432`
- `M440_NEARBY site=0x7d897 callsite=0x7d7f3 target=0x11acd0 name=dos_read delta=-164`
- `M440_NEARBY site=0x7d897 callsite=0x7d897 target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY site=0x7d897 callsite=0x7d951 target=0x11acd0 name=dos_read delta=+186`
- `M440_NEARBY site=0x7d897 callsite=0x7da21 target=0x11ae03 name=dos_seek delta=+394`
- `M440_NEARBY site=0x7d897 callsite=0x7da3d target=0x11acd0 name=dos_read delta=+422`
- `M440_NEARBY_DOS_CALLS site=0x7d951 radius=512 count=7`
- `M440_NEARBY site=0x7d951 callsite=0x7d7f3 target=0x11acd0 name=dos_read delta=-350`
- `M440_NEARBY site=0x7d951 callsite=0x7d897 target=0x11acd0 name=dos_read delta=-186`
- `M440_NEARBY site=0x7d951 callsite=0x7d951 target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY site=0x7d951 callsite=0x7da21 target=0x11ae03 name=dos_seek delta=+208`
- `M440_NEARBY site=0x7d951 callsite=0x7da3d target=0x11acd0 name=dos_read delta=+236`
- `M440_NEARBY site=0x7d951 callsite=0x7db0b target=0x11ae03 name=dos_seek delta=+442`
- `M440_NEARBY site=0x7d951 callsite=0x7db26 target=0x11acd0 name=dos_read delta=+469`
- `M440_NEARBY_DOS_CALLS site=0x7da3d radius=512 count=6`
- `M440_NEARBY site=0x7da3d callsite=0x7d897 target=0x11acd0 name=dos_read delta=-422`
- `M440_NEARBY site=0x7da3d callsite=0x7d951 target=0x11acd0 name=dos_read delta=-236`
- `M440_NEARBY site=0x7da3d callsite=0x7da21 target=0x11ae03 name=dos_seek delta=-28`
- `M440_NEARBY site=0x7da3d callsite=0x7da3d target=0x11acd0 name=dos_read delta=+0`
- `M440_NEARBY site=0x7da3d callsite=0x7db0b target=0x11ae03 name=dos_seek delta=+206`
- `M440_NEARBY site=0x7da3d callsite=0x7db26 target=0x11acd0 name=dos_read delta=+233`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
