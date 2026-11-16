# m251-picard.txt — curated observations

**Source:** `ResearchArchive.zip` / `m251-picard.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `ca365edd1966712435eb9a9d463024ce66b8452bb2a5b7547413b6852d10c022`; **encoding:** `utf-16-le`; **original lines:** 22; **selected lines:** 22.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_BOUNDARY_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 startA=3204 endA=4489 startB=3234 endB=4526 bitsA=1285 bitsB=1292 directEqual=1194 directCompared=1285 bestShift=0 bestEqual=1194 bestCompared=1285
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=1285 equal=1194 qualifyingRuns=4 reported=4 minRun=32
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=direct rank=1 offsetA=706 offsetB=706 absoluteA=3910 absoluteB=3940 length=462 leftCompared=16 leftEqual=15 rightCompared=16 rightEqual=10 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=direct rank=2 offsetA=141 offsetB=141 absoluteA=3345 absoluteB=3375 length=429 leftCompared=16 leftEqual=15 rightCompared=16 rightEqual=15 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=direct rank=3 offsetA=571 offsetB=571 absoluteA=3775 absoluteB=3805 length=134 leftCompared=16 leftEqual=15 rightCompared=16 rightEqual=15 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=direct rank=4 offsetA=69 offsetB=69 absoluteA=3273 absoluteB=3303 length=71 leftCompared=16 leftEqual=15 rightCompared=16 rightEqual=15 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_INTERVAL resource=BRDGPICA.SPR mode=direct from=1 to=2 startA=140 startB=140 width=1 directEqual=0 localBestDelta=0 localBestEqual=0 localBestCompared=0
SPR_BOUNDARY_INTERVAL resource=BRDGPICA.SPR mode=direct from=2 to=3 startA=570 startB=570 width=1 directEqual=0 localBestDelta=0 localBestEqual=0 localBestCompared=0
SPR_BOUNDARY_INTERVAL resource=BRDGPICA.SPR mode=direct from=3 to=4 startA=705 startB=705 width=1 directEqual=0 localBestDelta=0 localBestEqual=0 localBestCompared=0
SPR_BOUNDARY_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 startA=4967 endA=5093 startB=5004 endB=5120 bitsA=126 bitsB=116 directEqual=94 directCompared=116 bestShift=0 bestEqual=94 bestCompared=116
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=116 equal=94 qualifyingRuns=1 reported=1 minRun=32
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=direct rank=1 offsetA=61 offsetB=61 absoluteA=5028 absoluteB=5065 length=55 leftCompared=16 leftEqual=11 rightCompared=0 rightEqual=0 leftImmediateMismatch=1 rightImmediateMismatch=0
SPR_BOUNDARY_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 startA=9730 endA=10499 startB=9792 endB=10603 bitsA=769 bitsB=811 directEqual=412 directCompared=769 bestShift=7 bestEqual=573 bestCompared=769
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=769 equal=412 qualifyingRuns=0 reported=0 minRun=32
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=best-shift shift=7 compared=769 equal=573 qualifyingRuns=1 reported=1 minRun=32
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=best-shift rank=1 offsetA=190 offsetB=197 absoluteA=9920 absoluteB=9989 length=388 leftCompared=16 leftEqual=14 rightCompared=16 rightEqual=10 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_GAP resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 startA=6342 endA=6591 startB=6464 endB=6713 bitsA=249 bitsB=249 directEqual=130 directCompared=249 bestShift=30 bestEqual=149 bestCompared=219
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 mode=direct shift=0 compared=249 equal=130 qualifyingRuns=0 reported=0 minRun=32
SPR_BOUNDARY_MAP resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 mode=best-shift shift=30 compared=219 equal=149 qualifyingRuns=1 reported=1 minRun=32
SPR_BOUNDARY_RUN resource=BRDGPICA.SPR mode=best-shift rank=1 offsetA=115 offsetB=145 absoluteA=6457 absoluteB=6609 length=45 leftCompared=16 leftEqual=9 rightCompared=16 rightEqual=13 leftImmediateMismatch=1 rightImmediateMismatch=1
SPR_BOUNDARY_SUMMARY resource=BRDGPICA.SPR inspected=4 expected=4
SPR_BOUNDARY_NOTE M249 gap coordinates, MSB-first. Up to 12 exact runs >=32 bits per alignment, with 16-bit boundary contexts. Local interval shifts +/-16 are descriptive; insertions/deletions and codec structure are not inferred.
```
