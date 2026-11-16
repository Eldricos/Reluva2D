# m274-alternative-chain-inspection.txt — curated observations

**Source:** `ResearchArchive.zip` / `m274-alternative-chain-inspection.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `0333d38fde041f7242c5d3e219f254d25f955b38855b41d6d0a905c7688607a8`; **encoding:** `utf-16-le`; **original lines:** 13; **selected lines:** 13.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M274_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M274_ANCHOR resource=BRDGPICA.SPR interval=1 anchor=1 a=9908 b=9977 length=11 displacement=69 fullOccurrencesA=26 fullOccurrencesB=26 surround256A=1 surround256B=1 fullPairs=676 sameDisplacementPairs=1
SPR_M274_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M274_ANCHOR resource=BRDGDATA.SPR interval=1 anchor=1 a=11221 b=11163 length=13 displacement=-58 fullOccurrencesA=3 fullOccurrencesB=4 surround256A=1 surround256B=1 fullPairs=12 sameDisplacementPairs=1
SPR_M274_INTERVAL resource=BRDGDATA.SPR gap=2 interval=2 a=11569 bitsA=76 b=11518 bitsB=79 deltaBits=3
SPR_M274_ANCHOR resource=BRDGDATA.SPR interval=2 anchor=1 a=11573 b=11525 length=13 displacement=-48 fullOccurrencesA=11 fullOccurrencesB=10 surround256A=1 surround256B=1 fullPairs=110 sameDisplacementPairs=1
SPR_M274_ANCHOR resource=BRDGDATA.SPR interval=2 anchor=2 a=11619 b=11574 length=13 displacement=-45 fullOccurrencesA=10 fullOccurrencesB=7 surround256A=2 surround256B=2 fullPairs=70 sameDisplacementPairs=1
SPR_M274_CHAIN_CANDIDATE resource=BRDGDATA.SPR interval=2 a1=10785 b1=11525 a2=10834 b2=11574 displacement1=740 displacement2=740 spacingA=49 spacingB=49 spacingDeltaA=3 spacingDeltaB=0 exact=0 near8=1 constantDisplacement=1 original=0 overlapsOriginalA1=0 overlapsOriginalB1=1 overlapsOriginalA2=0 overlapsOriginalB2=1
SPR_M274_CHAIN_CANDIDATE resource=BRDGDATA.SPR interval=2 a1=11573 b1=11525 a2=11619 b2=11574 displacement1=-48 displacement2=-45 spacingA=46 spacingB=49 spacingDeltaA=0 spacingDeltaB=0 exact=1 near8=1 constantDisplacement=0 original=1 overlapsOriginalA1=1 overlapsOriginalB1=1 overlapsOriginalA2=1 overlapsOriginalB2=1
SPR_M274_CHAIN_CANDIDATE resource=BRDGDATA.SPR interval=2 a1=11573 b1=11525 a2=11849 b2=11801 displacement1=-48 displacement2=-48 spacingA=276 spacingB=276 spacingDeltaA=230 spacingDeltaB=227 exact=0 near8=0 constantDisplacement=1 original=0 overlapsOriginalA1=1 overlapsOriginalB1=1 overlapsOriginalA2=0 overlapsOriginalB2=0
SPR_M274_CHAIN_SUMMARY resource=BRDGDATA.SPR interval=2 orderedPairs=300 exactSpacingPairs=1 near8SpacingPairs=2 constantDisplacementPairs=2 originalExactPairs=1
SPR_M274_SUMMARY intervals=3 scenarios=8 resources=2
SPR_M274_NOTE Enumerated exact/near8/constant-shift chains; verified unique original exact-spacing chain. No codec interpretation.
```
