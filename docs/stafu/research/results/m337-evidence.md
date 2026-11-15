# M337 — spr embedded data trace

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m337-spr-embedded-data-trace.txt`
- Original bytes: 78398
- SHA256: `a224f271665b828dbad35b960bef31c0fde479415cbd0b5378d5c7cbde641e24`
- Original encoding: `utf-16`
- Original line count: 415
- Curated lines retained: 20

## Selected diagnostic observations

- `M337_REGION name=helper_564528 start=564528 end=565450 reached=43 boundedCFG=1`
- `M337_CALL region=helper_564528 at=564558 target=1170094`
- `M337_CALL region=helper_564528 at=564581 target=1170094`
- `M337_CALL region=helper_564528 at=564605 target=1170094`
- `M337_RETURN region=helper_564528 at=564627`
- `M337_REGION_END name=helper_564528`
- `M337_REGION name=helper_563005 start=563005 end=563650 reached=25 boundedCFG=1`
- `M337_CALL region=helper_563005 at=563031 target=517136`
- `M337_RETURN region=helper_563005 at=563061`
- `M337_REGION_END name=helper_563005`
- `M337_CALL region=constructor_567212 at=567243 target=563005`
- `M337_CALL region=constructor_567212 at=567305 target=587664`
- `M337_CALL region=constructor_567212 at=567364 target=564528`
- `M337_CALL region=constructor_567212 at=567464 target=603497`
- `M337_CALL region=constructor_567212 at=567477 target=570696`
- `M337_CALL region=constructor_567212 at=567482 target=570696`
- `M337_CALL region=allocation_587664 at=587670 target=1175190`
- `M337_CALL region=allocation_587664 at=587692 target=1152516`
- `M337_CALL region=caller_580700 at=580767 target=567212`
- `M337_QUESTIONS helper_564528_input_consumption=unknown embedded_data_at_0x88_writes=unknown transform_vs_copy=unknown`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
