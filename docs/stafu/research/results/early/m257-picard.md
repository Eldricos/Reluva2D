# m257-picard.txt — curated observations

**Source:** `ResearchArchive.zip` / `m257-picard.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `227ae8146d1182b607e2ddf856528b03c7b631e8a809ee461419d4eb2fc49527`; **encoding:** `utf-16-le`; **original lines:** 30; **selected lines:** 16.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M257_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bestShift=0 compared=1285 exactStart=706 exactLength=462 absoluteA=3910 absoluteB=3940
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=left requested=32 available=706 compared=32 mismatch=1 patternA=01000110110010110111100100010011 patternB=01000110110010110111100100010010 comparison================================X
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=right requested=32 available=117 compared=32 mismatch=18 patternA=11000010110110010100000000100111 patternB=00001010111100001011011001001000 comparison=XX==X=====X=X==XXXXX=XX==XX=XXXX
SPR_M257_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bestShift=0 compared=116 exactStart=61 exactLength=55 absoluteA=5028 absoluteB=5065
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=left requested=32 available=61 compared=32 mismatch=10 patternA=11000000001001111011000100001111 patternB=00010100000001111011000100010000 comparison=XX=X=X====X================XXXXX
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=right requested=32 available=0 compared=0 mismatch=0 patternA= patternB= comparison=
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=right requested=64 available=0 compared=0 mismatch=0 patternA= patternB= comparison=
SPR_M257_TRANSITION resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=0 side=end offset=116 leftBits=32 rightBits=0 patternA=00101101100110110110100011011001 patternB=00101101100110110110100011011001
SPR_M257_PAIR resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 bestShift=7 compared=769 exactStart=190 exactLength=388 absoluteA=9920 absoluteB=9989
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=7 side=left requested=32 available=190 compared=32 mismatch=5 patternA=01101101000110110011100111100111 patternB=01101100110110110010100111100110 comparison========XXX=========X===========X
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=1 imageB=2 shift=7 side=right requested=32 available=191 compared=32 mismatch=14 patternA=00000111101001100101001101110100 patternB=10000101101110001000101100110011 comparison=X=====X====XXXX=XX=XX====X===XXX
SPR_M257_PAIR resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 bestShift=30 compared=219 exactStart=115 exactLength=45 absoluteA=6457 absoluteB=6609
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 shift=30 side=left requested=32 available=115 compared=32 mismatch=14 patternA=00000111010101100011011000000001 patternB=00000000110010101100011010100000 comparison======XXXX==XXX==XXXX====X=X====X
SPR_M257_FLANK resource=BRDGPICA.SPR record=5 imageA=3 imageB=4 shift=30 side=right requested=32 available=59 compared=32 mismatch=5 patternA=00000001100111100110001100001011 patternB=11100001100111100101001100001011 comparison=XXX===============XX============
SPR_M257_SUMMARY resource=BRDGPICA.SPR inspected=4 expected=4
SPR_M257_NOTE M249 gap coordinates, MSB-first. Best +/-32-bit alignment uses M256 ranking. Flanks clipped to gap bounds; exact-run boundaries are observational, not codec fields.
```
