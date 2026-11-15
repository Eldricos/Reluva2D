# M370 — dispatcher operand consistency

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m370-dispatcher-operand-consistency.txt`
- Original bytes: 9416
- SHA256: `65f2bb9ca4d2847029e0085985a1338c456f8a75ae0cbc9ac238d8882ae0c389`
- Original encoding: `utf-16`
- Original line count: 34
- Curated lines retained: 10

## Selected diagnostic observations

- `M370_ENCODING name=object_state expected=2eff248534230100 actual=2eff248534230100 exact=1`
- `M370_ADDRESS name=object_state hypothesis=CS_base_object1 linear=0x22334 object1Rva=0x12334 mapped=obj1:logical19:physical0x8d2c8 tableFixupMatches=7`
- `M370_ADDRESS name=object_state hypothesis=CS_base_zero linear=0x12334 object1Rva=0x2334 mapped=obj1:logical3:physical0x7d2c8 tableFixupMatches=0`
- `M370_ADDRESS name=object_state hypothesis=displacement_is_linear linear=0x12334 object1Rva=0x2334 mapped=obj1:logical3:physical0x7d2c8 tableFixupMatches=0`
- `M370_TABLE name=object_state entries=7 uniqueTargets=6 exactJumpEncoding=1`
- `M370_ENCODING name=global_state expected=2eff248588340100 actual=2eff248588340100 exact=1`
- `M370_ADDRESS name=global_state hypothesis=CS_base_object1 linear=0x23488 object1Rva=0x13488 mapped=obj1:logical20:physical0x8e41c tableFixupMatches=7`
- `M370_ADDRESS name=global_state hypothesis=CS_base_zero linear=0x13488 object1Rva=0x3488 mapped=obj1:logical4:physical0x7e41c tableFixupMatches=0`
- `M370_ADDRESS name=global_state hypothesis=displacement_is_linear linear=0x13488 object1Rva=0x3488 mapped=obj1:logical4:physical0x7e41c tableFixupMatches=0`
- `M370_TABLE name=global_state entries=7 uniqueTargets=4 exactJumpEncoding=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
