# m283-record2-bit-chain-transition-analysis.txt — curated observations

**Source:** `ResearchArchive.zip` / `m283-record2-bit-chain-transition-analysis.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `54bbad40e9fcf6447c3181889486a58914a4f62e04bcab50ee294121eae9db0c`; **encoding:** `utf-16-le`; **original lines:** 200; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M283_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitsA=960 bitsB=968 chainRuns=5
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=0 spanA=1 spanB=5 deltaBits=4 shiftBefore=0 shiftAfter=4 shiftDelta=4 arithmeticOK=1 terminal=0 bestInternalRun=1 bestInternalShift=1 bestInternalA=0 bestInternalB=1
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=1 bitA=1 bitB=5 length=81 shift=4 leftExtend=0 rightExtend=0 beforeA=1 beforeB=01000 afterA=1100000100110011 afterB=0100001000001010
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=1 spanA=10 spanB=17 deltaBits=7 shiftBefore=4 shiftAfter=11 shiftDelta=7 arithmeticOK=1 terminal=0 bestInternalRun=8 bestInternalShift=5 bestInternalA=1 bestInternalB=6
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=2 bitA=92 bitB=103 length=90 shift=11 leftExtend=0 rightExtend=0 beforeA=1101001100000100 beforeB=1000010000010101 afterA=0011001101010001 afterB=1100110010001001
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=2 spanA=16 spanB=14 deltaBits=-2 shiftBefore=11 shiftAfter=9 shiftDelta=-2 arithmeticOK=1 terminal=0 bestInternalRun=7 bestInternalShift=-2 bestInternalA=2 bestInternalB=0
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=3 bitA=198 bitB=207 length=88 shift=9 leftExtend=0 rightExtend=0 beforeA=0011001101010001 beforeB=1111001100100010 afterA=1000000000101000 afterB=0110010010100100
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=3 spanA=13 spanB=16 deltaBits=3 shiftBefore=9 shiftAfter=12 shiftDelta=3 arithmeticOK=1 terminal=0 bestInternalRun=5 bestInternalShift=-2 bestInternalA=8 bestInternalB=6
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=4 bitA=299 bitB=311 length=72 shift=12 leftExtend=0 rightExtend=0 beforeA=1001000000000101 beforeB=0110010010100100 afterA=1100110010001100 afterB=0011000101000010
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=4 spanA=14 spanB=16 deltaBits=2 shiftBefore=12 shiftAfter=14 shiftDelta=2 arithmeticOK=1 terminal=0 bestInternalRun=6 bestInternalShift=-2 bestInternalA=2 bestInternalB=0
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=5 bitA=385 bitB=399 length=79 shift=14 leftExtend=0 rightExtend=0 beforeA=1111001100100011 beforeB=0011000101000010 afterA=1000100100110100 afterB=0111000000000101
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 transition=5 spanA=496 spanB=490 deltaBits=-6 shiftBefore=14 shiftAfter=8 shiftDelta=-6 arithmeticOK=1 terminal=1 bestInternalRun=105 bestInternalShift=24 bestInternalA=172 bestInternalB=196
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 fromA=192 toA=239 fromB=193 toB=245 bytesA=47 bytesB=52 deltaBytes=5 shiftBefore=1 shiftAfter=6 deltaShift=5 consistent=1 terminal=0
SPR_M283_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 bitsA=376 bitsB=416 chainRuns=1
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 transition=0 spanA=203 spanB=234 deltaBits=31 shiftBefore=0 shiftAfter=31 shiftDelta=31 arithmeticOK=1 terminal=0 bestInternalRun=15 bestInternalShift=27 bestInternalA=134 bestInternalB=161
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 segment=1 bitA=203 bitB=234 length=122 shift=31 leftExtend=0 rightExtend=0 beforeA=1000001010000010 beforeB=0000000000101101 afterA=0011000000000111 afterB=1100110000010110
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 transition=1 spanA=51 spanB=60 deltaBits=9 shiftBefore=31 shiftAfter=40 shiftDelta=9 arithmeticOK=1 terminal=1 bestInternalRun=16 bestInternalShift=11 bestInternalA=28 bestInternalB=39
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 fromA=258 toA=266 fromB=264 toB=271 bytesA=8 bytesB=7 deltaBytes=-1 shiftBefore=6 shiftAfter=5 deltaShift=-1 consistent=1 terminal=0
SPR_M283_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 bitsA=64 bitsB=56 chainRuns=0
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 transition=0 spanA=64 spanB=56 deltaBits=-8 shiftBefore=0 shiftAfter=-8 shiftDelta=-8 arithmeticOK=1 terminal=1 bestInternalRun=28 bestInternalShift=-8 bestInternalA=34 bestInternalB=26
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 fromA=284 toA=322 fromB=289 toB=328 bytesA=38 bytesB=39 deltaBytes=1 shiftBefore=5 shiftAfter=6 deltaShift=1 consistent=1 terminal=0
SPR_M283_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 bitsA=304 bitsB=312 chainRuns=1
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 transition=0 spanA=45 spanB=50 deltaBits=5 shiftBefore=0 shiftAfter=5 shiftDelta=5 arithmeticOK=1 terminal=0 bestInternalRun=14 bestInternalShift=-3 bestInternalA=27 bestInternalB=24
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 segment=1 bitA=45 bitB=50 length=201 shift=5 leftExtend=0 rightExtend=0 beforeA=0101011100111001 beforeB=0011001110100010 afterA=1100110000001110 afterB=0011000000000010
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 transition=1 spanA=58 spanB=61 deltaBits=3 shiftBefore=5 shiftAfter=8 shiftDelta=3 arithmeticOK=1 terminal=1 bestInternalRun=20 bestInternalShift=3 bestInternalA=14 bestInternalB=17
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 fromA=339 toA=385 fromB=345 toB=396 bytesA=46 bytesB=51 deltaBytes=5 shiftBefore=6 shiftAfter=11 deltaShift=5 consistent=1 terminal=0
SPR_M283_BIT_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 bitsA=368 bitsB=408 chainRuns=1
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 transition=0 spanA=76 spanB=93 deltaBits=17 shiftBefore=0 shiftAfter=17 shiftDelta=17 arithmeticOK=1 terminal=0 bestInternalRun=22 bestInternalShift=6 bestInternalA=29 bestInternalB=35
SPR_M283_ANCHOR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 segment=1 bitA=76 bitB=93 length=73 shift=17 leftExtend=0 rightExtend=0 beforeA=1111000000001010 beforeB=1111000000001001 afterA=1111110111000001 afterB=0111011100000111
SPR_M283_TRANSITION resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 transition=1 spanA=219 spanB=242 deltaBits=23 shiftBefore=17 shiftAfter=40 shiftDelta=23 arithmeticOK=1 terminal=1 bestInternalRun=76 bestInternalShift=23 bestInternalA=66 bestInternalB=89
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 fromA=446 toA=446 fromB=457 toB=457 bytesA=0 bytesB=0 deltaBytes=0 shiftBefore=11 shiftAfter=11 deltaShift=0 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 prefix=56 candidates=1 chainSegments=1
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 a=296 b=300 length=24 shift=4 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=left byteA=0x89 byteB=0x49
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=right byteA=0x66 byteB=0x79
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=0 fromA=56 toA=296 fromB=56 toB=300 bytesA=240 bytesB=244 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=1 fromA=320 toA=457 fromB=324 toB=462 bytesA=137 bytesB=138 deltaBytes=1 shiftBefore=4 shiftAfter=5 deltaShift=1 consistent=1 terminal=1
SPR_M283_PAIR resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 prefix=92 candidates=1 chainSegments=1
SPR_M283_BOUNDARY resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 a=93 b=94 length=37 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=left byteA=0x87 byteB=0xA3
SPR_M283_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=right byteA=0x19 byteB=0x18
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=0 fromA=92 toA=93 fromB=92 toB=94 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M283_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=1 fromA=130 toA=462 fromB=131 toB=456 bytesA=332 bytesB=325 deltaBytes=-7 shiftBefore=1 shiftAfter=-6 deltaShift=-7 consistent=1 terminal=1
SPR_M283_RESOURCE resource=BRDGPICA.SPR pairs=8 withMatches=6 candidates=12 chainSegments=12 gaps=20 consistentGaps=20
```
