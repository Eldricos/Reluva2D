# M412 — call boundary stack lifetime

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m412-call-boundary-stack-lifetime.txt`
- Original bytes: 42094
- SHA256: `b20b9d7d884005c4f8ed20abe361e871e575f14d60b06cf32912e2f2b82537e2`
- Original encoding: `utf-16`
- Original line count: 202
- Curated lines retained: 6

## Selected diagnostic observations

- `M412_PATH_COUNTS name=ALLOC_A total=7 withoutUnconditionalBarrier=6`
- `M412_PATH_COUNTS name=ALLOC_B total=8 withoutUnconditionalBarrier=8`
- `M412_PATH_COUNTS name=BROADCAST_A total=20 withoutUnconditionalBarrier=20`
- `M412_PATH_COUNTS name=BROADCAST_B total=20 withoutUnconditionalBarrier=20`
- `M412_PATH_COUNTS name=VMFILEIO total=19 withoutUnconditionalBarrier=19`
- `M412_NOTE earlier calls are lifetime boundaries; callee cleanup is unknown`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
