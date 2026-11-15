# M443 — stream transfer provenance

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m443-stream-transfer-provenance.txt`
- Original bytes: 71864
- SHA256: `1a37881c5f1a5d2f3897718714a92abf7a953060de3f97ba7acb9eb403c8abcf`
- Original encoding: `utf-16`
- Original line count: 455
- Curated lines retained: 5

## Selected diagnostic observations

- `M443_DIRECT_CALLS name=buffered_reader calls=0x899a9->0x7e684,0x899c6->0x8ff50,0x899df->0x8ff50,0x899e7->0x7af04,0x899f5->0x7c2d0,0x89a7e->0x7e684,0x89a9b->0x8ff50,0x89ab4->0x8ff50,0x89abc->0x7af04,0x89aca->0x7c2d0`
- `M443_RAW_CALLERS target=transfer count=3 sites=0x89794,0x899a9,0x89a7e`
- `M443_RAW_CALLERS target=buffered_reader count=3 sites=0x7bf23,0x7bff5,0x7c016`
- `M443_CAUTION raw E8 matches and linear pre-call alignments are hypotheses, not runtime execution proof. Function boundaries are provisional. No SPR decoding is asserted.`
- `M443_DONE readOnly=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
