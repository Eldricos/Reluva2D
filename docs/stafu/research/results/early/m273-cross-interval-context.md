# m273-cross-interval-context.txt — curated observations

**Source:** `ResearchArchive.zip` / `m273-cross-interval-context.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `e6b26ddc611effdc3ac2b759561a7f075dc792d7ac28ec18574f90eb9450e921`; **encoding:** `utf-16-le`; **original lines:** 10; **selected lines:** 10.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M273_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M273_ANCHOR resource=BRDGPICA.SPR interval=1 anchor=1 a=9908 b=9977 length=11 displacement=69 fullOccurrencesA=26 fullOccurrencesB=26 surround256A=1 surround256B=1 fullPairs=676 sameDisplacementPairs=1
SPR_M273_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M273_ANCHOR resource=BRDGDATA.SPR interval=1 anchor=1 a=11221 b=11163 length=13 displacement=-58 fullOccurrencesA=3 fullOccurrencesB=4 surround256A=1 surround256B=1 fullPairs=12 sameDisplacementPairs=1
SPR_M273_INTERVAL resource=BRDGDATA.SPR gap=2 interval=2 a=11569 bitsA=76 b=11518 bitsB=79 deltaBits=3
SPR_M273_ANCHOR resource=BRDGDATA.SPR interval=2 anchor=1 a=11573 b=11525 length=13 displacement=-48 fullOccurrencesA=11 fullOccurrencesB=10 surround256A=1 surround256B=1 fullPairs=110 sameDisplacementPairs=1
SPR_M273_ANCHOR resource=BRDGDATA.SPR interval=2 anchor=2 a=11619 b=11574 length=13 displacement=-45 fullOccurrencesA=10 fullOccurrencesB=7 surround256A=2 surround256B=2 fullPairs=70 sameDisplacementPairs=1
SPR_M273_CHAIN resource=BRDGDATA.SPR interval=2 anchors=2 expectedSpacingA=46 expectedSpacingB=49 orderedPairs=300 exactSpacingPairs=1 near8SpacingPairs=2 constantDisplacementPairs=2
SPR_M273_SUMMARY intervals=3 scenarios=8 resources=2
SPR_M273_NOTE Whole-payload and 256-bit-surround occurrence counts; two-anchor ordered chain alternatives. No codec interpretation.
```
