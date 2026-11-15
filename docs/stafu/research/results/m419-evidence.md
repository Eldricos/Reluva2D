# M419 — internal resolver control flow

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m419-internal-resolver-control-flow.txt`
- Original bytes: 8198
- SHA256: `4c2e7a573346c842429443e9724ae6c1d88cb6294d806b9b24b600c443435880`
- Original encoding: `utf-16`
- Original line count: 54
- Curated lines retained: 6

## Selected diagnostic observations

- `M419_BRANCH file=0x2e974 target=0x2e993 withinHelper=1 label=jnz short existing-handle targetBytes=8b46fe0b46fc742a8b46060b`
- `M419_BRANCH file=0x2e999 target=0x2e9c5 withinHelper=1 label=jz short no-handle targetBytes=b80100eb022bc01fc9c21000`
- `M419_BRANCH file=0x2e9a1 target=0x2e9ca withinHelper=1 label=jz short no-export-name targetBytes=2bc01fc9c21000c80800001e`
- `M419_BRANCH file=0x2e9c3 target=0x2e9ca withinHelper=1 label=jnz short success targetBytes=2bc01fc9c21000c80800001e`
- `M419_MODEL inputOutputPointer=[bp+4] moduleHandlePointer=[bp+8] exportName=[bp+12:bp+14] moduleName=[bp+16:bp+18]`
- `M419_LIMITATIONS staticOnly=1 branchTargetsNeedBoundaryValidation=1 runtimeModuleStateUnknown=1 farCallSemanticsInferred=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
