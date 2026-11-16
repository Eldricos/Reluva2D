# spr-bits-riker.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-bits-riker.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `b61343659a19a28dc1337ef663604bf11d8d84cbadea5cef788cf5047ddda90d`; **encoding:** `utf-16-le`; **original lines:** 19; **selected lines:** 19.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=2 gapBytes=3 offsetA=1637 offsetB=1694 leftMatch=63 rightMatch=26 bytesA=514016 bytesB=4f4026 xor=1e0030 xorBits=00011110_00000000_00110000 changedBits=6 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=3 gapBytes=3 offsetA=1666 offsetB=1723 leftMatch=26 rightMatch=27 bytesA=145005 bytesB=13500d xor=070008 xorBits=00000111_00000000_00001000 changedBits=4 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=4 gapBytes=2 offsetA=1696 offsetB=1753 leftMatch=27 rightMatch=27 bytesA=f401 bytesB=d402 xor=2003 xorBits=00100000_00000011 changedBits=3 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=5 gapBytes=3 offsetA=1725 offsetB=1782 leftMatch=27 rightMatch=33 bytesA=3d005a bytesB=35009a xor=0800c0 xorBits=00001000_00000000_11000000 changedBits=3 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=6 gapBytes=2 offsetA=1761 offsetB=1818 leftMatch=33 rightMatch=31 bytesA=e802 bytesB=a804 xor=4006 xorBits=01000000_00000110 changedBits=3 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=7 gapBytes=3 offsetA=1794 offsetB=1851 leftMatch=31 rightMatch=35 bytesA=3d005a bytesB=35009a xor=0800c0 xorBits=00001000_00000000_11000000 changedBits=3 changedBytes=2
SPR_BITS_SUBSTITUTION resource=BRDGRIKR.SPR record=3 imageA=3 imageB=4 block=8 gapBytes=2 offsetA=1832 offsetB=1889 leftMatch=35 rightMatch=37 bytesA=f401 bytesB=d402 xor=2003 xorBits=00100000_00000011 changedBits=3 changedBytes=2
SPR_BITS_SUMMARY resource=BRDGRIKR.SPR direct=35 neighborPairs=28 pairsWithMatches=16 anchoredSubstitutions=7 changedBytes=14 changedBits=25 uniquePatterns=5 truncatedPairs=0
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=0 changes=3
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=1 changes=5
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=2 changes=3
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=3 changes=4
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=4 changes=2
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=5 changes=3
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=6 changes=3
SPR_BITS_POSITION resource=BRDGRIKR.SPR bit=7 changes=2
SPR_BITS_REPEAT resource=BRDGRIKR.SPR count=2 bytesA=3d005a bytesB=35009a
SPR_BITS_REPEAT resource=BRDGRIKR.SPR count=2 bytesA=f401 bytesB=d402
SPR_BITS_NOTE XOR bits use bit 0 as least significant within each byte; only 1..8-byte equal-length gaps with >=24-byte anchors are analyzed; no codec semantics inferred.
```
