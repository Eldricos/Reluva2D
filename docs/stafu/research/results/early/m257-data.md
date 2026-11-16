# m257-data.txt — curated observations

**Source:** `ResearchArchive.zip` / `m257-data.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `bbd7a7ce27ca644606ab423f136ee386bb330e0869bfd9e9ea861163184da2f5`; **encoding:** `utf-16-le`; **original lines:** 16; **selected lines:** 8.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M257_PAIR resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 bestShift=-20 compared=390 exactStart=113 exactLength=215 absoluteA=10106 absoluteB=10079
SPR_M257_FLANK resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 shift=-20 side=left requested=32 available=113 compared=32 mismatch=15 patternA=10000100110000010000100101111100 patternB=10010101101001100111010011111101 comparison====X===X=XX==XXX=XXXXX=XX======X
SPR_M257_FLANK resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 shift=-20 side=right requested=32 available=62 compared=32 mismatch=14 patternA=00001000010000100001001101000010 patternB=11010000010010011010000100001011 comparison=XX=XX=======X=XXX=XX==X==X==X==X
SPR_M257_PAIR resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 bestShift=-17 compared=1178 exactStart=457 exactLength=314 absoluteA=11255 absoluteB=11204
SPR_M257_FLANK resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 shift=-17 side=left requested=32 available=457 compared=32 mismatch=8 patternA=11010101001100000111010000000001 patternB=10010101110100000110010110000000 comparison==X======XXX========X===XX======X
SPR_M257_FLANK resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 shift=-17 side=right requested=32 available=407 compared=32 mismatch=11 patternA=01000010010100110011011000010001 patternB=10000010010010100110110010011001 comparison=XX=========XX==X=X=XX=X=X===X===
SPR_M257_SUMMARY resource=BRDGDATA.SPR inspected=2 expected=2
SPR_M257_NOTE M249 gap coordinates, MSB-first. Best +/-32-bit alignment uses M256 ranking. Flanks clipped to gap bounds; exact-run boundaries are observational, not codec fields.
```
