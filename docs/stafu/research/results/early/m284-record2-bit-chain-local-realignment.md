# m284-record2-bit-chain-local-realignment.txt — curated observations

**Source:** `ResearchArchive.zip` / `m284-record2-bit-chain-local-realignment.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `e81392b17417c3cf46615bbb1ae78ebc496d5ac6308b522b7a431e964e3f68cf`; **encoding:** `utf-16-le`; **original lines:** 271; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M284_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M284_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M284_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M284_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitsA=960 bitsB=968 primaryRuns=5 primaryBits=410
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=0 bitsA=1 bitsB=5 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=1 bitsA=10 bitsB=17 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=2 bitsA=16 bitsB=14 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=3 bitsA=13 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=4 bitsA=14 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=5 bitsA=496 bitsB=490 runs=3 addedBits=280
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=5 segment=1 bitA=514 bitB=552 length=82 localShift=24 globalShift=38
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=5 segment=2 bitA=636 bitB=674 length=105 localShift=24 globalShift=38
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 span=5 segment=3 bitA=783 bitB=829 length=93 localShift=32 globalShift=46
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 primaryBits=410 addedBits=280 combinedBits=690 addedRuns=3
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=0 bitsA=1 bitsB=5 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=1 bitsA=10 bitsB=17 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=2 bitsA=16 bitsB=14 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=3 bitsA=13 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=4 bitsA=14 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=5 bitsA=496 bitsB=490 runs=3 addedBits=280
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=5 segment=1 bitA=514 bitB=552 length=82 localShift=24 globalShift=38
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=5 segment=2 bitA=636 bitB=674 length=105 localShift=24 globalShift=38
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 span=5 segment=3 bitA=783 bitB=829 length=93 localShift=32 globalShift=46
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 primaryBits=410 addedBits=280 combinedBits=690 addedRuns=3
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=0 bitsA=1 bitsB=5 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=1 bitsA=10 bitsB=17 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=2 bitsA=16 bitsB=14 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=3 bitsA=13 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=4 bitsA=14 bitsB=16 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=5 bitsA=496 bitsB=490 runs=1 addedBits=105
SPR_M284_LOCAL_MATCH resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 span=5 segment=1 bitA=636 bitB=674 length=105 localShift=24 globalShift=38
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 primaryBits=410 addedBits=105 combinedBits=515 addedRuns=1
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 fromA=192 toA=239 fromB=193 toB=245 bytesA=47 bytesB=52 deltaBytes=5 shiftBefore=1 shiftAfter=6 deltaShift=5 consistent=1 terminal=0
SPR_M284_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 bitsA=376 bitsB=416 primaryRuns=1 primaryBits=122
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=32 span=0 bitsA=203 bitsB=234 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=32 span=1 bitsA=51 bitsB=60 runs=0 addedBits=0
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=32 primaryBits=122 addedBits=0 combinedBits=122 addedRuns=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=64 span=0 bitsA=203 bitsB=234 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=64 span=1 bitsA=51 bitsB=60 runs=0 addedBits=0
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=64 primaryBits=122 addedBits=0 combinedBits=122 addedRuns=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=96 span=0 bitsA=203 bitsB=234 runs=0 addedBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=96 span=1 bitsA=51 bitsB=60 runs=0 addedBits=0
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=96 primaryBits=122 addedBits=0 combinedBits=122 addedRuns=0
SPR_M284_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 fromA=258 toA=266 fromB=264 toB=271 bytesA=8 bytesB=7 deltaBytes=-1 shiftBefore=6 shiftAfter=5 deltaShift=-1 consistent=1 terminal=0
SPR_M284_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 bitsA=64 bitsB=56 primaryRuns=0 primaryBits=0
SPR_M284_LOCAL_SPAN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 threshold=32 span=0 bitsA=64 bitsB=56 runs=0 addedBits=0
SPR_M284_REFINEMENT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 threshold=32 primaryBits=0 addedBits=0 combinedBits=0 addedRuns=0
```
