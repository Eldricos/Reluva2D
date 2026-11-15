# M386 — dos4gw loader region audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m386-dos4gw-loader-region-audit.txt`
- Original bytes: 32928
- SHA256: `b40b4a7caae510201ad96704557837ea255a40ac1f039825135b9904a4b355f0`
- Original encoding: `utf-16`
- Original line count: 194
- Curated lines retained: 10

## Selected diagnostic observations

- `M386_MZ declaredImageBytes=62580 entryFile=0x4152 initialCS=0x1bd initialIP=0x2382 e_lfanew=0x9b40000 note=e_lfanew_may_not_point_to_LE`
- `M386_MARKER name='DOS4GLOADER' count=4 offsets=0x1da4f,0x1da5b,0x34343,0x3434f`
- `M386_MARKER name='DOS4GMAINLOADERVECT' count=1 offsets=0x317e1`
- `M386_MARKER name='LINEXE_LOADER' count=4 offsets=0x31ccb,0x31da8,0x329e2,0x338cb`
- `M386_MARKER name='SEGEXE_LOADER' count=4 offsets=0x31cbd,0x33511,0x33532,0x338bd`
- `M386_MARKER name='EXP_LOADER' count=2 offsets=0x31b37,0x338b2`
- `M386_MARKER name='[Enter loader' count=1 offsets=0x325b6`
- `M386_MARKER name='In loader]' count=1 offsets=0x325c7`
- `M386_MARKER name='Leave loader' count=1 offsets=0x325d3`
- `M386_MARKER name='invalid linear executable format' count=1 offsets=0x32851`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
