# M373 — le source offset semantics

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m373-le-source-offset-semantics.txt`
- Original bytes: 14146
- SHA256: `93857b5e0f11c788da62fa8e5d0f0014206eb720a198183ce5ca96ae019dca2a`
- Original encoding: `utf-16`
- Original line count: 52
- Curated lines retained: 35

## Selected diagnostic observations

- `M373_SPEC_AUDIT sourceType07=offset32 sourceOffsetField=signed16 possibleCrossPage=1 chainBitSource=0x20 chainBitTarget=0x08 chainNotObservedByRestrictedParser=1`
- `M373_LAYOUT pageSize=4096 modulePages=371 object1Base=0x10000 object1FirstPage=1 object1Pages=217`
- `M373_REGION name=object_state pageBaseRva=0x12000 tableOffset=0x334`
- `M373_TABLE name=object_state index=0 source=0x334 record=885 flags=0x10 targetObject=1 targetOffset=0x1239b`
- `M373_TABLE name=object_state index=1 source=0x338 record=876 flags=0x10 targetObject=1 targetOffset=0x1238e`
- `M373_TABLE name=object_state index=2 source=0x33c record=867 flags=0x10 targetObject=1 targetOffset=0x123ca`
- `M373_TABLE name=object_state index=3 source=0x340 record=858 flags=0x10 targetObject=1 targetOffset=0x123f8`
- `M373_TABLE name=object_state index=4 source=0x344 record=849 flags=0x10 targetObject=1 targetOffset=0x123eb`
- `M373_TABLE name=object_state index=5 source=0x348 record=840 flags=0x10 targetObject=1 targetOffset=0x123ca`
- `M373_TABLE name=object_state index=6 source=0x34c record=831 flags=0x10 targetObject=1 targetOffset=0x12429`
- `M373_HANDLER name=object_state target=0x1239b local=0x39b`
- `M373_NEAR name=object_state handler=0x1239b record=982 signedSourceRva=0x123b1 delta=+22 original=481502eb targetObject=2 targetOffset=0x9ee78 relocatedValue=0x18ee78 precedingInstructionCandidates=-1:or_byte_ptr_[eax_+_0x15],_2|-5:and_byte_ptr_[ebx_+_0x48801445],_cl`
- `M373_HANDLER name=object_state target=0x1238e local=0x38e`
- `M373_NEAR name=object_state handler=0x1238e record=991 signedSourceRva=0x1237d delta=-17 original=5a1e0000 targetObject=2 targetOffset=0x1867 relocatedValue=0xf1867 precedingInstructionCandidates=-1:mov_eax,_0x1e5a`
- `M373_NEAR name=object_state handler=0x1238e record=998 signedSourceRva=0x12378 delta=-22 original=1e000050 targetObject=2 targetOffset=0x1848 relocatedValue=0xf1848 precedingInstructionCandidates=-2:mov_eax,_0x1e48|-3:add_byte_ptr_[eax_+_0x1e48],_bh`
- `M373_HANDLER name=object_state target=0x123ca local=0x3ca`
- `M373_NEAR name=object_state handler=0x123ca record=973 signedSourceRva=0x123c6 delta=-4 original=45140948 targetObject=2 targetOffset=0x9ee78 relocatedValue=0x18ee78 precedingInstructionCandidates=-1:mov_eax,_dword_ptr_[ebp_+_0x14]|-2:ror_dword_ptr_[ebx_+_0x48091445],_0x3c`
- `M373_HANDLER name=object_state target=0x123f8 local=0x3f8`
- `M373_NEAR name=object_state handler=0x123f8 record=964 signedSourceRva=0x1240e delta=+22 original=f8048b55 targetObject=2 targetOffset=0x9ee78 relocatedValue=0x18ee78 precedingInstructionCandidates=-1:sar_eax,_4`
- `M373_HANDLER name=object_state target=0x123eb local=0x3eb`
- `M373_HANDLER name=object_state target=0x12429 local=0x429`
- `M373_NEAR name=object_state handler=0x12429 record=1060 signedSourceRva=0x1243a delta=+17 original=45140190 targetObject=2 targetOffset=0x9ef10 relocatedValue=0x18ef10 precedingInstructionCandidates=-1:mov_eax,_dword_ptr_[ebp_+_0x14]|-2:add_byte_ptr_[ebx_-_0x6ffeebbb],_cl`
- `M373_NEAR name=object_state handler=0x12429 record=1069 signedSourceRva=0x12423 delta=-6 original=40c80800 targetObject=2 targetOffset=0x9ee78 relocatedValue=0x18ee78 precedingInstructionCandidates=-1:call_0x9ec67`
- `M373_REGION name=global_state pageBaseRva=0x13000 tableOffset=0x488`
- `M373_TABLE name=global_state index=0 source=0x488 record=1285 flags=0x10 targetObject=1 targetOffset=0x13598`
- `M373_TABLE name=global_state index=1 source=0x48c record=1276 flags=0x10 targetObject=1 targetOffset=0x13598`
- `M373_TABLE name=global_state index=2 source=0x490 record=1267 flags=0x10 targetObject=1 targetOffset=0x1356e`
- `M373_TABLE name=global_state index=3 source=0x494 record=1258 flags=0x10 targetObject=1 targetOffset=0x134ed`
- `M373_TABLE name=global_state index=4 source=0x498 record=1249 flags=0x10 targetObject=1 targetOffset=0x13598`
- `M373_TABLE name=global_state index=5 source=0x49c record=1240 flags=0x10 targetObject=1 targetOffset=0x1356e`
- `M373_TABLE name=global_state index=6 source=0x4a0 record=1231 flags=0x10 targetObject=1 targetOffset=0x13561`
- `M373_HANDLER name=global_state target=0x13598 local=0x598`
- `M373_NEAR name=global_state handler=0x13598 record=1312 signedSourceRva=0x135a0 delta=+8 original=8b45e8f6 targetObject=2 targetOffset=0x9ee74 relocatedValue=0x18ee74 precedingInstructionCandidates=-1:or_eax,_0xf6e8458b|-3:adc_byte_ptr_[ebp_+_ecx_-_0x75],_dh`
- `M373_HANDLER name=global_state target=0x1356e local=0x56e`
- `M373_NEAR name=global_state handler=0x1356e record=1321 signedSourceRva=0x13574 delta=+6 original=f80345f4 targetObject=2 targetOffset=0x1940 relocatedValue=0xf1940 precedingInstructionCandidates=-1:sar_eax,_1|-2:ret_0xf8d1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
