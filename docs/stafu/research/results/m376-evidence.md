# M376 — dispatcher address space consistency

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m376-dispatcher-address-space-consistency.txt`
- Original bytes: 23028
- SHA256: `f3ae6856e16cd4079709b0113f2d2eb60968366bbd2369623b83c859d2819988`
- Original encoding: `utf-16`
- Original line count: 69
- Curated lines retained: 23

## Selected diagnostic observations

- `M376_LAYOUT object1Base=0x10000 object1Size=0xd889a object2Base=0xf0000 object2Size=0x1821b0`
- `M376_JUMP name=object_state jumpRva=0xf579 physical=0x8a50d expected=2eff248534230100 observed=2eff248534230100 matches=1 operand=0x12334`
- `M376_SELECTOR name=object_state assumedCSBase=0x0 effectiveLinear=0x12334 objectRvaCandidates=1:0x2334 reachesProposedTable=0 selectorObserved=0`
- `M376_SELECTOR name=object_state assumedCSBase=0x10000 effectiveLinear=0x22334 objectRvaCandidates=1:0x12334 reachesProposedTable=1 selectorObserved=0`
- `M376_SELECTOR name=object_state assumedCSBase=0xf0000 effectiveLinear=0x102334 objectRvaCandidates=2:0x12334 reachesProposedTable=0 selectorObserved=0`
- `M376_TARGET name=object_state object=1 rva=0x1238e physical=0x8d322 rawBefore=0083c40ce8e1dbfe rawAt=ff50b87e1e0000506a00e89feffeff83c40ceb338b45f48b runtimeCodeUnverified=1`
- `M376_TARGET name=object_state object=1 rva=0x1239b physical=0x8d32f rawBefore=0000506a00e89fef rawAt=feff83c40ceb338b45f48b40088945d4eb208b4514804815 runtimeCodeUnverified=1`
- `M376_TARGET name=object_state object=1 rva=0x123ca physical=0x8d35e rawBefore=e089c18b45140948 rawAt=3ceb08837dd42074daebe18b45f80145fce9c30e00008b45 runtimeCodeUnverified=1`
- `M376_TARGET name=object_state object=1 rva=0x123eb physical=0x8d37f rawBefore=668b50088b451466 rawAt=8950648b45f4668b500c8b4514668950668b45f80145fce9 runtimeCodeUnverified=1`
- `M376_TARGET name=object_state object=1 rva=0x123f8 physical=0x8d38c rawBefore=f4668b500c8b4514 rawAt=668950668b45f80145fce99c0e00008b45f48b400cc1f804 runtimeCodeUnverified=1`
- `M376_TARGET name=object_state object=1 rva=0x12429 physical=0x8d3bd rawBefore=04e840c80800c1f8 rawAt=0489c289d0c1fa1ff7fb0315549506008b451401909c0000 runtimeCodeUnverified=1`
- `M376_NEAR_REFERENCE name=object_state needle=operand value=0x12334 windowRva=0xf379-0xf779 rawByteHits=1 positions=0xf57d semanticReferencesUnverified=1`
- `M376_NEAR_REFERENCE name=object_state needle=tableLinear value=0x22334 windowRva=0xf379-0xf779 rawByteHits=0 positions=none semanticReferencesUnverified=1`
- `M376_JUMP name=global_state jumpRva=0x106c1 physical=0x8b655 expected=2eff248588340100 observed=2eff248588340100 matches=1 operand=0x13488`
- `M376_SELECTOR name=global_state assumedCSBase=0x0 effectiveLinear=0x13488 objectRvaCandidates=1:0x3488 reachesProposedTable=0 selectorObserved=0`
- `M376_SELECTOR name=global_state assumedCSBase=0x10000 effectiveLinear=0x23488 objectRvaCandidates=1:0x13488 reachesProposedTable=1 selectorObserved=0`
- `M376_SELECTOR name=global_state assumedCSBase=0xf0000 effectiveLinear=0x103488 objectRvaCandidates=2:0x13488 reachesProposedTable=0 selectorObserved=0`
- `M376_TARGET name=global_state object=1 rva=0x134ed physical=0x8e481 rawBefore=fa1f2bc2d1f889c2 rawAt=8b45e88b404ec1f81029d08945f08b45e88b5050c1fa108b runtimeCodeUnverified=1`
- `M376_TARGET name=global_state object=1 rva=0x13561 physical=0x8e4f5 rawBefore=8b45e8f640380874 rawAt=358b45e8ff70588b45fc8b55fcc1fa1f2bc2d1f80345f450 runtimeCodeUnverified=1`
- `M376_TARGET name=global_state object=1 rva=0x1356e physical=0x8e502 rawBefore=70588b45fc8b55fc rawAt=c1fa1f2bc2d1f80345f4508b45f88b55f8c1fa1f2bc2d1f8 runtimeCodeUnverified=1`
- `M376_TARGET name=global_state object=1 rva=0x13598 physical=0x8e52c rawBefore=c40ce9f20200008b rawAt=45e8f6403810740d8b45e8f64038800f85200100008b45e8 runtimeCodeUnverified=1`
- `M376_NEAR_REFERENCE name=global_state needle=operand value=0x13488 windowRva=0x104c1-0x108c1 rawByteHits=1 positions=0x106c5 semanticReferencesUnverified=1`
- `M376_NEAR_REFERENCE name=global_state needle=tableLinear value=0x23488 windowRva=0x104c1-0x108c1 rawByteHits=0 positions=none semanticReferencesUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
