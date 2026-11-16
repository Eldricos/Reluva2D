# m251-data.txt — curated observations

**Source:** `ResearchArchive.zip` / `m251-data.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `e382aaf7eb81f394b7e2231cb2ec334a6dd7af542e12b49e6b3223d044ebb815`; **encoding:** `utf-16-le`; **original lines:** 10; **selected lines:** 10.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_BOUNDARY_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 startA=9973 endA=10390 startB=9966 endB=10356 bitsA=417 bitsB=390 directEqual=204 directCompared=390 bestShift=-20 bestEqual=295 bestCompared=390
SPR_BOUNDARY_MAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=direct shift=0 compared=390 equal=204 qualifyingRuns=0 reported=0 minRun=32
SPR_BOUNDARY_MAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=best-shift shift=-20 compared=390 equal=295 qualifyingRuns=1 reported=1 minRun=32
SPR_BOUNDARY_RUN resource=BRDGDATA.SPR mode=best-shift rank=1 offsetA=133 offsetB=113 absoluteA=10106 absoluteB=10079 length=215 leftCompared=16 leftEqual=8 rightCompared=16 rightEqual=9 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 startA=10781 endA=11976 startB=10747 endB=11952 bitsA=1195 bitsB=1205 directEqual=620 directCompared=1195 bestShift=-17 bestEqual=781 bestCompared=1178
SPR_BOUNDARY_MAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=direct shift=0 compared=1195 equal=620 qualifyingRuns=0 reported=0 minRun=32
SPR_BOUNDARY_MAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 mode=best-shift shift=-17 compared=1178 equal=781 qualifyingRuns=1 reported=1 minRun=32
SPR_BOUNDARY_RUN resource=BRDGDATA.SPR mode=best-shift rank=1 offsetA=474 offsetB=457 absoluteA=11255 absoluteB=11204 length=314 leftCompared=16 leftEqual=12 rightCompared=16 rightEqual=11 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_SUMMARY resource=BRDGDATA.SPR inspected=2 expected=2
SPR_BOUNDARY_NOTE M249 gap coordinates, MSB-first. Up to 12 exact runs >=32 bits per alignment, with 16-bit boundary contexts. Local interval shifts +/-16 are descriptive; insertions/deletions and codec structure are not inferred.
```
