# m281-record2-unmatched-interval-bit-analysis.txt — curated observations

**Source:** `ResearchArchive.zip` / `m281-record2-unmatched-interval-bit-analysis.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `2d992fecf2be19415e6f6253259927bba1fed3ad2720bdd27f9528274ba84cdd`; **encoding:** `utf-16-le`; **original lines:** 152; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitsA=960 bitsB=968 deltaBits=8 bitPrefix=0 bitSuffix=0 bestShiftedRun=90 bestShift=11 bestBitA=92 bestBitB=103 nonByteAligned=1 prefixSuffixMin=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 fromA=192 toA=239 fromB=193 toB=245 bytesA=47 bytesB=52 deltaBytes=5 shiftBefore=1 shiftAfter=6 deltaShift=5 consistent=1 terminal=0
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 bitsA=376 bitsB=416 deltaBits=40 bitPrefix=5 bitSuffix=6 bestShiftedRun=122 bestShift=31 bestBitA=203 bestBitB=234 nonByteAligned=1 prefixSuffixMin=5
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 fromA=258 toA=266 fromB=264 toB=271 bytesA=8 bytesB=7 deltaBytes=-1 shiftBefore=6 shiftAfter=5 deltaShift=-1 consistent=1 terminal=0
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 bitsA=64 bitsB=56 deltaBits=-8 bitPrefix=6 bitSuffix=1 bestShiftedRun=28 bestShift=-8 bestBitA=34 bestBitB=26 nonByteAligned=0 prefixSuffixMin=1
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 fromA=284 toA=322 fromB=289 toB=328 bytesA=38 bytesB=39 deltaBytes=1 shiftBefore=5 shiftAfter=6 deltaShift=1 consistent=1 terminal=0
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 bitsA=304 bitsB=312 deltaBits=8 bitPrefix=7 bitSuffix=1 bestShiftedRun=201 bestShift=5 bestBitA=45 bestBitB=50 nonByteAligned=1 prefixSuffixMin=1
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 fromA=339 toA=385 fromB=345 toB=396 bytesA=46 bytesB=51 deltaBytes=5 shiftBefore=6 shiftAfter=11 deltaShift=5 consistent=1 terminal=0
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 bitsA=368 bitsB=408 deltaBits=40 bitPrefix=1 bitSuffix=3 bestShiftedRun=73 bestShift=17 bestBitA=76 bestBitB=93 nonByteAligned=1 prefixSuffixMin=1
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 fromA=446 toA=446 fromB=457 toB=457 bytesA=0 bytesB=0 deltaBytes=0 shiftBefore=11 shiftAfter=11 deltaShift=0 consistent=1 terminal=1
SPR_M281_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 bitsA=0 bitsB=0 deltaBits=0 bitPrefix=0 bitSuffix=0 bestShiftedRun=0 bestShift=0 bestBitA=0 bestBitB=0 nonByteAligned=0 prefixSuffixMin=0
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 prefix=56 candidates=1 chainSegments=1
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 a=296 b=300 length=24 shift=4 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=left byteA=0x89 byteB=0x49
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=right byteA=0x66 byteB=0x79
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=0 fromA=56 toA=296 fromB=56 toB=300 bytesA=240 bytesB=244 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=1 fromA=320 toA=457 fromB=324 toB=462 bytesA=137 bytesB=138 deltaBytes=1 shiftBefore=4 shiftAfter=5 deltaShift=1 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 prefix=92 candidates=1 chainSegments=1
SPR_M281_BOUNDARY resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 a=93 b=94 length=37 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=left byteA=0x87 byteB=0xA3
SPR_M281_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=right byteA=0x19 byteB=0x18
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=0 fromA=92 toA=93 fromB=92 toB=94 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M281_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=1 fromA=130 toA=462 fromB=131 toB=456 bytesA=332 bytesB=325 deltaBytes=-7 shiftBefore=1 shiftAfter=-6 deltaShift=-7 consistent=1 terminal=1
SPR_M281_RESOURCE resource=BRDGPICA.SPR pairs=8 withMatches=6 candidates=12 chainSegments=12 gaps=20 consistentGaps=20
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=1 ordinalB=2 prefix=48 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=48 toA=246 fromB=48 toB=249 bytesA=198 bytesB=201 deltaBytes=3 shiftBefore=0 shiftAfter=3 deltaShift=3 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=2 ordinalB=3 prefix=84 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=84 toA=249 fromB=84 toB=255 bytesA=165 bytesB=171 deltaBytes=6 shiftBefore=0 shiftAfter=6 deltaShift=6 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=3 ordinalB=4 prefix=84 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=84 toA=255 fromB=84 toB=256 bytesA=171 bytesB=172 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=4 ordinalB=5 prefix=84 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=84 toA=256 fromB=84 toB=255 bytesA=172 bytesB=171 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=5 ordinalB=6 prefix=84 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=84 toA=255 fromB=84 toB=252 bytesA=171 bytesB=168 deltaBytes=-3 shiftBefore=0 shiftAfter=-3 deltaShift=-3 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=6 ordinalB=7 prefix=48 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=48 toA=252 fromB=48 toB=243 bytesA=204 bytesB=195 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=7 ordinalB=8 prefix=48 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=7 ordinalB=8 gap=0 fromA=48 toA=243 fromB=48 toB=256 bytesA=195 bytesB=208 deltaBytes=13 shiftBefore=0 shiftAfter=13 deltaShift=13 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGDATA.SPR ordinalA=8 ordinalB=9 prefix=68 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGDATA.SPR ordinalA=8 ordinalB=9 gap=0 fromA=68 toA=256 fromB=68 toB=258 bytesA=188 bytesB=190 deltaBytes=2 shiftBefore=0 shiftAfter=2 deltaShift=2 consistent=1 terminal=1
SPR_M281_RESOURCE resource=BRDGDATA.SPR pairs=8 withMatches=0 candidates=0 chainSegments=0 gaps=8 consistentGaps=8
SPR_M281_PAIR resource=BRDGRIKR.SPR ordinalA=1 ordinalB=2 prefix=147 candidates=0 chainSegments=0
SPR_M281_GAP resource=BRDGRIKR.SPR ordinalA=1 ordinalB=2 gap=0 fromA=147 toA=357 fromB=147 toB=360 bytesA=210 bytesB=213 deltaBytes=3 shiftBefore=0 shiftAfter=3 deltaShift=3 consistent=1 terminal=1
SPR_M281_PAIR resource=BRDGRIKR.SPR ordinalA=2 ordinalB=3 prefix=137 candidates=1 chainSegments=1
```
