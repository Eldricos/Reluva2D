# spr-milestone34.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-milestone34.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `86fd2d1d286e6a05330bd26c6fef252be49d3ed8c290e84e2443a66afe9d8dae`; **encoding:** `utf-16-le`; **original lines:** 19; **selected lines:** 19.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
Three consecutive substantial image payloads within selected records.
BRDGDATA.SPR record=1 events=2,5,8 tags=PMOC,PMOC,MOCS bytes=5880,5877,1477 sharedBits=8 trims=0,0,6 starts=47032,47008,11802 startDeltas=-24,-35206 headerEqual=yes
BRDGDATA.SPR record=1 events=5,8,10 tags=PMOC,MOCS,MOCS bytes=5877,1477,1486 sharedBits=8 trims=0,6,2 starts=47008,11802,11878 startDeltas=-35206,76 headerEqual=yes
BRDGDATA.SPR record=1 events=8,10,12 tags=MOCS,MOCS,MOCS bytes=1477,1486,1473 sharedBits=2387 trims=4,0,1 starts=9425,9501,9396 startDeltas=76,-105 headerEqual=yes
BRDGDATA.SPR record=1 events=10,12,14 tags=MOCS,MOCS,MOCS bytes=1486,1473,1468 sharedBits=1506 trims=1,2,0 starts=10381,10276,10238 startDeltas=-105,-38 headerEqual=yes
BRDGDATA.SPR record=1 events=12,14,16 tags=MOCS,MOCS,MOCS bytes=1473,1468,1489 sharedBits=1504 trims=4,2,0 starts=10276,10238,10408 startDeltas=-38,170 headerEqual=yes
RECORD BRDGDATA.SPR 1 triples=5
BRDGTROI.SPR record=1 events=2,6,8 tags=PMOC,MOCS,MOCS bytes=3763,3744,3709 sharedBits=15541 trims=2,5,0 starts=14561,14406,14131 startDeltas=-155,-275 headerEqual=yes
BRDGTROI.SPR record=1 events=6,8,10 tags=MOCS,MOCS,MOCS bytes=3744,3709,3649 sharedBits=15541 trims=5,0,2 starts=14406,14131,13649 startDeltas=-275,-482 headerEqual=yes
RECORD BRDGTROI.SPR 1 triples=2
BRDGTROI.SPR record=4 events=26,28,30 tags=MOCS,MOCS,MOCS bytes=3715,3679,3686 sharedBits=15563 trims=4,0,1 starts=14153,13869,13924 startDeltas=-284,55 headerEqual=yes
BRDGTROI.SPR record=4 events=28,30,32 tags=MOCS,MOCS,MOCS bytes=3679,3686,3730 sharedBits=15563 trims=0,1,0 starts=13869,13924,14277 startDeltas=55,353 headerEqual=yes
BRDGTROI.SPR record=4 events=30,32,34 tags=MOCS,MOCS,MOCS bytes=3686,3730,3776 sharedBits=15563 trims=1,0,3 starts=13924,14277,14642 startDeltas=353,365 headerEqual=yes
BRDGTROI.SPR record=4 events=32,34,36 tags=MOCS,MOCS,MOCS bytes=3730,3776,3763 sharedBits=15564 trims=0,3,4 starts=14276,14641,14536 startDeltas=365,-105 headerEqual=yes
BRDGTROI.SPR record=4 events=34,36,38 tags=MOCS,MOCS,MOCS bytes=3776,3763,3741 sharedBits=15566 trims=1,2,0 starts=14641,14536,14362 startDeltas=-105,-174 headerEqual=yes
BRDGTROI.SPR record=4 events=36,38,40 tags=MOCS,MOCS,MOCS bytes=3763,3741,3785 sharedBits=15551 trims=6,4,0 starts=14547,14373,14729 startDeltas=-174,356 headerEqual=yes
RECORD BRDGTROI.SPR 4 triples=6
TOTAL triples=13 suffix128=11 unavailable=0
Offsets are from each full opaque payload, MSB-first. Trims require zero terminal bits; no decoder assumptions.
```
