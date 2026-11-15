# M369 — le fixup target semantics

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m369-le-fixup-target-semantics.txt`
- Original bytes: 56076
- SHA256: `9d1866c6cd127fa4b6314930df6b726ef33eb2d17b05303aa471f2549bcea001`
- Original encoding: `utf-16`
- Original line count: 202
- Curated lines retained: 35

## Selected diagnostic observations

- `M369_ADDRESS name=object_state state=1 interpretation=object_rva rva=0x1239b inObject=1 inCurrentPage=1`
- `M369_ADDRESS name=object_state state=1 interpretation=linear_absolute rva=0x239b inObject=1 inCurrentPage=0`
- `M369_ADDRESS name=object_state state=1 interpretation=base_added_twice rva=0x2239b inObject=1 inCurrentPage=0`
- `M369_BOUNDARY name=object_state state=1 delta=-4 startRva=0x12397 decoded=3 decodedBytes=4 fixupOverlap=0 privilegedOrUnusual=1 first=add_al,_ch`
- `M369_BOUNDARY name=object_state state=1 delta=-3 startRva=0x12398 decoded=5 decodedBytes=16 fixupOverlap=0 privilegedOrUnusual=0 first=call_0x133c`
- `M369_BOUNDARY name=object_state state=1 delta=-2 startRva=0x12399 decoded=2 decodedBytes=2 fixupOverlap=0 privilegedOrUnusual=1 first=lahf_`
- `M369_BOUNDARY name=object_state state=1 delta=-1 startRva=0x1239a decoded=1 decodedBytes=1 fixupOverlap=0 privilegedOrUnusual=1 first=out_dx,_eax`
- `M369_BOUNDARY name=object_state state=1 delta=+0 startRva=0x1239b decoded=0 decodedBytes=0 fixupOverlap=0 privilegedOrUnusual=0 first=none`
- `M369_BOUNDARY name=object_state state=1 delta=+1 startRva=0x1239c decoded=5 decodedBytes=17 fixupOverlap=0 privilegedOrUnusual=0 first=inc_dword_ptr_[ebx_+_0x33eb0cc4]`
- `M369_BOUNDARY name=object_state state=1 delta=+2 startRva=0x1239d decoded=5 decodedBytes=14 fixupOverlap=0 privilegedOrUnusual=0 first=add_esp,_0xc`
- `M369_BOUNDARY name=object_state state=1 delta=+3 startRva=0x1239e decoded=5 decodedBytes=22 fixupOverlap=1 privilegedOrUnusual=0 first=les_ecx,_ptr_[ebx_+_ebp*8]`
- `M369_BOUNDARY name=object_state state=1 delta=+4 startRva=0x1239f decoded=5 decodedBytes=21 fixupOverlap=1 privilegedOrUnusual=0 first=or_al,_0xeb`
- `M369_ADDRESS name=object_state state=2 interpretation=object_rva rva=0x1238e inObject=1 inCurrentPage=1`
- `M369_ADDRESS name=object_state state=2 interpretation=linear_absolute rva=0x238e inObject=1 inCurrentPage=0`
- `M369_ADDRESS name=object_state state=2 interpretation=base_added_twice rva=0x2238e inObject=1 inCurrentPage=0`
- `M369_BOUNDARY name=object_state state=2 delta=-4 startRva=0x1238a decoded=5 decodedBytes=14 fixupOverlap=0 privilegedOrUnusual=0 first=call_0xffffff70`
- `M369_BOUNDARY name=object_state state=2 delta=-3 startRva=0x1238b decoded=1 decodedBytes=2 fixupOverlap=0 privilegedOrUnusual=0 first=loope_0x12368`
- `M369_BOUNDARY name=object_state state=2 delta=-2 startRva=0x1238c decoded=0 decodedBytes=0 fixupOverlap=0 privilegedOrUnusual=0 first=none`
- `M369_BOUNDARY name=object_state state=2 delta=-1 startRva=0x1238d decoded=0 decodedBytes=0 fixupOverlap=0 privilegedOrUnusual=0 first=none`
- `M369_BOUNDARY name=object_state state=2 delta=+0 startRva=0x1238e decoded=5 decodedBytes=10 fixupOverlap=0 privilegedOrUnusual=0 first=call_dword_ptr_[eax_-_0x48]`
- `M369_BOUNDARY name=object_state state=2 delta=+1 startRva=0x1238f decoded=5 decodedBytes=14 fixupOverlap=0 privilegedOrUnusual=0 first=push_eax`
- `M369_BOUNDARY name=object_state state=2 delta=+2 startRva=0x12390 decoded=5 decodedBytes=16 fixupOverlap=0 privilegedOrUnusual=0 first=mov_eax,_0x1e7e`
- `M369_BOUNDARY name=object_state state=2 delta=+3 startRva=0x12391 decoded=5 decodedBytes=12 fixupOverlap=0 privilegedOrUnusual=0 first=jle_0x123b1`
- `M369_BOUNDARY name=object_state state=2 delta=+4 startRva=0x12392 decoded=5 decodedBytes=11 fixupOverlap=0 privilegedOrUnusual=0 first=push_ds`
- `M369_ADDRESS name=object_state state=3 interpretation=object_rva rva=0x123ca inObject=1 inCurrentPage=1`
- `M369_ADDRESS name=object_state state=3 interpretation=linear_absolute rva=0x23ca inObject=1 inCurrentPage=0`
- `M369_ADDRESS name=object_state state=3 interpretation=base_added_twice rva=0x223ca inObject=1 inCurrentPage=0`
- `M369_BOUNDARY name=object_state state=3 delta=-4 startRva=0x123c6 decoded=5 decodedBytes=12 fixupOverlap=1 privilegedOrUnusual=0 first=inc_ebp`
- `M369_BOUNDARY name=object_state state=3 delta=-3 startRva=0x123c7 decoded=4 decodedBytes=11 fixupOverlap=1 privilegedOrUnusual=0 first=adc_al,_9`
- `M369_BOUNDARY name=object_state state=3 delta=-2 startRva=0x123c8 decoded=5 decodedBytes=13 fixupOverlap=1 privilegedOrUnusual=0 first=or_dword_ptr_[eax_+_0x3c],_ecx`
- `M369_BOUNDARY name=object_state state=3 delta=-1 startRva=0x123c9 decoded=3 decodedBytes=9 fixupOverlap=1 privilegedOrUnusual=0 first=dec_eax`
- `M369_BOUNDARY name=object_state state=3 delta=+0 startRva=0x123ca decoded=2 decodedBytes=8 fixupOverlap=0 privilegedOrUnusual=0 first=cmp_al,_0xeb`
- `M369_BOUNDARY name=object_state state=3 delta=+1 startRva=0x123cb decoded=5 decodedBytes=13 fixupOverlap=0 privilegedOrUnusual=0 first=jmp_0x123d5`
- `M369_BOUNDARY name=object_state state=3 delta=+2 startRva=0x123cc decoded=1 decodedBytes=6 fixupOverlap=0 privilegedOrUnusual=0 first=or_byte_ptr_[ebx_+_0x7420d47d],_al`
- `M369_BOUNDARY name=object_state state=3 delta=+3 startRva=0x123cd decoded=5 decodedBytes=14 fixupOverlap=0 privilegedOrUnusual=0 first=cmp_dword_ptr_[ebp_-_0x2c],_0x20`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
