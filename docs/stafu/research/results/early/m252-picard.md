# m252-picard.txt — curated observations

**Source:** `ResearchArchive.zip` / `m252-picard.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `a6e58788b797099d9046433fedeaf26431903efa4fb75756f0d9f55039b106e1`; **encoding:** `utf-16-le`; **original lines:** 19; **selected lines:** 19.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_SPARSE_GAP record=5 imageA=1 imageB=2 startA=3204 startB=3234 compared=1285 equal=1194 longRuns=4
SPR_SPARSE_EXACT offsetA=69 offsetB=69 absoluteA=3273 absoluteB=3303 length=71
SPR_SPARSE_EXACT offsetA=141 offsetB=141 absoluteA=3345 absoluteB=3375 length=429
SPR_SPARSE_EXACT offsetA=571 offsetB=571 absoluteA=3775 absoluteB=3805 length=134
SPR_SPARSE_EXACT offsetA=706 offsetB=706 absoluteA=3910 absoluteB=3940 length=462
SPR_SPARSE_INTERVAL fromRun=1 toRun=2 startA=3344 startB=3374 width=1
SPR_SPARSE_BIT index=1 offset=140 absoluteA=3344 absoluteB=3374 byteA=418 bitInByteA=0 byteB=421 bitInByteB=6 valueA=1 valueB=0 direction=1to0
SPR_SPARSE_INTERVAL fromRun=2 toRun=3 startA=3774 startB=3804 width=1
SPR_SPARSE_BIT index=2 offset=570 absoluteA=3774 absoluteB=3804 byteA=471 bitInByteA=6 byteB=475 bitInByteB=4 valueA=0 valueB=1 direction=0to1
SPR_SPARSE_INTERVAL fromRun=3 toRun=4 startA=3909 startB=3939 width=1
SPR_SPARSE_BIT index=3 offset=705 absoluteA=3909 absoluteB=3939 byteA=488 bitInByteA=5 byteB=492 bitInByteB=3 valueA=1 valueB=0 direction=1to0
SPR_SPARSE_SPACING from=1 to=2 distanceBits=430
SPR_SPARSE_SPACING from=2 to=3 distanceBits=135
SPR_SPARSE_PHASE period=10 samePhase=0 phase1=0 phase2=0 phase3=5
SPR_SPARSE_PHASE period=20 samePhase=0 phase1=0 phase2=10 phase3=5
SPR_SPARSE_PHASE period=30 samePhase=0 phase1=20 phase2=0 phase3=15
SPR_SPARSE_PHASE period=40 samePhase=0 phase1=20 phase2=10 phase3=25
SPR_SPARSE_SUMMARY isolated=3 longRuns=4 compared=1285 equal=1194
SPR_SPARSE_NOTE MSB-first payload-relative coordinates; phase is relative to gap start. Period checks are descriptive only. No codec semantics inferred.
```
