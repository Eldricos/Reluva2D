# m263-stability.txt — curated observations

**Source:** `ResearchArchive.zip` / `m263-stability.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `d8761aa37922280eec3be856ed7100d28572cf05c6986dc399bed3ceae13e9d0`; **encoding:** `utf-16-le`; **original lines:** 84; **selected lines:** 84.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M263_GAP resource=BRDGPICA.SPR gap=1 record=5 imageA=1 imageB=2 bitsA=1285 bitsB=1292 candidates=21 selected=6 chain=6 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=1 a=3210 b=3243 length=37 relativeShift=3 displacement=33 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=2 a=3273 b=3303 length=71 relativeShift=0 displacement=30 byteAlignedA=0 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=1 to=2 model=unit substitutionCost=1 gapCost=1 score=5 matches=21 substitutions=2 insertions=0 deletions=3 clusters=4 longestMatch=15
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=1 to=2 model=sub2 substitutionCost=2 gapCost=1 score=7 matches=21 substitutions=2 insertions=0 deletions=3 clusters=4 longestMatch=15
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=1 to=2 model=gap2 substitutionCost=1 gapCost=2 score=8 matches=21 substitutions=2 insertions=0 deletions=3 clusters=4 longestMatch=15
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=1 to=2 model=gap3 substitutionCost=1 gapCost=3 score=11 matches=21 substitutions=2 insertions=0 deletions=3 clusters=4 longestMatch=15
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=1 to=2 model=sub3 substitutionCost=3 gapCost=1 score=7 matches=21 substitutions=0 insertions=2 deletions=5 clusters=5 longestMatch=12
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=1 to=2 model=sub2 sharedMatchPairs=21 baselineMatches=21 modelMatches=21 sharedInsertSites=0 sharedDeleteSites=3 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=1 to=2 model=gap2 sharedMatchPairs=21 baselineMatches=21 modelMatches=21 sharedInsertSites=0 sharedDeleteSites=3 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=1 to=2 model=gap3 sharedMatchPairs=21 baselineMatches=21 modelMatches=21 sharedInsertSites=0 sharedDeleteSites=3 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=1 to=2 model=sub3 sharedMatchPairs=17 baselineMatches=21 modelMatches=21 sharedInsertSites=0 sharedDeleteSites=1 sameScript=0
SPR_M263_INTERVAL resource=BRDGPICA.SPR gap=1 from=1 to=2 startA=3247 startB=3280 bitsA=26 bitsB=23 deltaBits=-3
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=3 a=3345 b=3375 length=429 relativeShift=0 displacement=30 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=4 a=3775 b=3805 length=134 relativeShift=0 displacement=30 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=5 a=3910 b=3940 length=462 relativeShift=0 displacement=30 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=1 rank=6 a=4420 b=4457 length=66 relativeShift=7 displacement=37 byteAlignedA=0 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=5 to=6 model=unit substitutionCost=1 gapCost=1 score=14 matches=41 substitutions=7 insertions=7 deletions=0 clusters=12 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=5 to=6 model=sub2 substitutionCost=2 gapCost=1 score=17 matches=43 substitutions=3 insertions=9 deletions=2 clusters=11 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=5 to=6 model=gap2 substitutionCost=1 gapCost=2 score=21 matches=41 substitutions=7 insertions=7 deletions=0 clusters=12 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=5 to=6 model=gap3 substitutionCost=1 gapCost=3 score=28 matches=41 substitutions=7 insertions=7 deletions=0 clusters=12 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=1 from=5 to=6 model=sub3 substitutionCost=3 gapCost=1 score=17 matches=43 substitutions=0 insertions=12 deletions=5 clusters=5 longestMatch=20
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=5 to=6 model=sub2 sharedMatchPairs=31 baselineMatches=41 modelMatches=43 sharedInsertSites=3 sharedDeleteSites=0 sameScript=0
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=5 to=6 model=gap2 sharedMatchPairs=41 baselineMatches=41 modelMatches=41 sharedInsertSites=7 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=5 to=6 model=gap3 sharedMatchPairs=41 baselineMatches=41 modelMatches=41 sharedInsertSites=7 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=1 from=5 to=6 model=sub3 sharedMatchPairs=20 baselineMatches=41 modelMatches=43 sharedInsertSites=0 sharedDeleteSites=0 sameScript=0
SPR_M263_INTERVAL resource=BRDGPICA.SPR gap=1 from=5 to=6 startA=4372 startB=4402 bitsA=48 bitsB=55 deltaBits=7
SPR_M263_GAP resource=BRDGPICA.SPR gap=2 record=5 imageA=1 imageB=2 bitsA=126 bitsB=116 candidates=1 selected=1 chain=1 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGPICA.SPR gap=2 rank=1 a=5028 b=5065 length=55 relativeShift=0 displacement=37 byteAlignedA=0 byteAlignedB=0
SPR_M263_GAP resource=BRDGPICA.SPR gap=3 record=5 imageA=1 imageB=2 bitsA=769 bitsB=811 candidates=6 selected=2 chain=2 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGPICA.SPR gap=3 rank=1 a=9874 b=9933 length=34 relativeShift=-3 displacement=59 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=3 rank=2 a=9920 b=9989 length=388 relativeShift=7 displacement=69 byteAlignedA=1 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=3 from=1 to=2 model=unit substitutionCost=1 gapCost=1 score=11 matches=11 substitutions=1 insertions=10 deletions=0 clusters=2 longestMatch=11
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=3 from=1 to=2 model=sub2 substitutionCost=2 gapCost=1 score=12 matches=11 substitutions=1 insertions=10 deletions=0 clusters=2 longestMatch=11
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=3 from=1 to=2 model=gap2 substitutionCost=1 gapCost=2 score=21 matches=11 substitutions=1 insertions=10 deletions=0 clusters=2 longestMatch=11
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=3 from=1 to=2 model=gap3 substitutionCost=1 gapCost=3 score=31 matches=11 substitutions=1 insertions=10 deletions=0 clusters=2 longestMatch=11
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=3 from=1 to=2 model=sub3 substitutionCost=3 gapCost=1 score=12 matches=11 substitutions=0 insertions=11 deletions=1 clusters=2 longestMatch=11
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=3 from=1 to=2 model=sub2 sharedMatchPairs=11 baselineMatches=11 modelMatches=11 sharedInsertSites=10 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=3 from=1 to=2 model=gap2 sharedMatchPairs=11 baselineMatches=11 modelMatches=11 sharedInsertSites=10 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=3 from=1 to=2 model=gap3 sharedMatchPairs=11 baselineMatches=11 modelMatches=11 sharedInsertSites=10 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=3 from=1 to=2 model=sub3 sharedMatchPairs=11 baselineMatches=11 modelMatches=11 sharedInsertSites=10 sharedDeleteSites=0 sameScript=0
SPR_M263_INTERVAL resource=BRDGPICA.SPR gap=3 from=1 to=2 startA=9908 startB=9967 bitsA=12 bitsB=22 deltaBits=10
SPR_M263_GAP resource=BRDGPICA.SPR gap=4 record=5 imageA=3 imageB=4 bitsA=249 bitsB=249 candidates=3 selected=3 chain=3 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGPICA.SPR gap=4 rank=1 a=6374 b=6509 length=25 relativeShift=13 displacement=135 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGPICA.SPR gap=4 rank=2 a=6457 b=6609 length=45 relativeShift=30 displacement=152 byteAlignedA=0 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=4 from=1 to=2 model=unit substitutionCost=1 gapCost=1 score=23 matches=52 substitutions=6 insertions=17 deletions=0 clusters=16 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=4 from=1 to=2 model=sub2 substitutionCost=2 gapCost=1 score=27 matches=53 substitutions=4 insertions=18 deletions=1 clusters=16 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=4 from=1 to=2 model=gap2 substitutionCost=1 gapCost=2 score=40 matches=52 substitutions=6 insertions=17 deletions=0 clusters=16 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=4 from=1 to=2 model=gap3 substitutionCost=1 gapCost=3 score=57 matches=52 substitutions=6 insertions=17 deletions=0 clusters=16 longestMatch=20
SPR_M263_PROFILE resource=BRDGPICA.SPR gap=4 from=1 to=2 model=sub3 substitutionCost=3 gapCost=1 score=27 matches=53 substitutions=0 insertions=22 deletions=5 clusters=6 longestMatch=31
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=4 from=1 to=2 model=sub2 sharedMatchPairs=42 baselineMatches=52 modelMatches=53 sharedInsertSites=12 sharedDeleteSites=0 sameScript=0
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=4 from=1 to=2 model=gap2 sharedMatchPairs=52 baselineMatches=52 modelMatches=52 sharedInsertSites=17 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=4 from=1 to=2 model=gap3 sharedMatchPairs=52 baselineMatches=52 modelMatches=52 sharedInsertSites=17 sharedDeleteSites=0 sameScript=1
SPR_M263_STABILITY resource=BRDGPICA.SPR gap=4 from=1 to=2 model=sub3 sharedMatchPairs=0 baselineMatches=52 modelMatches=53 sharedInsertSites=0 sharedDeleteSites=0 sameScript=0
SPR_M263_INTERVAL resource=BRDGPICA.SPR gap=4 from=1 to=2 startA=6399 startB=6534 bitsA=58 bitsB=75 deltaBits=17
SPR_M263_SPAN resource=BRDGPICA.SPR gap=4 rank=3 a=6522 b=6674 length=26 relativeShift=30 displacement=152 byteAlignedA=0 byteAlignedB=0
SPR_M263_GAP resource=BRDGDATA.SPR gap=1 record=0 imageA=0 imageB=1 bitsA=417 bitsB=390 candidates=1 selected=1 chain=1 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGDATA.SPR gap=1 rank=1 a=10106 b=10079 length=215 relativeShift=-20 displacement=-27 byteAlignedA=0 byteAlignedB=0
SPR_M263_GAP resource=BRDGDATA.SPR gap=2 record=0 imageA=0 imageB=1 bitsA=1195 bitsB=1205 candidates=5 selected=3 chain=3 minRun=24 shiftRange=32
SPR_M263_SPAN resource=BRDGDATA.SPR gap=2 rank=1 a=10873 b=10819 length=308 relativeShift=-20 displacement=-54 byteAlignedA=0 byteAlignedB=0
SPR_M263_SPAN resource=BRDGDATA.SPR gap=2 rank=2 a=11255 b=11204 length=314 relativeShift=-17 displacement=-51 byteAlignedA=0 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=1 to=2 model=unit substitutionCost=1 gapCost=1 score=21 matches=58 substitutions=14 insertions=5 deletions=2 clusters=17 longestMatch=8
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=1 to=2 model=sub2 substitutionCost=2 gapCost=1 score=29 matches=61 substitutions=7 insertions=9 deletions=6 clusters=17 longestMatch=8
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=1 to=2 model=gap2 substitutionCost=1 gapCost=2 score=28 matches=58 substitutions=14 insertions=5 deletions=2 clusters=17 longestMatch=8
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=1 to=2 model=gap3 substitutionCost=1 gapCost=3 score=33 matches=50 substitutions=24 insertions=3 deletions=0 clusters=15 longestMatch=8
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=1 to=2 model=sub3 substitutionCost=3 gapCost=1 score=29 matches=61 substitutions=0 insertions=16 deletions=13 clusters=19 longestMatch=8
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=1 to=2 model=sub2 sharedMatchPairs=35 baselineMatches=58 modelMatches=61 sharedInsertSites=1 sharedDeleteSites=2 sameScript=0
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=1 to=2 model=gap2 sharedMatchPairs=58 baselineMatches=58 modelMatches=58 sharedInsertSites=5 sharedDeleteSites=2 sameScript=1
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=1 to=2 model=gap3 sharedMatchPairs=17 baselineMatches=58 modelMatches=50 sharedInsertSites=0 sharedDeleteSites=0 sameScript=0
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=1 to=2 model=sub3 sharedMatchPairs=33 baselineMatches=58 modelMatches=61 sharedInsertSites=1 sharedDeleteSites=2 sameScript=0
SPR_M263_INTERVAL resource=BRDGDATA.SPR gap=2 from=1 to=2 startA=11181 startB=11127 bitsA=74 bitsB=77 deltaBits=3
SPR_M263_SPAN resource=BRDGDATA.SPR gap=2 rank=3 a=11645 b=11597 length=283 relativeShift=-14 displacement=-48 byteAlignedA=0 byteAlignedB=0
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=2 to=3 model=unit substitutionCost=1 gapCost=1 score=23 matches=60 substitutions=12 insertions=7 deletions=4 clusters=18 longestMatch=12
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=2 to=3 model=sub2 substitutionCost=2 gapCost=1 score=27 matches=64 substitutions=3 insertions=12 deletions=9 clusters=20 longestMatch=13
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=2 to=3 model=gap2 substitutionCost=1 gapCost=2 score=31 matches=54 substitutions=21 insertions=4 deletions=1 clusters=16 longestMatch=10
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=2 to=3 model=gap3 substitutionCost=1 gapCost=3 score=36 matches=49 substitutions=27 insertions=3 deletions=0 clusters=19 longestMatch=9
SPR_M263_PROFILE resource=BRDGDATA.SPR gap=2 from=2 to=3 model=sub3 substitutionCost=3 gapCost=1 score=27 matches=64 substitutions=0 insertions=15 deletions=12 clusters=21 longestMatch=13
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=2 to=3 model=sub2 sharedMatchPairs=22 baselineMatches=60 modelMatches=64 sharedInsertSites=1 sharedDeleteSites=1 sameScript=0
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=2 to=3 model=gap2 sharedMatchPairs=2 baselineMatches=60 modelMatches=54 sharedInsertSites=0 sharedDeleteSites=0 sameScript=0
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=2 to=3 model=gap3 sharedMatchPairs=2 baselineMatches=60 modelMatches=49 sharedInsertSites=0 sharedDeleteSites=0 sameScript=0
SPR_M263_STABILITY resource=BRDGDATA.SPR gap=2 from=2 to=3 model=sub3 sharedMatchPairs=20 baselineMatches=60 modelMatches=64 sharedInsertSites=1 sharedDeleteSites=1 sameScript=0
SPR_M263_INTERVAL resource=BRDGDATA.SPR gap=2 from=2 to=3 startA=11569 startB=11518 bitsA=76 bitsB=79 deltaBits=3
SPR_M263_SUMMARY gaps=6 resources=2
SPR_M263_NOTE Five global edit-cost models with match=0, diagonal-first ties; compare exact matched coordinate pairs against unit baseline. These are alignment sensitivities, not codec operations.
```
