# m266-short-matches.txt — curated observations

**Source:** `ResearchArchive.zip` / `m266-short-matches.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `931509804247e284cc2bf554a091785b0dcd5a09133acbc2ed82fa3c49a93143`; **encoding:** `utf-16-le`; **original lines:** 16; **selected lines:** 16.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M266_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M266_CANDIDATES resource=BRDGPICA.SPR interval=1 threshold=4 total=5 reported=5
SPR_M266_HIT resource=BRDGPICA.SPR interval=1 rank=1 a=9908 b=9977 length=11 displacement=69 fullA=26 fullB=26 intervalA=1 intervalB=1 alternativePairs=0 uniqueBoth=0
SPR_M266_HIT resource=BRDGPICA.SPR interval=1 rank=2 a=9914 b=9977 length=6 displacement=63 fullA=626 fullB=669 intervalA=2 intervalB=1 alternativePairs=1 uniqueBoth=0
SPR_M266_HIT resource=BRDGPICA.SPR interval=1 rank=3 a=9908 b=9983 length=5 displacement=75 fullA=2437 fullB=2522 intervalA=2 intervalB=2 alternativePairs=3 uniqueBoth=0
SPR_M266_HIT resource=BRDGPICA.SPR interval=1 rank=4 a=9913 b=9971 length=5 displacement=58 fullA=2670 fullB=2797 intervalA=1 intervalB=2 alternativePairs=1 uniqueBoth=0
SPR_M266_HIT resource=BRDGPICA.SPR interval=1 rank=5 a=9908 b=9972 length=4 displacement=64 fullA=5820 fullB=6070 intervalA=2 intervalB=3 alternativePairs=5 uniqueBoth=0
SPR_M266_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M266_CANDIDATES resource=BRDGDATA.SPR interval=1 threshold=13 total=1 reported=1
SPR_M266_HIT resource=BRDGDATA.SPR interval=1 rank=1 a=11221 b=11163 length=13 displacement=-58 fullA=3 fullB=4 intervalA=1 intervalB=1 alternativePairs=0 uniqueBoth=0
SPR_M266_INTERVAL resource=BRDGDATA.SPR gap=2 interval=2 a=11569 bitsA=76 b=11518 bitsB=79 deltaBits=3
SPR_M266_CANDIDATES resource=BRDGDATA.SPR interval=2 threshold=13 total=2 reported=2
SPR_M266_HIT resource=BRDGDATA.SPR interval=2 rank=1 a=11573 b=11525 length=13 displacement=-48 fullA=11 fullB=10 intervalA=1 intervalB=1 alternativePairs=0 uniqueBoth=0
SPR_M266_HIT resource=BRDGDATA.SPR interval=2 rank=2 a=11619 b=11574 length=13 displacement=-45 fullA=10 fullB=7 intervalA=1 intervalB=1 alternativePairs=0 uniqueBoth=0
SPR_M266_SUMMARY intervals=3 resources=2
SPR_M266_NOTE MSB-first maximal diagonal runs; Picard threshold=4, Data threshold=13. Counts include overlapping occurrences; top 20 hits per interval. Uniqueness is descriptive, not a probability or codec operation.
```
