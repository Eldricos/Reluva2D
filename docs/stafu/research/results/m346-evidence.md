# M346 — post read buffer processing

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m346-post-read-buffer-processing.txt`
- Original bytes: 66276
- SHA256: `9d39534d24625eb3dddaf095d047e1e5bab27ae27545b1371ab9672b1ea56f4c`
- Original encoding: `utf-16`
- Original line count: 502
- Curated lines retained: 29

## Selected diagnostic observations

- `M346_FUNCTION_END name=post_read_645730`
- `M346_CALL site=563188 target=517540`
- `M346_FUNCTION_END name=stream_close_563166`
- `M346_FAILURE function=related_646108 site=646447`
- `M346_FUNCTION_END name=related_646108`
- `M346_FAILURE function=related_646841 site=647157`
- `M346_FUNCTION_END name=related_646841`
- `M346_ANCHOR name=read_destination start=568440 end=568540`
- `M346_CALL site=568454 target=563062`
- `M346_CALL site=568471 target=645730`
- `M346_CALL site=568484 target=563166`
- `M346_CALL site=568503 target=1152516`
- `M346_ANCHOR_END name=read_destination`
- `M346_ANCHOR name=read_embedded start=568284 end=568345`
- `M346_CALL site=568294 target=563062`
- `M346_ANCHOR_END name=read_embedded`
- `M346_ANCHOR name=post_read_570032 start=570032 end=570064`
- `M346_CALL site=570032 target=563062`
- `M346_ANCHOR_END name=post_read_570032`
- `M346_ANCHOR name=embedded_object_init start=567408 end=567450`
- `M346_ANCHOR_END name=embedded_object_init`
- `M346_ANCHOR name=external_object_init start=572353 end=572395`
- `M346_CALL site=572383 target=603497`
- `M346_ANCHOR_END name=external_object_init`
- `M346_RAW_CALLERS target=645730 count=15 sites=518608,520401,522627,522996,524346,524370,534118,564464,568471,571376,639850,645316,647960,650061,677057`
- `M346_RAW_CALLERS target=563166 count=12 sites=519559,532555,532861,532972,533014,565697,566190,568484,757507,784530,839928,1059249`
- `M346_RAW_CALLERS target=646108 count=2 sites=565561,568378`
- `M346_RAW_CALLERS target=646841 count=2 sites=565595,568410`
- `M346_LIMITATIONS staticOnly=1 physicalOffsets=1 LEFixupsUnapplied=1 indirectControlFlowUnresolved=1 functionBoundsHeuristic=1 rawCallersMayBeData=1 pointerProvenanceUnverified=1 SPRDecoderUnverified=1`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
