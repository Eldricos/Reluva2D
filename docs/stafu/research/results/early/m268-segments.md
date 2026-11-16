# m268-segments.txt — curated observations

**Source:** `ResearchArchive.zip` / `m268-segments.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `3d0deda23559fc5f8ce3137cecd02e6c08a535ee6479cc9f3c8716d044d986c9`; **encoding:** `utf-16-le`; **original lines:** 61; **selected lines:** 56.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M268_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M268_SEGMENT resource=BRDGPICA.SPR interval=1 mask=0 segment=1 a=9908 b=9967 bitsA=12 bitsB=22 delta=10
SPR_M268_BITS resource=BRDGPICA.SPR interval=1 mask=0 segment=1 bitsA=100111100111 bitsB=0110110010100111100110
SPR_M268_COMPARE resource=BRDGPICA.SPR interval=1 mask=0 segment=1 prefix=0 suffix=0 longestCommon=11 bestShift=10 bestEqual=11 bestCompared=12
SPR_M268_SCENARIO resource=BRDGPICA.SPR interval=1 mask=0 matched=0 remainingA=12 remainingB=22 monotonic=1
SPR_M268_SEGMENT resource=BRDGPICA.SPR interval=1 mask=1 segment=1 a=9908 b=9967 bitsA=0 bitsB=10 delta=10
SPR_M268_BITS resource=BRDGPICA.SPR interval=1 mask=1 segment=1 bitsA=- bitsB=0110110010
SPR_M268_COMPARE resource=BRDGPICA.SPR interval=1 mask=1 segment=1 prefix=0 suffix=0 longestCommon=0 bestShift=0 bestEqual=0 bestCompared=0
SPR_M268_CONSTRAINT resource=BRDGPICA.SPR interval=1 mask=1 a=9908 b=9977 length=11 displacement=69
SPR_M268_SEGMENT resource=BRDGPICA.SPR interval=1 mask=1 segment=2 a=9919 b=9988 bitsA=1 bitsB=1 delta=0
SPR_M268_BITS resource=BRDGPICA.SPR interval=1 mask=1 segment=2 bitsA=1 bitsB=0
SPR_M268_COMPARE resource=BRDGPICA.SPR interval=1 mask=1 segment=2 prefix=0 suffix=0 longestCommon=0 bestShift=0 bestEqual=0 bestCompared=1
SPR_M268_SCENARIO resource=BRDGPICA.SPR interval=1 mask=1 matched=11 remainingA=1 remainingB=11 monotonic=1
SPR_M268_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=1 mask=0 segment=1 a=11181 b=11127 bitsA=74 bitsB=77 delta=3
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=1 mask=0 segment=1 prefix=0 suffix=0 longestCommon=13 bestShift=3 bestEqual=50 bestCompared=74
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=1 mask=0 matched=0 remainingA=74 remainingB=77 monotonic=1
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=1 mask=1 segment=1 a=11181 b=11127 bitsA=40 bitsB=36 delta=-4
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=1 mask=1 segment=1 prefix=0 suffix=0 longestCommon=10 bestShift=-2 bestEqual=24 bestCompared=36
SPR_M268_CONSTRAINT resource=BRDGDATA.SPR interval=1 mask=1 a=11221 b=11163 length=13 displacement=-58
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=1 mask=1 segment=2 a=11234 b=11176 bitsA=21 bitsB=28 delta=7
SPR_M268_BITS resource=BRDGDATA.SPR interval=1 mask=1 segment=2 bitsA=100000111010000000001 bitsB=0101110100000110010110000000
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=1 mask=1 segment=2 prefix=0 suffix=0 longestCommon=11 bestShift=7 bestEqual=17 bestCompared=21
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=1 mask=1 matched=13 remainingA=61 remainingB=64 monotonic=1
SPR_M268_INTERVAL resource=BRDGDATA.SPR gap=2 interval=2 a=11569 bitsA=76 b=11518 bitsB=79 deltaBits=3
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=0 segment=1 a=11569 b=11518 bitsA=76 bitsB=79 delta=3
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=0 segment=1 prefix=0 suffix=0 longestCommon=13 bestShift=6 bestEqual=52 bestCompared=73
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=2 mask=0 matched=0 remainingA=76 remainingB=79 monotonic=1
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=1 segment=1 a=11569 b=11518 bitsA=4 bitsB=7 delta=3
SPR_M268_BITS resource=BRDGDATA.SPR interval=2 mask=1 segment=1 bitsA=0100 bitsB=1000001
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=1 segment=1 prefix=0 suffix=0 longestCommon=3 bestShift=1 bestEqual=3 bestCompared=4
SPR_M268_CONSTRAINT resource=BRDGDATA.SPR interval=2 mask=1 a=11573 b=11525 length=13 displacement=-48
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=1 segment=2 a=11586 b=11538 bitsA=59 bitsB=59 delta=0
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=1 segment=2 prefix=0 suffix=0 longestCommon=13 bestShift=3 bestEqual=41 bestCompared=56
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=2 mask=1 matched=13 remainingA=63 remainingB=66 monotonic=1
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=2 segment=1 a=11569 b=11518 bitsA=50 bitsB=56 delta=6
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=2 segment=1 prefix=0 suffix=0 longestCommon=13 bestShift=6 bestEqual=31 bestCompared=50
SPR_M268_CONSTRAINT resource=BRDGDATA.SPR interval=2 mask=2 a=11619 b=11574 length=13 displacement=-45
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=2 segment=2 a=11632 b=11587 bitsA=13 bitsB=10 delta=-3
SPR_M268_BITS resource=BRDGDATA.SPR interval=2 mask=2 segment=2 bitsA=1010000011010 bitsB=0110000011
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=2 segment=2 prefix=0 suffix=0 longestCommon=8 bestShift=0 bestEqual=8 bestCompared=10
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=2 mask=2 matched=13 remainingA=63 remainingB=66 monotonic=1
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=3 segment=1 a=11569 b=11518 bitsA=4 bitsB=7 delta=3
SPR_M268_BITS resource=BRDGDATA.SPR interval=2 mask=3 segment=1 bitsA=0100 bitsB=1000001
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=3 segment=1 prefix=0 suffix=0 longestCommon=3 bestShift=1 bestEqual=3 bestCompared=4
SPR_M268_CONSTRAINT resource=BRDGDATA.SPR interval=2 mask=3 a=11573 b=11525 length=13 displacement=-48
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=3 segment=2 a=11586 b=11538 bitsA=33 bitsB=36 delta=3
SPR_M268_BITS resource=BRDGDATA.SPR interval=2 mask=3 segment=2 bitsA=011011000010001001100110110000100 bitsB=110010011001110100001000010100101011
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=3 segment=2 prefix=0 suffix=0 longestCommon=11 bestShift=3 bestEqual=20 bestCompared=33
SPR_M268_CONSTRAINT resource=BRDGDATA.SPR interval=2 mask=3 a=11619 b=11574 length=13 displacement=-45
SPR_M268_SEGMENT resource=BRDGDATA.SPR interval=2 mask=3 segment=3 a=11632 b=11587 bitsA=13 bitsB=10 delta=-3
SPR_M268_BITS resource=BRDGDATA.SPR interval=2 mask=3 segment=3 bitsA=1010000011010 bitsB=0110000011
SPR_M268_COMPARE resource=BRDGDATA.SPR interval=2 mask=3 segment=3 prefix=0 suffix=0 longestCommon=8 bestShift=0 bestEqual=8 bestCompared=10
SPR_M268_SCENARIO resource=BRDGDATA.SPR interval=2 mask=3 matched=26 remainingA=50 remainingB=53 monotonic=1
SPR_M268_SUMMARY intervals=3 scenarios=8 resources=2
SPR_M268_NOTE MSB-first bitstrings of unmatched segments; best shift maximizes equal bits, then overlap. No codec interpretation.
```
