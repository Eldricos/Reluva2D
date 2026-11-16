# spr-milestone36.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-milestone36.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `04df432574247b0f181b4403d14d329289e8832e14b459b21d22f12ff48844b6`; **encoding:** `utf-16-le`; **original lines:** 37; **selected lines:** 37.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
BRDGDATA.SPR record=1 payloads=7 commonBits=7 shuffledCommonBits=1
SEGMENTS BRDGDATA.SPR record=1 count=2 thresholdBits=128
SEGMENT events=2..5 payloads=2 sharedBits=32768 tags=PMOC..PMOC endpointHeadersEqual=yes interveningControls=2 EMIT=1 KRAM=0
BREAK events=5->8 pairBits=8 tagChange=yes headerChange=no
SEGMENT events=8..16 payloads=5 sharedBits=1504 tags=MOCS..MOCS endpointHeadersEqual=yes interveningControls=4 EMIT=4 KRAM=0
event=2 tag=PMOC bytes=5880 trim=1 startBit=47032
event=5 tag=PMOC bytes=5877 trim=1 startBit=47008
event=8 tag=MOCS bytes=1477 trim=7 startBit=11802
event=10 tag=MOCS bytes=1486 trim=3 startBit=11878
event=12 tag=MOCS bytes=1473 trim=4 startBit=11773
event=14 tag=MOCS bytes=1468 trim=2 startBit=11735
event=16 tag=MOCS bytes=1489 trim=0 startBit=11905
BRDGTROI.SPR record=1 payloads=4 commonBits=15541 shuffledCommonBits=1
SEGMENTS BRDGTROI.SPR record=1 count=1 thresholdBits=128
SEGMENT events=2..10 payloads=4 sharedBits=15541 tags=PMOC..MOCS endpointHeadersEqual=yes interveningControls=5 EMIT=3 KRAM=1
event=2 tag=PMOC bytes=3763 trim=2 startBit=14561
event=6 tag=MOCS bytes=3744 trim=5 startBit=14406
event=8 tag=MOCS bytes=3709 trim=0 startBit=14131
event=10 tag=MOCS bytes=3649 trim=2 startBit=13649
BRDGTROI.SPR record=4 payloads=11 commonBits=4 shuffledCommonBits=1
SEGMENTS BRDGTROI.SPR record=4 count=2 thresholdBits=128
SEGMENT events=0..7 payloads=3 sharedBits=252 tags=MOCS..MOCS endpointHeadersEqual=yes interveningControls=4 EMIT=3 KRAM=0
BREAK events=7->26 pairBits=6 tagChange=no headerChange=yes
SEGMENT events=26..40 payloads=8 sharedBits=15551 tags=MOCS..MOCS endpointHeadersEqual=yes interveningControls=7 EMIT=6 KRAM=0
event=0 tag=MOCS bytes=90 trim=0 startBit=716
event=3 tag=MOCS bytes=94 trim=5 startBit=743
event=7 tag=MOCS bytes=94 trim=2 startBit=746
event=26 tag=MOCS bytes=3715 trim=6 startBit=29710
event=28 tag=MOCS bytes=3679 trim=2 startBit=29426
event=30 tag=MOCS bytes=3686 trim=3 startBit=29481
event=32 tag=MOCS bytes=3730 trim=2 startBit=29834
event=34 tag=MOCS bytes=3776 trim=5 startBit=30199
event=36 tag=MOCS bytes=3763 trim=6 startBit=30094
event=38 tag=MOCS bytes=3741 trim=4 startBit=29920
event=40 tag=MOCS bytes=3785 trim=0 startBit=30276
All substantial image payloads (>=64 bytes) within each selected record; 4096-byte suffix cap; zero terminal trims only.
Shuffled controls preserve per-payload byte frequencies but not order. No format or frame semantics inferred.
```
