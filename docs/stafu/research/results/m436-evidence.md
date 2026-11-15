# M436 — resource lower level trace

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m436-resource-lower-level-trace.txt`
- Original bytes: 142612
- SHA256: `b3ab7a9b83b8f798bb91a4792a693b1fdceea42a2928bef8486917cbd50b3148`
- Original encoding: `utf-16`
- Original line count: 691
- Curated lines retained: 16

## Selected diagnostic observations

- `M436_CFG name=resource_state_8A3F4 entry=0x8a3f4 limit=0x8a518 instructions=44 edges=4`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a3f8 operation=mov operands=esi, dword ptr [esp + 0x14]`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a3fc operation=cmp operands=byte ptr [0x99558], 0`
- `M436_CALL name=resource_state_8A3F4 file=0x8a425 target=0x8ff50`
- `M436_RETURN name=resource_state_8A3F4 file=0x8a431`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a46e operation=mov operands=eax, dword ptr [ebx + 0x44]`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a478 operation=mov operands=edx, dword ptr [ebx + 0x50]`
- `M436_CALL name=resource_state_8A3F4 file=0x8a47d target=0x897b3`
- `M436_CALL name=resource_state_8A3F4 file=0x8a485 target=0x8b548`
- `M436_RETURN name=resource_state_8A3F4 file=0x8a48e`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a4f2 operation=mov operands=eax, dword ptr [esp + 0x18]`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a4f6 operation=mov operands=dword ptr [esi + 0x54], eax`
- `M436_MEMORY name=resource_state_8A3F4 file=0x8a4f9 operation=mov operands=al, byte ptr [esi + 0x5c]`
- `M436_RAW_DIRECT_CALLERS target=0x11b850 count=5`
- `M436_CAUTION raw E8 hits may be data or mid-instruction; no runtime or interprocedural provenance proof.`
- `M436_DONE readOnly=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
