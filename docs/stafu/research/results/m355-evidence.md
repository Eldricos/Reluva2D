# M355 — dispatch operand validation

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m355-dispatch-operand-validation.txt`
- Original bytes: 38956
- SHA256: `b633a9c2b47dcb51972e789839ccc824c530a8f69adb9301bfaeebd122659ded`
- Original encoding: `utf-16`
- Original line count: 177
- Curated lines retained: 35

## Selected diagnostic observations

- `M355_LAYOUT objects=2 pages=371 pageSize=4096 dataOrigin=503700`
- `M355_DISPATCH_BEGIN name=object_state site=566541 tableDisplacement=0x12334`
- `M355_ANCHOR name=object_state label=entry physical=566260 mappings=obj1:rva0xf460:virtual0x1f460`
- `M355_STATE name=object_state state=1 records=1`
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=-5 physical=578346 first=push 0`
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=-2 physical=578349 first=lahf `
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=-1 physical=578350 first=out dx, eax`
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=0 physical=578351 first=decode_failed`
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=1 physical=578352 first=inc dword ptr [ebx + 0x33eb0cc4]`
- `M355_TARGET_BOUNDARY name=object_state state=1 delta=2 physical=578353 first=add esp, 0xc`
- `M355_STATE name=object_state state=2 records=1`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=-5 physical=578333 first=or al, 0xe8`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=-2 physical=578336 first=decode_failed`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=-1 physical=578337 first=decode_failed`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=0 physical=578338 first=call dword ptr [eax - 0x48]`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=1 physical=578339 first=push eax`
- `M355_TARGET_BOUNDARY name=object_state state=2 delta=2 physical=578340 first=mov eax, 0x1e7e`
- `M355_STATE name=object_state state=3 records=1`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=-5 physical=578393 first=mov eax, dword ptr [ebp + 0x14]`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=-2 physical=578396 first=or dword ptr [eax + 0x3c], ecx`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=-1 physical=578397 first=dec eax`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=0 physical=578398 first=cmp al, 0xeb`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=1 physical=578399 first=jmp 0x8d369`
- `M355_TARGET_BOUNDARY name=object_state state=3 delta=2 physical=578400 first=or byte ptr [ebx + 0x7420d47d], al`
- `M355_STATE name=object_state state=4 records=1`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=-5 physical=578439 first=push eax`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=-2 physical=578442 first=inc ebp`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=-1 physical=578443 first=adc al, 0x66`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=0 physical=578444 first=mov word ptr [eax + 0x66], dx`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=1 physical=578445 first=mov dword ptr [eax + 0x66], edx`
- `M355_TARGET_BOUNDARY name=object_state state=4 delta=2 physical=578446 first=push eax`
- `M355_STATE name=object_state state=5 records=1`
- `M355_TARGET_BOUNDARY name=object_state state=5 delta=-5 physical=578426 first=or byte ptr [ebx - 0x7699ebbb], cl`
- `M355_TARGET_BOUNDARY name=object_state state=5 delta=-2 physical=578429 first=adc al, 0x66`
- `M355_TARGET_BOUNDARY name=object_state state=5 delta=-1 physical=578430 first=mov word ptr [eax + 0x64], dx`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
