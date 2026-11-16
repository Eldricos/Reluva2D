# m255-picard.txt — curated observations

**Source:** `ResearchArchive.zip` / `m255-picard.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `9c53a5953780e380c1b1927e9308c48367891b83cece1edec5e7b7c612849a9e`; **encoding:** `utf-16-le`; **original lines:** 40; **selected lines:** 32.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M255_BOUNDARY offset=69 absoluteA=3273 absoluteB=3303 beforeEqual=0 afterEqual=1
SPR_M255_WINDOW offset=69 flank=8 differing=1 patternA=1000111010000000 patternB=1000111110000000 comparison========X======== repeatsA=1 repeatsB=1 crossAinB=4 crossBinA=1
SPR_M255_WINDOW offset=69 flank=16 differing=1 patternA=00110101100011101000000001001101 patternB=00110101100011111000000001001101 comparison================X================ repeatsA=0 repeatsB=0 crossAinB=1 crossBinA=0
SPR_M255_BOUNDARY offset=140 absoluteA=3344 absoluteB=3374 beforeEqual=1 afterEqual=0
SPR_M255_WINDOW offset=140 flank=8 differing=1 patternA=1110011010110000 patternB=1110011000110000 comparison=========X======= repeatsA=15 repeatsB=1 crossAinB=16 crossBinA=1
SPR_M255_WINDOW offset=140 flank=16 differing=1 patternA=00101001111001101011000010111011 patternB=00101001111001100011000010111011 comparison=================X=============== repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=141 absoluteA=3345 absoluteB=3375 beforeEqual=0 afterEqual=1
SPR_M255_WINDOW offset=141 flank=8 differing=1 patternA=1100110101100001 patternB=1100110001100001 comparison========X======== repeatsA=3 repeatsB=0 crossAinB=2 crossBinA=1
SPR_M255_WINDOW offset=141 flank=16 differing=1 patternA=01010011110011010110000101110111 patternB=01010011110011000110000101110111 comparison================X================ repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=570 absoluteA=3774 absoluteB=3804 beforeEqual=1 afterEqual=0
SPR_M255_WINDOW offset=570 flank=8 differing=1 patternA=1110010100101111 patternB=1110010110101111 comparison=========X======= repeatsA=3 repeatsB=1 crossAinB=4 crossBinA=1
SPR_M255_WINDOW offset=570 flank=16 differing=1 patternA=01011001111001010010111110001101 patternB=01011001111001011010111110001101 comparison=================X=============== repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=571 absoluteA=3775 absoluteB=3805 beforeEqual=0 afterEqual=1
SPR_M255_WINDOW offset=571 flank=8 differing=1 patternA=1100101001011111 patternB=1100101101011111 comparison========X======== repeatsA=0 repeatsB=1 crossAinB=1 crossBinA=3
SPR_M255_WINDOW offset=571 flank=16 differing=1 patternA=10110011110010100101111100011011 patternB=10110011110010110101111100011011 comparison================X================ repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=705 absoluteA=3909 absoluteB=3939 beforeEqual=1 afterEqual=0
SPR_M255_WINDOW offset=705 flank=8 differing=1 patternA=1000100110110000 patternB=1000100100110000 comparison=========X======= repeatsA=0 repeatsB=1 crossAinB=1 crossBinA=0
SPR_M255_WINDOW offset=705 flank=16 differing=1 patternA=10111100100010011011000010110011 patternB=10111100100010010011000010110011 comparison=================X=============== repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=706 absoluteA=3910 absoluteB=3940 beforeEqual=0 afterEqual=1
SPR_M255_WINDOW offset=706 flank=8 differing=1 patternA=0001001101100001 patternB=0001001001100001 comparison========X======== repeatsA=0 repeatsB=1 crossAinB=1 crossBinA=0
SPR_M255_WINDOW offset=706 flank=16 differing=1 patternA=01111001000100110110000101100111 patternB=01111001000100100110000101100111 comparison================X================ repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_BOUNDARY offset=1168 absoluteA=4372 absoluteB=4402 beforeEqual=1 afterEqual=0
SPR_M255_WINDOW offset=1168 flank=8 differing=3 patternA=1100101111000010 patternB=1100101100001010 comparison=========XX==X=== repeatsA=4 repeatsB=12 crossAinB=2 crossBinA=11
SPR_M255_WINDOW offset=1168 flank=16 differing=6 patternA=10101011110010111100001011011001 patternB=10101011110010110000101011110000 comparison=================XX==X=====X=X==X repeatsA=0 repeatsB=0 crossAinB=0 crossBinA=0
SPR_M255_REGION index=1 start=0 end=69 compared=69 differing=25
SPR_M255_REGION index=2 start=69 end=140 compared=71 differing=0
SPR_M255_REGION index=3 start=141 end=570 compared=429 differing=0
SPR_M255_REGION index=4 start=571 end=705 compared=134 differing=0
SPR_M255_REGION index=5 start=706 end=1168 compared=462 differing=0
SPR_M255_REGION index=6 start=1168 end=1285 compared=117 differing=63
SPR_M255_SUMMARY boundaries=8 widths=8,16,32 regions=6
SPR_M255_NOTE MSB-first zero-based payload-relative positions; windows span each observed transition. Recurrences search full direct payloads, excluding self. Boundaries are observational, not codec fields.
```
