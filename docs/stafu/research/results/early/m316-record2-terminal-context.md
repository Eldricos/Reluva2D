# m316-record2-terminal-context.txt — curated observations

**Source:** `ResearchArchive.zip` / `m316-record2-terminal-context.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `c76acf8f5a47b3a296027da6672c6de266f39e5060d074533c83dffa57989149`; **encoding:** `utf-16-le`; **original lines:** 694; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 prefix=115 candidates=0 chainSegments=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=1 ordinalB=2 gap=0 fromA=115 toA=460 fromB=115 toB=451 bytesA=345 bytesB=336 deltaBytes=-9 shiftBefore=0 shiftAfter=-9 deltaShift=-9 consistent=1 terminal=1
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 prefix=102 candidates=1 chainSegments=1
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 a=374 b=368 length=28 shift=-6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=left byteA=0xB9 byteB=0xD9
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 candidate=1 side=right byteA=0x0C byteB=0x0F
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=0 fromA=102 toA=374 fromB=102 toB=368 bytesA=272 bytesB=266 deltaBytes=-6 shiftBefore=0 shiftAfter=-6 deltaShift=-6 consistent=1 terminal=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=2 ordinalB=3 gap=1 fromA=402 toA=451 fromB=396 toB=447 bytesA=49 bytesB=51 deltaBytes=2 shiftBefore=-6 shiftAfter=-4 deltaShift=2 consistent=1 terminal=1
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 prefix=102 candidates=0 chainSegments=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=3 ordinalB=4 gap=0 fromA=102 toA=447 fromB=102 toB=451 bytesA=345 bytesB=349 deltaBytes=4 shiftBefore=0 shiftAfter=4 deltaShift=4 consistent=1 terminal=1
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 prefix=66 candidates=2 chainSegments=2
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 a=67 b=68 length=24 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=left byteA=0x30 byteB=0x04
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=1 side=right byteA=0x52 byteB=0x51
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 a=244 b=239 length=16 shift=-5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=left byteA=0x44 byteB=0x15
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 candidate=2 side=right byteA=0x66 byteB=0x6E
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=0 fromA=66 toA=67 fromB=66 toB=68 bytesA=1 bytesB=2 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=1 fromA=91 toA=244 fromB=92 toB=239 bytesA=153 bytesB=147 deltaBytes=-6 shiftBefore=1 shiftAfter=-5 deltaShift=-6 consistent=1 terminal=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=4 ordinalB=5 gap=2 fromA=260 toA=451 fromB=255 toB=455 bytesA=191 bytesB=200 deltaBytes=9 shiftBefore=-5 shiftAfter=4 deltaShift=9 consistent=1 terminal=1
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 prefix=66 candidates=2 chainSegments=2
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 a=68 b=67 length=24 shift=-1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=left byteA=0x04 byteB=0x30
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=1 side=right byteA=0x51 byteB=0x52
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 a=319 b=322 length=16 shift=3 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=left byteA=0x72 byteB=0x24
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 candidate=2 side=right byteA=0x61 byteB=0x63
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=0 fromA=66 toA=68 fromB=66 toB=67 bytesA=2 bytesB=1 deltaBytes=-1 shiftBefore=0 shiftAfter=-1 deltaShift=-1 consistent=1 terminal=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=1 fromA=92 toA=319 fromB=91 toB=322 bytesA=227 bytesB=231 deltaBytes=4 shiftBefore=-1 shiftAfter=3 deltaShift=4 consistent=1 terminal=0
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=5 ordinalB=6 gap=2 fromA=335 toA=455 fromB=338 toB=446 bytesA=120 bytesB=108 deltaBytes=-12 shiftBefore=3 shiftAfter=-9 deltaShift=-12 consistent=1 terminal=1
SPR_M293_PAIR resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 prefix=56 candidates=5 chainSegments=5
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 a=385 b=396 length=61 shift=11 leftA=byte leftB=byte leftEqual=0 rightA=end rightB=end rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=1 side=left byteA=0x78 byteB=0x60
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 a=239 b=245 length=19 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=left byteA=0x13 byteB=0x53
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=2 side=right byteA=0x60 byteB=0x62
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 a=266 b=271 length=18 shift=5 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=left byteA=0xE7 byteB=0xE5
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=3 side=right byteA=0x30 byteB=0x31
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 a=322 b=328 length=17 shift=6 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=left byteA=0x24 byteB=0x72
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=4 side=right byteA=0xB8 byteB=0xE6
SPR_M293_BOUNDARY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 a=176 b=177 length=16 shift=1 leftA=byte leftB=byte leftEqual=0 rightA=byte rightB=byte rightEqual=0 maximal=1
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=left byteA=0x43 byteB=0x12
SPR_M293_EDGE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 candidate=5 side=right byteA=0x1E byteB=0x19
SPR_M293_GAP resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 fromA=56 toA=176 fromB=56 toB=177 bytesA=120 bytesB=121 deltaBytes=1 shiftBefore=0 shiftAfter=1 deltaShift=1 consistent=1 terminal=0
SPR_M293_BASELINE resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitsA=960 bitsB=968 m282Bits=410 m284Bits=690 m285Bits=690 m286Bits=690 m284Anchors=8 retainedM284=8
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=1 kind=leading fromA=0 toA=1 fromB=0 toB=5 bitsA=1 bitsB=5 deltaBits=4 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=2 kind=internal fromA=82 toA=92 fromB=86 toB=103 bitsA=10 bitsB=17 deltaBits=7 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=3 kind=internal fromA=182 toA=198 fromB=193 toB=207 bitsA=16 bitsB=14 deltaBits=-2 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=4 kind=internal fromA=286 toA=299 fromB=295 toB=311 bitsA=13 bitsB=16 deltaBits=3 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=5 kind=internal fromA=371 toA=385 fromB=383 toB=399 bitsA=14 bitsB=16 deltaBits=2 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=6 kind=internal fromA=464 toA=514 fromB=478 toB=552 bitsA=50 bitsB=74 deltaBits=24 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=7 kind=internal fromA=596 toA=636 fromB=634 toB=674 bitsA=40 bitsB=40 deltaBits=0 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=8 kind=internal fromA=741 toA=783 fromB=779 toB=829 bitsA=42 bitsB=50 deltaBits=8 unconstrainedContained=0
SPR_M293_INTERVAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 interval=9 kind=trailing fromA=876 toA=960 fromB=922 toB=968 bitsA=84 bitsB=46 deltaBits=-38 unconstrainedContained=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=0 coverageScale=4 objective=2760 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=1 coverageScale=4 objective=3803 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=2 coverageScale=4 objective=4846 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=4 coverageScale=4 objective=6932 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=8 coverageScale=4 objective=11104 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_SENSITIVITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 evidenceWeight=16 coverageScale=4 objective=19448 bits=690 runs=8 retainedPreserved=8 droppedPreserved=0 optimalTie=0
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=1 bitB=5 length=81 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=92 bitB=103 length=90 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=198 bitB=207 length=88 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=299 bitB=311 length=72 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=385 bitB=399 length=79 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=514 bitB=552 length=82 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=636 bitB=674 length=105 selectedAcross=6 configurations=6 stable=1
SPR_M293_STABILITY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 bitA=783 bitB=829 length=93 selectedAcross=6 configurations=6 stable=1
SPR_M293_GLOBAL resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 pool=9 m290Bits=690 coverageOnlyBits=690 highEvidenceBits=690 stableBaselineSegments=8 baselineSegments=8
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=1 bitA=1 bitB=5 length=81 shift=4 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=2 bitA=92 bitB=103 length=90 shift=11 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=3 bitA=198 bitB=207 length=88 shift=9 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=4 bitA=299 bitB=311 length=72 shift=12 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=5 bitA=385 bitB=399 length=79 shift=14 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=6 bitA=514 bitB=552 length=82 shift=38 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=7 bitA=636 bitB=674 length=105 shift=38 preserved=1
SPR_M293_SELECTED resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 segment=8 bitA=783 bitB=829 length=93 shift=46 preserved=1
SPR_M293_EXIT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=1 baselineSegment=1 bitA=1 bitB=5 length=81 shift=4 ambiguous=0
SPR_M293_DEPENDENCY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=1 exited=1 entered=0 changedSpanA=81 changedSpanB=81 minBitA=1 maxBitA=82 minBitB=5 maxBitB=86 enteredUniqueBits=0 enteredAmbiguousBits=0 enteredAmbiguousRuns=0 localizedA=1 localizedB=1
SPR_M293_PERTURB resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=1 pool=8 bits=609 deltaBits=-81 objective=3354 deltaObjective=-449 runs=7 retainedBaseline=7 replacedBaseline=1
SPR_M293_EXIT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=2 baselineSegment=2 bitA=92 bitB=103 length=90 shift=11 ambiguous=0
SPR_M293_DEPENDENCY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=2 exited=1 entered=0 changedSpanA=90 changedSpanB=90 minBitA=92 maxBitA=182 minBitB=103 maxBitB=193 enteredUniqueBits=0 enteredAmbiguousBits=0 enteredAmbiguousRuns=0 localizedA=1 localizedB=1
SPR_M293_PERTURB resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=2 pool=8 bits=600 deltaBits=-90 objective=3309 deltaObjective=-494 runs=7 retainedBaseline=7 replacedBaseline=1
SPR_M293_EXIT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=3 baselineSegment=3 bitA=198 bitB=207 length=88 shift=9 ambiguous=0
SPR_M293_DEPENDENCY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=3 exited=1 entered=0 changedSpanA=88 changedSpanB=88 minBitA=198 maxBitA=286 minBitB=207 maxBitB=295 enteredUniqueBits=0 enteredAmbiguousBits=0 enteredAmbiguousRuns=0 localizedA=1 localizedB=1
SPR_M293_PERTURB resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=3 pool=8 bits=602 deltaBits=-88 objective=3320 deltaObjective=-483 runs=7 retainedBaseline=7 replacedBaseline=1
SPR_M293_EXIT resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=4 baselineSegment=4 bitA=299 bitB=311 length=72 shift=12 ambiguous=0
SPR_M293_DEPENDENCY resource=BRDGPICA.SPR ordinalA=6 ordinalB=7 gap=0 mode=leave_one_out parameter=4 exited=1 entered=0 changedSpanA=72 changedSpanB=72 minBitA=299 maxBitA=371 minBitB=311 maxBitB=383 enteredUniqueBits=0 enteredAmbiguousBits=0 enteredAmbiguousRuns=0 localizedA=1 localizedB=1
```
