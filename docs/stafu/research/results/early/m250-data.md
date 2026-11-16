# m250-data.txt — curated observations

**Source:** `ResearchArchive.zip` / `m250-data.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `0a93e097310f9f39027c8e234df97b72ed1640ff4a2ac5bf840fd9a4b4ea470b`; **encoding:** `utf-16-le`; **original lines:** 28; **selected lines:** 28.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_LOCAL_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 startA=9973 endA=10390 startB=9966 endB=10356 bitsA=417 bitsB=390 directEqual=204 directCompared=390 bestShift=-20 bestEqual=295 bestCompared=390
SPR_LOCAL_MAP resource=BRDGDATA.SPR mode=direct shift=0 compared=390 equal=204 mismatches=186 equalRuns=95 mismatchRuns=96 longestEqual=8 longestMismatch=7 top1MismatchBits=7 top3MismatchBits=19 top5MismatchBits=30
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=249 offsetB=249 length=8
SPR_LOCAL_RUN_NOTE resource=BRDGDATA.SPR mode=direct reported=1 minLength=8 maxRuns=48
SPR_LOCAL_MAP resource=BRDGDATA.SPR mode=best-shift shift=-20 compared=390 equal=295 mismatches=95 equalRuns=40 mismatchRuns=40 longestEqual=215 longestMismatch=13 top1MismatchBits=13 top3MismatchBits=24 top5MismatchBits=33
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=mismatch offsetA=44 offsetB=24 length=13
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=133 offsetB=113 length=215
SPR_LOCAL_RUN_NOTE resource=BRDGDATA.SPR mode=best-shift reported=2 minLength=8 maxRuns=48
SPR_LOCAL_GAP resource=BRDGDATA.SPR record=0 imageA=0 imageB=1 startA=10781 endA=11976 startB=10747 endB=11952 bitsA=1195 bitsB=1205 directEqual=620 directCompared=1195 bestShift=-17 bestEqual=781 bestCompared=1178
SPR_LOCAL_MAP resource=BRDGDATA.SPR mode=direct shift=0 compared=1195 equal=620 mismatches=575 equalRuns=267 mismatchRuns=267 longestEqual=17 longestMismatch=8 top1MismatchBits=8 top3MismatchBits=22 top5MismatchBits=35
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=2 offsetB=2 length=9
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=102 offsetB=102 length=17
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=123 offsetB=123 length=10
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=339 offsetB=339 length=8
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=350 offsetB=350 length=8
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=mismatch offsetA=365 offsetB=365 length=8
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=direct type=equal offsetA=652 offsetB=652 length=12
SPR_LOCAL_RUN_NOTE resource=BRDGDATA.SPR mode=direct reported=7 minLength=8 maxRuns=48
SPR_LOCAL_MAP resource=BRDGDATA.SPR mode=best-shift shift=-17 compared=1178 equal=781 mismatches=397 equalRuns=211 mismatchRuns=211 longestEqual=314 longestMismatch=9 top1MismatchBits=9 top3MismatchBits=20 top5MismatchBits=30
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=64 offsetB=47 length=13
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=81 offsetB=64 length=9
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=mismatch offsetA=345 offsetB=328 length=9
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=453 offsetB=436 length=8
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=474 offsetB=457 length=314
SPR_LOCAL_RUN resource=BRDGDATA.SPR mode=best-shift type=equal offsetA=790 offsetB=773 length=9
SPR_LOCAL_RUN_NOTE resource=BRDGDATA.SPR mode=best-shift reported=6 minLength=8 maxRuns=48
SPR_LOCAL_SUMMARY resource=BRDGDATA.SPR inspected=2 expected=2
SPR_LOCAL_NOTE Coordinates are verified M249 gap coordinates. Direct and best-shift comparisons use full gaps and MSB-first bits. Runs shorter than 8 bits are omitted from detail, not totals. No codec interpretation is implied.
```
