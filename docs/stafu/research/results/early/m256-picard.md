# m256-picard.txt — curated observations

**Source:** `ResearchArchive.zip` / `m256-picard.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `a0c11619562ad0bd48a42cc30886e09f318b985a02d966f8e2365a3ee0d36d33`; **encoding:** `utf-16-le`; **original lines:** 19; **selected lines:** 19.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M256_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bitsA=1285 bitsB=1292 bestShift=0
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=1285 equal=1194 mismatch=91 mismatchRuns=44 longestExact=462 exactRuns32=4 leadingMismatchRun=2 trailingMismatchRun=2 centralBits=642 centralMismatch=2 q1=26 q2=1 q3=1 q4=63
SPR_M256_EXACT resource=BRDGPICA.SPR mode=direct rank=1 offsetA=706 offsetB=706 length=462 beforeBits=706 afterBits=117 leftDiff32=1 rightDiff32=18
SPR_M256_EXACT resource=BRDGPICA.SPR mode=direct rank=2 offsetA=141 offsetB=141 length=429 beforeBits=141 afterBits=715 leftDiff32=1 rightDiff32=1
SPR_M256_EXACT resource=BRDGPICA.SPR mode=direct rank=3 offsetA=571 offsetB=571 length=134 beforeBits=571 afterBits=580 leftDiff32=1 rightDiff32=1
SPR_M256_EXACT resource=BRDGPICA.SPR mode=direct rank=4 offsetA=69 offsetB=69 length=71 beforeBits=69 afterBits=1145 leftDiff32=8 rightDiff32=1
SPR_M256_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bitsA=126 bitsB=116 bestShift=0
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=116 equal=94 mismatch=22 mismatchRuns=12 longestExact=55 exactRuns32=1 leadingMismatchRun=2 trailingMismatchRun=0 centralBits=58 centralMismatch=10 q1=12 q2=7 q3=3 q4=0
SPR_M256_EXACT resource=BRDGPICA.SPR mode=direct rank=1 offsetA=61 offsetB=61 length=55 beforeBits=61 afterBits=0 leftDiff32=10 rightDiff32=0
SPR_M256_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bitsA=769 bitsB=811 bestShift=7
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=direct shift=0 compared=769 equal=412 mismatch=357 mismatchRuns=199 longestExact=14 exactRuns32=0 leadingMismatchRun=3 trailingMismatchRun=3 centralBits=384 centralMismatch=190 q1=78 q2=93 q3=97 q4=89
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 mode=best-shift shift=7 compared=769 equal=573 mismatch=196 mismatchRuns=82 longestExact=388 exactRuns32=1 leadingMismatchRun=0 trailingMismatchRun=2 centralBits=384 centralMismatch=0 q1=97 q2=0 q3=0 q4=99
SPR_M256_EXACT resource=BRDGPICA.SPR mode=best-shift rank=1 offsetA=190 offsetB=197 length=388 beforeBits=190 afterBits=191 leftDiff32=5 rightDiff32=14
SPR_M256_PAIR resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 bitsA=249 bitsB=249 bestShift=30
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 mode=direct shift=0 compared=249 equal=130 mismatch=119 mismatchRuns=56 longestExact=9 exactRuns32=0 leadingMismatchRun=2 trailingMismatchRun=2 centralBits=124 centralMismatch=61 q1=23 q2=33 q3=29 q4=34
SPR_M256_GAP resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 mode=best-shift shift=30 compared=219 equal=149 mismatch=70 mismatchRuns=33 longestExact=45 exactRuns32=1 leadingMismatchRun=0 trailingMismatchRun=0 centralBits=110 centralMismatch=31 q1=30 q2=26 q3=4 q4=10
SPR_M256_EXACT resource=BRDGPICA.SPR mode=best-shift rank=1 offsetA=115 offsetB=145 length=45 beforeBits=115 afterBits=59 leftDiff32=14 rightDiff32=5
SPR_M256_SUMMARY resource=BRDGPICA.SPR inspected=4 expected=4
SPR_M256_NOTE M249 gap coordinates, MSB-first. Direct and best +/-32-bit alignment; exact runs >=32 bits, up to 8 reported per alignment. Quartiles are alignment-local; no codec interpretation.
```
