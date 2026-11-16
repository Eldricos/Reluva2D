# m256-data.txt — curated observations

**Source:** `ResearchArchive.zip` / `m256-data.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `2a53459fbd14ea277f39a82f744b6617d029846a758c2cbc7488887624e159c2`; **encoding:** `utf-16-le`; **original lines:** 10; **selected lines:** 10.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M256_PAIR resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 bitsA=417 bitsB=390 bestShift=-20
SPR_M256_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=direct shift=0 compared=390 equal=204 mismatch=186 mismatchRuns=96 longestExact=8 exactRuns32=0 leadingMismatchRun=4 trailingMismatchRun=2 centralBits=195 centralMismatch=93 q1=49 q2=47 q3=45 q4=45
SPR_M256_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=best-shift shift=-20 compared=390 equal=295 mismatch=95 mismatchRuns=40 longestExact=215 exactRuns32=1 leadingMismatchRun=3 trailingMismatchRun=0 centralBits=195 centralMismatch=8 q1=61 q2=8 q3=0 q4=26
SPR_M256_EXACT resource=BRDGDATA.SPR mode=best-shift rank=1 offsetA=133 offsetB=113 length=215 beforeBits=113 afterBits=62 leftDiff32=15 rightDiff32=14
SPR_M256_PAIR resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 bitsA=1195 bitsB=1205 bestShift=-17
SPR_M256_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=direct shift=0 compared=1195 equal=620 mismatch=575 mismatchRuns=267 longestExact=17 exactRuns32=0 leadingMismatchRun=2 trailingMismatchRun=0 centralBits=598 centralMismatch=289 q1=141 q2=139 q3=151 q4=144
SPR_M256_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=best-shift shift=-17 compared=1178 equal=781 mismatch=397 mismatchRuns=211 longestExact=314 exactRuns32=1 leadingMismatchRun=0 trailingMismatchRun=1 centralBits=589 centralMismatch=114 q1=137 q2=64 q3=50 q4=146
SPR_M256_EXACT resource=BRDGDATA.SPR mode=best-shift rank=1 offsetA=474 offsetB=457 length=314 beforeBits=457 afterBits=407 leftDiff32=8 rightDiff32=11
SPR_M256_SUMMARY resource=BRDGDATA.SPR inspected=2 expected=2
SPR_M256_NOTE M249 gap coordinates, MSB-first. Direct and best +/-32-bit alignment; exact runs >=32 bits, up to 8 reported per alignment. Quartiles are alignment-local; no codec interpretation.
```
