# spr-bits-troi.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-bits-troi.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `d34db99b1b9217898755186dbe70e54dfc0a71dc9a5f3b8a62ca06253ea81f11`; **encoding:** `utf-16-le`; **original lines:** 13; **selected lines:** 13.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_BITS_SUBSTITUTION resource=BRDGTROI.SPR record=0 imageA=1 imageB=2 block=4 gapBytes=1 offsetA=1314 offsetB=1286 leftMatch=74 rightMatch=26 bytesA=f2 bytesB=3a xor=c8 xorBits=11001000 changedBits=3 changedBytes=1
SPR_BITS_SUBSTITUTION resource=BRDGTROI.SPR record=3 imageA=13 imageB=14 block=2 gapBytes=2 offsetA=192 offsetB=192 leftMatch=25 rightMatch=27 bytesA=2808 bytesB=22e8 xor=0ae0 xorBits=00001010_11100000 changedBits=5 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGTROI.SPR record=3 imageA=17 imageB=18 block=1 gapBytes=6 offsetA=94 offsetB=94 leftMatch=94 rightMatch=25 bytesA=5ed98c1d0cd7 bytesB=65838a9d0c97 xor=3b5a06800040 xorBits=00111011_01011010_00000110_10000000_00000000_01000000 changedBits=13 changedBytes=5
SPR_BITS_SUMMARY resource=BRDGTROI.SPR direct=24 neighborPairs=17 pairsWithMatches=17 anchoredSubstitutions=3 changedBytes=8 changedBits=21 uniquePatterns=3 truncatedPairs=0
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=0 changes=1
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=1 changes=4
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=2 changes=1
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=3 changes=4
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=4 changes=2
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=5 changes=2
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=6 changes=4
SPR_BITS_POSITION resource=BRDGTROI.SPR bit=7 changes=3
SPR_BITS_NOTE XOR bits use bit 0 as least significant within each byte; only 1..8-byte equal-length gaps with >=24-byte anchors are analyzed; no codec semantics inferred.
```
