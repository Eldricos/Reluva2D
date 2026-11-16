# spr-milestone37.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-milestone37.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `b149c187881cc3a8021958c395f8763d1416217856e0b740ab34463663417d0a`; **encoding:** `utf-16-le`; **original lines:** 138; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
BRDGDATA.SPR record=1 payloads=7 segments=2
SEGMENT events=2..5 count=2 commonBits=32768 tags=PMOC..PMOC
SEGMENT events=8..16 count=5 commonBits=1504 tags=MOCS..MOCS
BREAK events=5->8 tagChange=yes headerChange=no
BRDGDATA.SPR record=3 payloads=9 segments=1
SEGMENT events=3..20 count=9 commonBits=300 tags=MOCS..MOCS
BRDGDATA.SPR record=4 payloads=3 segments=1
SEGMENT events=1..6 count=3 commonBits=565 tags=MOCS..MOCS
SUMMARY BRDGDATA.SPR records=4 eligibleRecords=3 segments=4 multiPayloadSegments=4 breaks=1 breakTagChange=1 breakHeaderChange=0 breakBoth=0 breakNeither=0 withinTagChange=0 withinHeaderChange=0 withinBoth=0 withinNeither=15
BRDGPICA.SPR record=1 payloads=6 segments=3
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=6..15 count=4 commonBits=336 tags=MOCS..MOCS
SEGMENT events=45..45 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=3->6 tagChange=yes headerChange=yes
BREAK events=15->45 tagChange=yes headerChange=yes
BRDGPICA.SPR record=3 payloads=9 segments=1
SEGMENT events=3..20 count=9 commonBits=349 tags=MOCS..MOCS
BRDGPICA.SPR record=4 payloads=7 segments=7
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=5..5 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=7..7 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=9..9 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=11..11 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=13..13 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=15..15 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=3->5 tagChange=no headerChange=no
BREAK events=5->7 tagChange=no headerChange=no
BREAK events=7->9 tagChange=no headerChange=no
BREAK events=9->11 tagChange=no headerChange=no
BREAK events=11->13 tagChange=no headerChange=no
BREAK events=13->15 tagChange=no headerChange=no
BRDGPICA.SPR record=6 payloads=8 segments=6
SEGMENT events=0..5 count=3 commonBits=164 tags=PMOC..PMOC
SEGMENT events=7..7 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=9..9 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=11..11 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=13..13 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=15..15 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=5->7 tagChange=no headerChange=no
BREAK events=7->9 tagChange=no headerChange=no
BREAK events=9->11 tagChange=no headerChange=no
BREAK events=11->13 tagChange=no headerChange=no
BREAK events=13->15 tagChange=no headerChange=no
SUMMARY BRDGPICA.SPR records=6 eligibleRecords=4 segments=17 multiPayloadSegments=3 breaks=13 breakTagChange=2 breakHeaderChange=2 breakBoth=2 breakNeither=11 withinTagChange=0 withinHeaderChange=0 withinBoth=0 withinNeither=13
BRDGRIKR.SPR record=1 payloads=5 segments=2
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=7..14 count=4 commonBits=233 tags=MOCS..MOCS
BREAK events=3->7 tagChange=yes headerChange=yes
BRDGRIKR.SPR record=2 payloads=1 segments=1
SEGMENT events=0..0 count=1 commonBits=0 tags=PMOC..PMOC
BRDGRIKR.SPR record=3 payloads=9 segments=1
SEGMENT events=3..20 count=9 commonBits=247 tags=MOCS..MOCS
BRDGRIKR.SPR record=4 payloads=6 segments=6
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=5..5 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=7..7 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=9..9 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=11..11 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=13..13 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=3->5 tagChange=no headerChange=no
BREAK events=5->7 tagChange=no headerChange=no
BREAK events=7->9 tagChange=no headerChange=no
BREAK events=9->11 tagChange=no headerChange=no
BREAK events=11->13 tagChange=no headerChange=no
BRDGRIKR.SPR record=5 payloads=6 segments=6
SEGMENT events=0..0 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=5..5 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=7..7 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=9..9 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=11..11 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=0->3 tagChange=no headerChange=no
BREAK events=3->5 tagChange=no headerChange=no
BREAK events=5->7 tagChange=no headerChange=no
BREAK events=7->9 tagChange=no headerChange=no
BREAK events=9->11 tagChange=no headerChange=no
BRDGRIKR.SPR record=6 payloads=8 segments=8
SEGMENT events=0..0 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=3..3 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=5..5 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=7..7 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=9..9 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=11..11 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=13..13 count=1 commonBits=0 tags=PMOC..PMOC
SEGMENT events=15..15 count=1 commonBits=0 tags=PMOC..PMOC
BREAK events=0->3 tagChange=no headerChange=no
BREAK events=3->5 tagChange=no headerChange=no
BREAK events=5->7 tagChange=no headerChange=no
BREAK events=7->9 tagChange=no headerChange=no
BREAK events=9->11 tagChange=no headerChange=no
```
