# M435 — list node lifecycle

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m435-list-node-lifecycle.txt`
- Original bytes: 57732
- SHA256: `bde04b3d2937d6545f7643eeb4ef7e10c35571688e78754f3376fff2397d759a`
- Original encoding: `utf-16`
- Original line count: 312
- Curated lines retained: 9

## Selected diagnostic observations

- `M435_CALLEE function=node_constructor callsite=0x93575 target=0x930a4`
- `M435_FIRST_RET function=node_constructor file=0x9359d`
- `M435_FUNCTION_END name=node_constructor note=first-ret-not-CFG-proof`
- `M435_CALLEE function=list_destructor callsite=0x936eb target=0x93287`
- `M435_CALLEE function=list_destructor callsite=0x93723 target=0x93287`
- `M435_FIRST_RET function=list_destructor file=0x93731`
- `M435_FUNCTION_END name=list_destructor note=first-ret-not-CFG-proof`
- `M435_CAUTION first-ret traversal omits alternate branches; raw call sites can include data/mid-instruction false positives.`
- `M435_DONE readOnly=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
