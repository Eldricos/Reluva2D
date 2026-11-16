# m325-record-structure.txt — curated observations

**Source:** `ResearchArchive.zip` / `m325-record-structure.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `131e08912ac942e7a9b6560194f17e63f2270747d0a19821e67ef91969073612`; **encoding:** `utf-16-le`; **original lines:** 242; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M325_RESOURCE resource=BRDGPICA.SPR fileBytes=166784 records=6
SPR_M325_RECORD resource=BRDGPICA.SPR record=0 start=48 end=18909 bytes=18861 chunks=84
SPR_M325_RECORD resource=BRDGPICA.SPR record=1 start=18909 end=18973 bytes=64 chunks=4
SPR_M325_RECORD resource=BRDGPICA.SPR record=2 start=18973 end=23422 bytes=4449 chunks=23
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=0 tag=PMOC chunkOffset=18973 chunkBytes=24 payloadOffset=18981 payloadBytes=16 imageHeader=1 headerA=124 headerB=282 marker=0x00030000 imageDataOffset=18993 imageDataBytes=4
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=0 event=0 kind=reference headerA=124 headerB=282 marker=0x00030000 dataOffset=18993 dataBytes=4 chunkEnd=18997
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=1 tag=NSOP chunkOffset=18997 chunkBytes=16 payloadOffset=19005 payloadBytes=8 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=2 tag=KRAM chunkOffset=19013 chunkBytes=8 payloadOffset=19021 payloadBytes=0 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=3 tag=MOCS chunkOffset=19021 chunkBytes=480 payloadOffset=19029 payloadBytes=472 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=19041 imageDataBytes=460
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=1 event=3 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=19041 dataBytes=460 chunkEnd=19501
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=4 tag=NSOP chunkOffset=19501 chunkBytes=16 payloadOffset=19509 payloadBytes=8 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=5 tag=EMIT chunkOffset=19517 chunkBytes=12 payloadOffset=19525 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=6 tag=MOCS chunkOffset=19529 chunkBytes=471 payloadOffset=19537 payloadBytes=463 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=19549 imageDataBytes=451
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=2 event=6 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=19549 dataBytes=451 chunkEnd=20000
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=7 tag=EMIT chunkOffset=20000 chunkBytes=12 payloadOffset=20008 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=8 tag=MOCS chunkOffset=20012 chunkBytes=467 payloadOffset=20020 payloadBytes=459 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=20032 imageDataBytes=447
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=3 event=8 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=20032 dataBytes=447 chunkEnd=20479
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=9 tag=EMIT chunkOffset=20479 chunkBytes=12 payloadOffset=20487 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=10 tag=MOCS chunkOffset=20491 chunkBytes=471 payloadOffset=20499 payloadBytes=463 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=20511 imageDataBytes=451
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=4 event=10 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=20511 dataBytes=451 chunkEnd=20962
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=11 tag=EMIT chunkOffset=20962 chunkBytes=12 payloadOffset=20970 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=12 tag=MOCS chunkOffset=20974 chunkBytes=475 payloadOffset=20982 payloadBytes=467 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=20994 imageDataBytes=455
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=5 event=12 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=20994 dataBytes=455 chunkEnd=21449
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=13 tag=EMIT chunkOffset=21449 chunkBytes=12 payloadOffset=21457 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=14 tag=MOCS chunkOffset=21461 chunkBytes=466 payloadOffset=21469 payloadBytes=458 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=21481 imageDataBytes=446
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=6 event=14 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=21481 dataBytes=446 chunkEnd=21927
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=15 tag=EMIT chunkOffset=21927 chunkBytes=12 payloadOffset=21935 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=16 tag=MOCS chunkOffset=21939 chunkBytes=477 payloadOffset=21947 payloadBytes=469 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=21959 imageDataBytes=457
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=7 event=16 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=21959 dataBytes=457 chunkEnd=22416
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=17 tag=EMIT chunkOffset=22416 chunkBytes=12 payloadOffset=22424 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=18 tag=MOCS chunkOffset=22428 chunkBytes=482 payloadOffset=22436 payloadBytes=474 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=22448 imageDataBytes=462
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=8 event=18 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=22448 dataBytes=462 chunkEnd=22910
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=19 tag=EMIT chunkOffset=22910 chunkBytes=12 payloadOffset=22918 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=20 tag=MOCS chunkOffset=22922 chunkBytes=476 payloadOffset=22930 payloadBytes=468 imageHeader=1 headerA=32 headerB=28 marker=0x0001000D imageDataOffset=22942 imageDataBytes=456
SPR_M325_IMAGE resource=BRDGPICA.SPR ordinal=9 event=20 kind=direct headerA=32 headerB=28 marker=0x0001000D dataOffset=22942 dataBytes=456 chunkEnd=23398
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=21 tag=EMIT chunkOffset=23398 chunkBytes=12 payloadOffset=23406 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGPICA.SPR record=2 event=22 tag=PMUJ chunkOffset=23410 chunkBytes=12 payloadOffset=23418 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_RECORD2_SUMMARY resource=BRDGPICA.SPR images=10 direct=9 references=1
SPR_M325_RECORD resource=BRDGPICA.SPR record=3 start=23422 end=73737 bytes=50315 chunks=177
SPR_M325_RECORD resource=BRDGPICA.SPR record=4 start=73737 end=74089 bytes=352 chunks=20
SPR_M325_RECORD resource=BRDGPICA.SPR record=5 start=74089 end=166784 bytes=92695 chunks=113
SPR_M325_RESOURCE resource=BRDGDATA.SPR fileBytes=24363 records=4
SPR_M325_RECORD resource=BRDGDATA.SPR record=0 start=40 end=19742 bytes=19702 chunks=34
SPR_M325_RECORD resource=BRDGDATA.SPR record=1 start=19742 end=19822 bytes=80 chunks=6
SPR_M325_RECORD resource=BRDGDATA.SPR record=2 start=19822 end=22456 bytes=2634 chunks=23
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=0 tag=PMOC chunkOffset=19822 chunkBytes=24 payloadOffset=19830 payloadBytes=16 imageHeader=1 headerA=98 headerB=176 marker=0x00030000 imageDataOffset=19842 imageDataBytes=4
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=0 event=0 kind=reference headerA=98 headerB=176 marker=0x00030000 dataOffset=19842 dataBytes=4 chunkEnd=19846
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=1 tag=NSOP chunkOffset=19846 chunkBytes=16 payloadOffset=19854 payloadBytes=8 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=2 tag=KRAM chunkOffset=19862 chunkBytes=8 payloadOffset=19870 payloadBytes=0 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=3 tag=MOCS chunkOffset=19870 chunkBytes=266 payloadOffset=19878 payloadBytes=258 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=19890 imageDataBytes=246
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=1 event=3 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=19890 dataBytes=246 chunkEnd=20136
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=4 tag=NSOP chunkOffset=20136 chunkBytes=16 payloadOffset=20144 payloadBytes=8 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=5 tag=EMIT chunkOffset=20152 chunkBytes=12 payloadOffset=20160 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=6 tag=MOCS chunkOffset=20164 chunkBytes=269 payloadOffset=20172 payloadBytes=261 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=20184 imageDataBytes=249
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=2 event=6 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=20184 dataBytes=249 chunkEnd=20433
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=7 tag=EMIT chunkOffset=20433 chunkBytes=12 payloadOffset=20441 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=8 tag=MOCS chunkOffset=20445 chunkBytes=275 payloadOffset=20453 payloadBytes=267 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=20465 imageDataBytes=255
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=3 event=8 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=20465 dataBytes=255 chunkEnd=20720
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=9 tag=EMIT chunkOffset=20720 chunkBytes=12 payloadOffset=20728 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=10 tag=MOCS chunkOffset=20732 chunkBytes=276 payloadOffset=20740 payloadBytes=268 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=20752 imageDataBytes=256
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=4 event=10 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=20752 dataBytes=256 chunkEnd=21008
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=11 tag=EMIT chunkOffset=21008 chunkBytes=12 payloadOffset=21016 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=12 tag=MOCS chunkOffset=21020 chunkBytes=275 payloadOffset=21028 payloadBytes=267 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=21040 imageDataBytes=255
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=5 event=12 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=21040 dataBytes=255 chunkEnd=21295
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=13 tag=EMIT chunkOffset=21295 chunkBytes=12 payloadOffset=21303 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=14 tag=MOCS chunkOffset=21307 chunkBytes=272 payloadOffset=21315 payloadBytes=264 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=21327 imageDataBytes=252
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=6 event=14 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=21327 dataBytes=252 chunkEnd=21579
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=15 tag=EMIT chunkOffset=21579 chunkBytes=12 payloadOffset=21587 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=16 tag=MOCS chunkOffset=21591 chunkBytes=263 payloadOffset=21599 payloadBytes=255 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=21611 imageDataBytes=243
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=7 event=16 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=21611 dataBytes=243 chunkEnd=21854
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=17 tag=EMIT chunkOffset=21854 chunkBytes=12 payloadOffset=21862 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=18 tag=MOCS chunkOffset=21866 chunkBytes=276 payloadOffset=21874 payloadBytes=268 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=21886 imageDataBytes=256
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=8 event=18 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=21886 dataBytes=256 chunkEnd=22142
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=19 tag=EMIT chunkOffset=22142 chunkBytes=12 payloadOffset=22150 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=20 tag=MOCS chunkOffset=22154 chunkBytes=278 payloadOffset=22162 payloadBytes=270 imageHeader=1 headerA=18 headerB=16 marker=0x0002000D imageDataOffset=22174 imageDataBytes=258
SPR_M325_IMAGE resource=BRDGDATA.SPR ordinal=9 event=20 kind=direct headerA=18 headerB=16 marker=0x0002000D dataOffset=22174 dataBytes=258 chunkEnd=22432
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=21 tag=EMIT chunkOffset=22432 chunkBytes=12 payloadOffset=22440 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGDATA.SPR record=2 event=22 tag=PMUJ chunkOffset=22444 chunkBytes=12 payloadOffset=22452 payloadBytes=4 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_RECORD2_SUMMARY resource=BRDGDATA.SPR images=10 direct=9 references=1
SPR_M325_RECORD resource=BRDGDATA.SPR record=3 start=22456 end=24363 bytes=1907 chunks=76
SPR_M325_RESOURCE resource=BRDGRIKR.SPR fileBytes=141680 records=6
SPR_M325_RECORD resource=BRDGRIKR.SPR record=0 start=48 end=7472 bytes=7424 chunks=35
SPR_M325_RECORD resource=BRDGRIKR.SPR record=1 start=7472 end=13224 bytes=5752 chunks=4
SPR_M325_RECORD resource=BRDGRIKR.SPR record=2 start=13224 end=16783 bytes=3559 chunks=23
SPR_M325_CHUNK resource=BRDGRIKR.SPR record=2 event=0 tag=PMOC chunkOffset=13224 chunkBytes=24 payloadOffset=13232 payloadBytes=16 imageHeader=1 headerA=120 headerB=258 marker=0x00030000 imageDataOffset=13244 imageDataBytes=4
SPR_M325_IMAGE resource=BRDGRIKR.SPR ordinal=0 event=0 kind=reference headerA=120 headerB=258 marker=0x00030000 dataOffset=13244 dataBytes=4 chunkEnd=13248
SPR_M325_CHUNK resource=BRDGRIKR.SPR record=2 event=1 tag=NSOP chunkOffset=13248 chunkBytes=16 payloadOffset=13256 payloadBytes=8 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGRIKR.SPR record=2 event=2 tag=KRAM chunkOffset=13264 chunkBytes=8 payloadOffset=13272 payloadBytes=0 imageHeader=0 headerA=0 headerB=0 marker=0x00000000 imageDataOffset=0 imageDataBytes=0
SPR_M325_CHUNK resource=BRDGRIKR.SPR record=2 event=3 tag=MOCS chunkOffset=13272 chunkBytes=377 payloadOffset=13280 payloadBytes=369 imageHeader=1 headerA=20 headerB=24 marker=0x0001000D imageDataOffset=13292 imageDataBytes=357
SPR_M325_IMAGE resource=BRDGRIKR.SPR ordinal=1 event=3 kind=direct headerA=20 headerB=24 marker=0x0001000D dataOffset=13292 dataBytes=357 chunkEnd=13649
```
