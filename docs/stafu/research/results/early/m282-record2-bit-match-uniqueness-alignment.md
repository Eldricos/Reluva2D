# m282-record2-bit-match-uniqueness-alignment.txt — curated observations

**Source:** `ResearchArchive.zip` / `m282-record2-bit-match-uniqueness-alignment.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `a519363640dcf809853e8077281d617e416066be8ce7644292c3435ae4e15b78`; **encoding:** `utf-16-le`; **original lines:** 211; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 bitsA=960 bitsB=968 candidates=5 chainRuns=5 chainBits=410 nonByteChainRuns=5 strongest=90 strongestShift=11 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 segment=1 bitA=1 bitB=5 length=81 shift=4
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 segment=2 bitA=92 bitB=103 length=90 shift=11
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 segment=3 bitA=198 bitB=207 length=88 shift=9
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 segment=4 bitA=299 bitB=311 length=72 shift=12
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=32 segment=5 bitA=385 bitB=399 length=79 shift=14
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 bitsA=960 bitsB=968 candidates=5 chainRuns=5 chainBits=410 nonByteChainRuns=5 strongest=90 strongestShift=11 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 segment=1 bitA=1 bitB=5 length=81 shift=4
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 segment=2 bitA=92 bitB=103 length=90 shift=11
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 segment=3 bitA=198 bitB=207 length=88 shift=9
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 segment=4 bitA=299 bitB=311 length=72 shift=12
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=64 segment=5 bitA=385 bitB=399 length=79 shift=14
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 threshold=96 bitsA=960 bitsB=968 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 fromA=192 toA=239 fromB=193 toB=245 bytesA=47 bytesB=52 deltaBytes=5 shiftBefore=1 shiftAfter=6 deltaShift=5 consistent=1 terminal=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=32 bitsA=376 bitsB=416 candidates=1 chainRuns=1 chainBits=122 nonByteChainRuns=1 strongest=122 strongestShift=31 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=32 segment=1 bitA=203 bitB=234 length=122 shift=31
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=64 bitsA=376 bitsB=416 candidates=1 chainRuns=1 chainBits=122 nonByteChainRuns=1 strongest=122 strongestShift=31 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=64 segment=1 bitA=203 bitB=234 length=122 shift=31
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=96 bitsA=376 bitsB=416 candidates=1 chainRuns=1 chainBits=122 nonByteChainRuns=1 strongest=122 strongestShift=31 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 threshold=96 segment=1 bitA=203 bitB=234 length=122 shift=31
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 fromA=258 toA=266 fromB=264 toB=271 bytesA=8 bytesB=7 deltaBytes=-1 shiftBefore=6 shiftAfter=5 deltaShift=-1 consistent=1 terminal=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 threshold=32 bitsA=64 bitsB=56 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 threshold=64 bitsA=64 bitsB=56 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 threshold=96 bitsA=64 bitsB=56 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 fromA=284 toA=322 fromB=289 toB=328 bytesA=38 bytesB=39 deltaBytes=1 shiftBefore=5 shiftAfter=6 deltaShift=1 consistent=1 terminal=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=32 bitsA=304 bitsB=312 candidates=2 chainRuns=1 chainBits=201 nonByteChainRuns=1 strongest=201 strongestShift=5 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=32 segment=1 bitA=45 bitB=50 length=201 shift=5
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=64 bitsA=304 bitsB=312 candidates=1 chainRuns=1 chainBits=201 nonByteChainRuns=1 strongest=201 strongestShift=5 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=64 segment=1 bitA=45 bitB=50 length=201 shift=5
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=96 bitsA=304 bitsB=312 candidates=1 chainRuns=1 chainBits=201 nonByteChainRuns=1 strongest=201 strongestShift=5 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 threshold=96 segment=1 bitA=45 bitB=50 length=201 shift=5
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 fromA=339 toA=385 fromB=345 toB=396 bytesA=46 bytesB=51 deltaBytes=5 shiftBefore=6 shiftAfter=11 deltaShift=5 consistent=1 terminal=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 threshold=32 bitsA=368 bitsB=408 candidates=1 chainRuns=1 chainBits=73 nonByteChainRuns=1 strongest=73 strongestShift=17 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 threshold=32 segment=1 bitA=76 bitB=93 length=73 shift=17
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 threshold=64 bitsA=368 bitsB=408 candidates=1 chainRuns=1 chainBits=73 nonByteChainRuns=1 strongest=73 strongestShift=17 occurrencesA=1 occurrencesB=1
SPR_M282_CHAIN resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 threshold=64 segment=1 bitA=76 bitB=93 length=73 shift=17
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 threshold=96 bitsA=368 bitsB=408 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 fromA=446 toA=446 fromB=457 toB=457 bytesA=0 bytesB=0 deltaBytes=0 shiftBefore=11 shiftAfter=11 deltaShift=0 consistent=1 terminal=1
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 threshold=32 bitsA=0 bitsB=0 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 threshold=64 bitsA=0 bitsB=0 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 threshold=96 bitsA=0 bitsB=0 candidates=0 chainRuns=0 chainBits=0 nonByteChainRuns=0 strongest=0 strongestShift=0 occurrencesA=0 occurrencesB=0
SPR_M282_PAIR resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 prefix=56 candidates=1 chainSegments=1
SPR_M282_BOUNDARY resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 a=296 b=300 length=24 shift=4 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M282_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=left byteA=0x89 byteB=0x49
```
