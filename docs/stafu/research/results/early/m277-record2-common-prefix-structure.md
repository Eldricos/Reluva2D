# m277-record2-common-prefix-structure.txt — curated observations

**Source:** `ResearchArchive.zip` / `m277-record2-common-prefix-structure.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `4749b54c4df419b4328b20c454731282f4eea97deaebecb9cb3867d00f76429f`; **encoding:** `utf-16-le`; **original lines:** 181; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M277_RECORD resource=BRDGPICA.SPR record=2 images=10 references=1 direct=9
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=1 marker=0x0001000D payloadBytes=460 offset=19041
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=2 marker=0x0001000D payloadBytes=451 offset=19549
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=3 marker=0x0001000D payloadBytes=447 offset=20032
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=4 marker=0x0001000D payloadBytes=451 offset=20511
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=5 marker=0x0001000D payloadBytes=455 offset=20994
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=6 marker=0x0001000D payloadBytes=446 offset=21481
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=7 marker=0x0001000D payloadBytes=457 offset=21959
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=8 marker=0x0001000D payloadBytes=462 offset=22448
SPR_M277_IMAGE resource=BRDGPICA.SPR record=2 ordinal=9 marker=0x0001000D payloadBytes=456 offset=22942
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=1 ordinalB=2 markerA=0x0001000D markerB=0x0001000D bytesA=460 bytesB=451 prefix=115 firstDifference=byte differingOffset=115 byteA=0x4D byteB=0x4A
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=2 ordinalB=3 markerA=0x0001000D markerB=0x0001000D bytesA=451 bytesB=447 prefix=102 firstDifference=byte differingOffset=102 byteA=0xE6 byteB=0xF9
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=3 ordinalB=4 markerA=0x0001000D markerB=0x0001000D bytesA=447 bytesB=451 prefix=102 firstDifference=byte differingOffset=102 byteA=0xF9 byteB=0xE6
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=4 ordinalB=5 markerA=0x0001000D markerB=0x0001000D bytesA=451 bytesB=455 prefix=66 firstDifference=byte differingOffset=66 byteA=0x30 byteB=0x10
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=5 ordinalB=6 markerA=0x0001000D markerB=0x0001000D bytesA=455 bytesB=446 prefix=66 firstDifference=byte differingOffset=66 byteA=0x10 byteB=0x30
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=6 ordinalB=7 markerA=0x0001000D markerB=0x0001000D bytesA=446 bytesB=457 prefix=56 firstDifference=byte differingOffset=56 byteA=0xBC byteB=0x43
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=7 ordinalB=8 markerA=0x0001000D markerB=0x0001000D bytesA=457 bytesB=462 prefix=56 firstDifference=byte differingOffset=56 byteA=0x43 byteB=0xBC
SPR_M277_PAIR resource=BRDGPICA.SPR record=2 ordinalA=8 ordinalB=9 markerA=0x0001000D markerB=0x0001000D bytesA=462 bytesB=456 prefix=92 firstDifference=byte differingOffset=92 byteA=0x87 byteB=0x00
SPR_M277_RESOURCE resource=BRDGPICA.SPR adjacentDirectPairs=8 prefix32plus=8
SPR_M277_RECORD resource=BRDGDATA.SPR record=2 images=10 references=1 direct=9
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=1 marker=0x0002000D payloadBytes=246 offset=19890
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=2 marker=0x0002000D payloadBytes=249 offset=20184
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=3 marker=0x0002000D payloadBytes=255 offset=20465
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=4 marker=0x0002000D payloadBytes=256 offset=20752
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=5 marker=0x0002000D payloadBytes=255 offset=21040
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=6 marker=0x0002000D payloadBytes=252 offset=21327
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=7 marker=0x0002000D payloadBytes=243 offset=21611
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=8 marker=0x0002000D payloadBytes=256 offset=21886
SPR_M277_IMAGE resource=BRDGDATA.SPR record=2 ordinal=9 marker=0x0002000D payloadBytes=258 offset=22174
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=1 ordinalB=2 markerA=0x0002000D markerB=0x0002000D bytesA=246 bytesB=249 prefix=48 firstDifference=byte differingOffset=48 byteA=0x75 byteB=0x74
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=2 ordinalB=3 markerA=0x0002000D markerB=0x0002000D bytesA=249 bytesB=255 prefix=84 firstDifference=byte differingOffset=84 byteA=0xA7 byteB=0x67
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=3 ordinalB=4 markerA=0x0002000D markerB=0x0002000D bytesA=255 bytesB=256 prefix=84 firstDifference=byte differingOffset=84 byteA=0x67 byteB=0xA7
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=4 ordinalB=5 markerA=0x0002000D markerB=0x0002000D bytesA=256 bytesB=255 prefix=84 firstDifference=byte differingOffset=84 byteA=0xA7 byteB=0x67
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=5 ordinalB=6 markerA=0x0002000D markerB=0x0002000D bytesA=255 bytesB=252 prefix=84 firstDifference=byte differingOffset=84 byteA=0x67 byteB=0xA7
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=6 ordinalB=7 markerA=0x0002000D markerB=0x0002000D bytesA=252 bytesB=243 prefix=48 firstDifference=byte differingOffset=48 byteA=0x74 byteB=0x75
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=7 ordinalB=8 markerA=0x0002000D markerB=0x0002000D bytesA=243 bytesB=256 prefix=48 firstDifference=byte differingOffset=48 byteA=0x75 byteB=0x74
SPR_M277_PAIR resource=BRDGDATA.SPR record=2 ordinalA=8 ordinalB=9 markerA=0x0002000D markerB=0x0002000D bytesA=256 bytesB=258 prefix=68 firstDifference=byte differingOffset=68 byteA=0x59 byteB=0x69
SPR_M277_RESOURCE resource=BRDGDATA.SPR adjacentDirectPairs=8 prefix32plus=8
SPR_M277_RECORD resource=BRDGRIKR.SPR record=2 images=10 references=1 direct=9
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=1 marker=0x0001000D payloadBytes=357 offset=13292
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=2 marker=0x0001000D payloadBytes=360 offset=13697
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=3 marker=0x0001000D payloadBytes=353 offset=14089
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=4 marker=0x0001000D payloadBytes=352 offset=14474
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=5 marker=0x0001000D payloadBytes=351 offset=14858
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=6 marker=0x0001000D payloadBytes=359 offset=15241
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=7 marker=0x0001000D payloadBytes=353 offset=15632
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=8 marker=0x0001000D payloadBytes=357 offset=16017
SPR_M277_IMAGE resource=BRDGRIKR.SPR record=2 ordinal=9 marker=0x0001000D payloadBytes=353 offset=16406
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=1 ordinalB=2 markerA=0x0001000D markerB=0x0001000D bytesA=357 bytesB=360 prefix=147 firstDifference=byte differingOffset=147 byteA=0xC9 byteB=0xC8
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=2 ordinalB=3 markerA=0x0001000D markerB=0x0001000D bytesA=360 bytesB=353 prefix=137 firstDifference=byte differingOffset=137 byteA=0x97 byteB=0x87
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=3 ordinalB=4 markerA=0x0001000D markerB=0x0001000D bytesA=353 bytesB=352 prefix=137 firstDifference=byte differingOffset=137 byteA=0x87 byteB=0x97
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=4 ordinalB=5 markerA=0x0001000D markerB=0x0001000D bytesA=352 bytesB=351 prefix=154 firstDifference=byte differingOffset=154 byteA=0xF9 byteB=0xE7
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=5 ordinalB=6 markerA=0x0001000D markerB=0x0001000D bytesA=351 bytesB=359 prefix=141 firstDifference=byte differingOffset=141 byteA=0xC9 byteB=0xCB
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=6 ordinalB=7 markerA=0x0001000D markerB=0x0001000D bytesA=359 bytesB=353 prefix=137 firstDifference=byte differingOffset=137 byteA=0x97 byteB=0x87
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=7 ordinalB=8 markerA=0x0001000D markerB=0x0001000D bytesA=353 bytesB=357 prefix=137 firstDifference=byte differingOffset=137 byteA=0x87 byteB=0x97
SPR_M277_PAIR resource=BRDGRIKR.SPR record=2 ordinalA=8 ordinalB=9 markerA=0x0001000D markerB=0x0001000D bytesA=357 bytesB=353 prefix=137 firstDifference=byte differingOffset=137 byteA=0x97 byteB=0x87
SPR_M277_RESOURCE resource=BRDGRIKR.SPR adjacentDirectPairs=8 prefix32plus=8
SPR_M277_RECORD resource=BRDGTROI.SPR record=2 images=10 references=1 direct=9
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=1 marker=0x0001000D payloadBytes=201 offset=15341
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=2 marker=0x0001000D payloadBytes=198 offset=15590
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=3 marker=0x0001000D payloadBytes=200 offset=15820
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=4 marker=0x0001000D payloadBytes=198 offset=16052
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=5 marker=0x0001000D payloadBytes=198 offset=16282
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=6 marker=0x0002000D payloadBytes=201 offset=16512
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=7 marker=0x0001000D payloadBytes=197 offset=16745
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=8 marker=0x0001000D payloadBytes=197 offset=16974
SPR_M277_IMAGE resource=BRDGTROI.SPR record=2 ordinal=9 marker=0x0001000D payloadBytes=192 offset=17203
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=1 ordinalB=2 markerA=0x0001000D markerB=0x0001000D bytesA=201 bytesB=198 prefix=77 firstDifference=byte differingOffset=77 byteA=0x1F byteB=0x19
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=2 ordinalB=3 markerA=0x0001000D markerB=0x0001000D bytesA=198 bytesB=200 prefix=77 firstDifference=byte differingOffset=77 byteA=0x19 byteB=0x01
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=3 ordinalB=4 markerA=0x0001000D markerB=0x0001000D bytesA=200 bytesB=198 prefix=61 firstDifference=byte differingOffset=61 byteA=0x02 byteB=0x03
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=4 ordinalB=5 markerA=0x0001000D markerB=0x0001000D bytesA=198 bytesB=198 prefix=62 firstDifference=byte differingOffset=62 byteA=0x46 byteB=0x45
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=5 ordinalB=6 markerA=0x0001000D markerB=0x0002000D bytesA=198 bytesB=201 prefix=0 firstDifference=byte differingOffset=0 byteA=0xCE byteB=0x6E
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=6 ordinalB=7 markerA=0x0002000D markerB=0x0001000D bytesA=201 bytesB=197 prefix=0 firstDifference=byte differingOffset=0 byteA=0x6E byteB=0xCE
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=7 ordinalB=8 markerA=0x0001000D markerB=0x0001000D bytesA=197 bytesB=197 prefix=76 firstDifference=byte differingOffset=76 byteA=0xC0 byteB=0xC1
SPR_M277_PAIR resource=BRDGTROI.SPR record=2 ordinalA=8 ordinalB=9 markerA=0x0001000D markerB=0x0001000D bytesA=197 bytesB=192 prefix=79 firstDifference=byte differingOffset=79 byteA=0xFC byteB=0xE6
SPR_M277_RESOURCE resource=BRDGTROI.SPR adjacentDirectPairs=8 prefix32plus=6
SPR_M277_RECORD resource=BRDGWORF.SPR record=2 images=9 references=1 direct=8
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=1 marker=0x0002000D payloadBytes=182 offset=67515
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=2 marker=0x0002000D payloadBytes=187 offset=67745
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=3 marker=0x0002000D payloadBytes=192 offset=67964
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=4 marker=0x0002000D payloadBytes=197 offset=68188
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=5 marker=0x0002000D payloadBytes=201 offset=68417
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=6 marker=0x0002000D payloadBytes=180 offset=68650
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=7 marker=0x0002000D payloadBytes=179 offset=68862
SPR_M277_IMAGE resource=BRDGWORF.SPR record=2 ordinal=8 marker=0x0002000D payloadBytes=183 offset=69073
SPR_M277_PAIR resource=BRDGWORF.SPR record=2 ordinalA=1 ordinalB=2 markerA=0x0002000D markerB=0x0002000D bytesA=182 bytesB=187 prefix=115 firstDifference=byte differingOffset=115 byteA=0x34 byteB=0x2E
SPR_M277_PAIR resource=BRDGWORF.SPR record=2 ordinalA=2 ordinalB=3 markerA=0x0002000D markerB=0x0002000D bytesA=187 bytesB=192 prefix=90 firstDifference=byte differingOffset=90 byteA=0xA3 byteB=0x93
SPR_M277_PAIR resource=BRDGWORF.SPR record=2 ordinalA=3 ordinalB=4 markerA=0x0002000D markerB=0x0002000D bytesA=192 bytesB=197 prefix=90 firstDifference=byte differingOffset=90 byteA=0x93 byteB=0xA3
SPR_M277_PAIR resource=BRDGWORF.SPR record=2 ordinalA=4 ordinalB=5 markerA=0x0002000D markerB=0x0002000D bytesA=197 bytesB=201 prefix=100 firstDifference=byte differingOffset=100 byteA=0x1A byteB=0x16
SPR_M277_PAIR resource=BRDGWORF.SPR record=2 ordinalA=5 ordinalB=6 markerA=0x0002000D markerB=0x0002000D bytesA=201 bytesB=180 prefix=100 firstDifference=byte differingOffset=100 byteA=0x16 byteB=0x1A
```
