# m287-record2-anchor-conflict-interval-analysis.txt — curated observations

**Source:** `ResearchArchive.zip` / `m287-record2-anchor-conflict-interval-analysis.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `7155e35d26d958f6020d10ed8796d7541cd2e10ec8f35ff279dca91b0fb2e585`; **encoding:** `utf-16-le`; **original lines:** 218; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M287_BASELINE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitsA=960 bitsB=968 m282Bits=410 m284Bits=690 m285Bits=690 m286Bits=690 m284Anchors=8 retainedM284=8
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=1 kind=leading fromA=0 toA=1 fromB=0 toB=5 bitsA=1 bitsB=5 deltaBits=4 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=2 kind=internal fromA=82 toA=92 fromB=86 toB=103 bitsA=10 bitsB=17 deltaBits=7 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=3 kind=internal fromA=182 toA=198 fromB=193 toB=207 bitsA=16 bitsB=14 deltaBits=-2 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=4 kind=internal fromA=286 toA=299 fromB=295 toB=311 bitsA=13 bitsB=16 deltaBits=3 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=5 kind=internal fromA=371 toA=385 fromB=383 toB=399 bitsA=14 bitsB=16 deltaBits=2 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=6 kind=internal fromA=464 toA=514 fromB=478 toB=552 bitsA=50 bitsB=74 deltaBits=24 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=7 kind=internal fromA=596 toA=636 fromB=634 toB=674 bitsA=40 bitsB=40 deltaBits=0 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=8 kind=internal fromA=741 toA=783 fromB=779 toB=829 bitsA=42 bitsB=50 deltaBits=8 unconstrainedContained=0
SPR_M287_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=9 kind=trailing fromA=876 toA=960 fromB=922 toB=968 bitsA=84 bitsB=46 deltaBits=-38 unconstrainedContained=0
SPR_M287_COMPARISON resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 radius=128 threshold=32 unconstrainedBits=690 preservedBits=690 deltaBits=0 unconstrainedRuns=8 preservedRuns=8 competing=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=1 fromA=192 toA=239 fromB=193 toB=245 bytesA=47 bytesB=52 deltaBytes=5 shiftBefore=1 shiftAfter=6 deltaShift=5 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=2 fromA=258 toA=266 fromB=264 toB=271 bytesA=8 bytesB=7 deltaBytes=-1 shiftBefore=6 shiftAfter=5 deltaShift=-1 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=3 fromA=284 toA=322 fromB=289 toB=328 bytesA=38 bytesB=39 deltaBytes=1 shiftBefore=5 shiftAfter=6 deltaShift=1 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=4 fromA=339 toA=385 fromB=345 toB=396 bytesA=46 bytesB=51 deltaBytes=5 shiftBefore=6 shiftAfter=11 deltaShift=5 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=5 fromA=446 toA=446 fromB=457 toB=457 bytesA=0 bytesB=0 deltaBytes=0 shiftBefore=11 shiftAfter=11 deltaShift=0 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 prefix=56 candidates=1 chainSegments=1
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 a=296 b=300 length=24 shift=4 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=left byteA=0x89 byteB=0x49
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 candidate=1 side=right byteA=0x66 byteB=0x79
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=0 fromA=56 toA=296 fromB=56 toB=300 bytesA=240 bytesB=244 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=7 ordinalB=8 gap=1 fromA=320 toA=457 fromB=324 toB=462 bytesA=137 bytesB=138 deltaBytes=1 shiftBefore=4 shiftAfter=5 deltaShift=1 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 prefix=92 candidates=1 chainSegments=1
SPR_M287_BOUNDARY resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 a=93 b=94 length=37 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=left byteA=0x87 byteB=0xA3
SPR_M287_EDGE resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 candidate=1 side=right byteA=0x19 byteB=0x18
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=0 fromA=92 toA=93 fromB=92 toB=94 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M287_GAP resource=BRDGPICA.SPR ordinalA=8 ordinalB=9 gap=1 fromA=130 toA=462 fromB=131 toB=456 bytesA=332 bytesB=325 deltaBytes=-7 shiftBefore=1 shiftAfter=-6 deltaShift=-7 consistent=1 terminal=1
SPR_M287_RESOURCE resource=BRDGPICA.SPR pairs=8 withMatches=6 candidates=12 chainSegments=12 gaps=20 consistentGaps=20
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=1 ordinalB=2 prefix=48 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=48 toA=246 fromB=48 toB=249 bytesA=198 bytesB=201 deltaBytes=3 shiftBefore=0 shiftAfter=3 deltaShift=3 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=2 ordinalB=3 prefix=84 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=84 toA=249 fromB=84 toB=255 bytesA=165 bytesB=171 deltaBytes=6 shiftBefore=0 shiftAfter=6 deltaShift=6 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=3 ordinalB=4 prefix=84 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=84 toA=255 fromB=84 toB=256 bytesA=171 bytesB=172 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=4 ordinalB=5 prefix=84 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=84 toA=256 fromB=84 toB=255 bytesA=172 bytesB=171 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=5 ordinalB=6 prefix=84 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=84 toA=255 fromB=84 toB=252 bytesA=171 bytesB=168 deltaBytes=-3 shiftBefore=0 shiftAfter=-3 deltaShift=-3 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=6 ordinalB=7 prefix=48 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=48 toA=252 fromB=48 toB=243 bytesA=204 bytesB=195 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=7 ordinalB=8 prefix=48 candidates=0 chainSegments=0
SPR_M287_GAP resource=BRDGDATA.SPR ordinalA=7 ordinalB=8 gap=0 fromA=48 toA=243 fromB=48 toB=256 bytesA=195 bytesB=208 deltaBytes=13 shiftBefore=0 shiftAfter=13 deltaShift=13 consistent=1 terminal=1
SPR_M287_PAIR resource=BRDGDATA.SPR ordinalA=8 ordinalB=9 prefix=68 candidates=0 chainSegments=0
```
