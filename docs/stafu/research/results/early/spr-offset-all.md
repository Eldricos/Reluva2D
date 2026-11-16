# spr-offset-all.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-offset-all.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `66b76f4ceb53f2717f737912cf1cd1ed5bdb636fc8582cfc932438d9765c3d8e`; **encoding:** `utf-16-le`; **original lines:** 1190; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_OFFSET_RESOURCE resource=BRDGDATA.SPR size=24363 records=4
SPR_OFFSET_SOURCE record=0 image=7 chunk=18 tag=MOCS marker=0x00030000 bytes=d10b0000 le=3025 be=3507159040
CANDIDATE endian=LE value=3025 base=absolute target=0xbd1 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=3025 base=record-relative target=0xbf9 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=BE value=3507159040 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=8 chunk=20 tag=MOCS marker=0x00030000 bytes=d6110000 le=4566 be=3591438336
CANDIDATE endian=LE value=4566 base=absolute target=0x11d6 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=4566 base=record-relative target=0x11fe region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=BE value=3591438336 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=9 chunk=22 tag=MOCS marker=0x00030000 bytes=e8170000 le=6120 be=3893821440
CANDIDATE endian=LE value=6120 base=absolute target=0x17e8 region=image-data boundary=none record=0 chunk=5
CANDIDATE endian=LE value=6120 base=record-relative target=0x1810 region=image-data boundary=none record=0 chunk=5
CANDIDATE endian=BE value=3893821440 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=10 chunk=24 tag=MOCS marker=0x00030000 bytes=f11d0000 le=7665 be=4045209600
CANDIDATE endian=LE value=7665 base=absolute target=0x1df1 region=image-data boundary=none record=0 chunk=5
CANDIDATE endian=LE value=7665 base=record-relative target=0x1e19 region=image-data boundary=none record=0 chunk=5
CANDIDATE endian=BE value=4045209600 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=11 chunk=26 tag=PMOC marker=0x00030000 bytes=32350000 le=13618 be=842334208
CANDIDATE endian=LE value=13618 base=absolute target=0x3532 region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=LE value=13618 base=record-relative target=0x355a region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=BE value=842334208 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=12 chunk=28 tag=PMOC marker=0x00030000 bytes=7e4c0000 le=19582 be=2118909952
CANDIDATE endian=LE value=19582 base=absolute target=0x4c7e region=payload boundary=payload-start record=0 chunk=24
CANDIDATE endian=LE value=19582 base=record-relative target=0x4ca6 region=payload boundary=none record=0 chunk=26
CANDIDATE endian=BE value=2118909952 base=none status=out-of-range
SPR_OFFSET_SOURCE record=0 image=13 chunk=30 tag=PMOC marker=0x00030000 bytes=7e350000 le=13694 be=2117402624
CANDIDATE endian=LE value=13694 base=absolute target=0x357e region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=LE value=13694 base=record-relative target=0x35a6 region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=BE value=2117402624 base=none status=out-of-range
SPR_OFFSET_SOURCE record=1 image=0 chunk=0 tag=PMOC marker=0x00030000 bytes=b6350000 le=13750 be=3056926720
CANDIDATE endian=LE value=13750 base=absolute target=0x35b6 region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=BE value=3056926720 base=none status=out-of-range
SPR_OFFSET_SOURCE record=2 image=0 chunk=0 tag=PMOC marker=0x00030000 bytes=06360000 le=13830 be=104202240
CANDIDATE endian=LE value=13830 base=absolute target=0x3606 region=image-data boundary=none record=0 chunk=10
CANDIDATE endian=BE value=104202240 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=3 chunk=8 tag=MOCS marker=0x00030000 bytes=e3000000 le=227 be=3808428032
CANDIDATE endian=LE value=227 base=absolute target=0xe3 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=227 base=record-relative target=0x589b region=chunk boundary=none record=3 chunk=2
CANDIDATE endian=BE value=3808428032 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=4 chunk=10 tag=MOCS marker=0x00030000 bytes=d7020000 le=727 be=3607232512
CANDIDATE endian=LE value=727 base=absolute target=0x2d7 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=727 base=record-relative target=0x5a8f region=chunk boundary=chunk-start record=3 chunk=9
CANDIDATE endian=BE value=3607232512 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=5 chunk=12 tag=MOCS marker=0x00030000 bytes=0d020000 le=525 be=218234880
CANDIDATE endian=LE value=525 base=absolute target=0x20d region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=525 base=record-relative target=0x59c5 region=image-data boundary=none record=3 chunk=6
CANDIDATE endian=BE value=218234880 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=6 chunk=14 tag=MOCS marker=0x00030000 bytes=53010000 le=339 be=1392574464
CANDIDATE endian=LE value=339 base=absolute target=0x153 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=339 base=record-relative target=0x590b region=image-data boundary=none record=3 chunk=4
CANDIDATE endian=BE value=1392574464 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=7 chunk=16 tag=MOCS marker=0x00030000 bytes=77010000 le=375 be=1996554240
CANDIDATE endian=LE value=375 base=absolute target=0x177 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=375 base=record-relative target=0x592f region=image-data boundary=none record=3 chunk=4
CANDIDATE endian=BE value=1996554240 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=8 chunk=18 tag=MOCS marker=0x00030000 bytes=6b030000 le=875 be=1795358720
CANDIDATE endian=LE value=875 base=absolute target=0x36b region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=875 base=record-relative target=0x5b23 region=chunk boundary=chunk-start record=3 chunk=17
CANDIDATE endian=BE value=1795358720 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=9 chunk=20 tag=PMOC marker=0x00030000 bytes=175b0000 le=23319 be=391839744
CANDIDATE endian=LE value=23319 base=absolute target=0x5b17 region=payload boundary=none record=3 chunk=16
CANDIDATE endian=BE value=391839744 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=10 chunk=23 tag=PMOC marker=0x00030000 bytes=27440000 le=17447 be=658767872
CANDIDATE endian=LE value=17447 base=absolute target=0x4427 region=image-data boundary=none record=0 chunk=14
CANDIDATE endian=BE value=658767872 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=11 chunk=26 tag=MOCS marker=0x00030000 bytes=05030000 le=773 be=84082688
CANDIDATE endian=LE value=773 base=absolute target=0x305 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=773 base=record-relative target=0x5abd region=payload boundary=none record=3 chunk=11
CANDIDATE endian=BE value=84082688 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=12 chunk=29 tag=MOCS marker=0x00030000 bytes=5b020000 le=603 be=1526857728
CANDIDATE endian=LE value=603 base=absolute target=0x25b region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=603 base=record-relative target=0x5a13 region=image-data boundary=none record=3 chunk=6
CANDIDATE endian=BE value=1526857728 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=13 chunk=31 tag=MOCS marker=0x00030000 bytes=7f020000 le=639 be=2130837504
CANDIDATE endian=LE value=639 base=absolute target=0x27f region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=639 base=record-relative target=0x5a37 region=image-data boundary=none record=3 chunk=6
CANDIDATE endian=BE value=2130837504 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=14 chunk=33 tag=MOCS marker=0x00030000 bytes=73040000 le=1139 be=1929641984
CANDIDATE endian=LE value=1139 base=absolute target=0x473 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=1139 base=record-relative target=0x5c2b region=chunk boundary=chunk-start record=3 chunk=32
CANDIDATE endian=BE value=1929641984 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=15 chunk=35 tag=MOCS marker=0x00030000 bytes=a5030000 le=933 be=2768437248
CANDIDATE endian=LE value=933 base=absolute target=0x3a5 region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=933 base=record-relative target=0x5b5d region=chunk boundary=none record=3 chunk=20
CANDIDATE endian=BE value=2768437248 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=16 chunk=37 tag=MOCS marker=0x00030000 bytes=eb020000 le=747 be=3942776832
CANDIDATE endian=LE value=747 base=absolute target=0x2eb region=image-data boundary=none record=0 chunk=2
CANDIDATE endian=LE value=747 base=record-relative target=0x5aa3 region=payload boundary=payload-start record=3 chunk=10
CANDIDATE endian=BE value=3942776832 base=none status=out-of-range
SPR_OFFSET_SOURCE record=3 image=17 chunk=39 tag=MOCS marker=0x00030000 bytes=0f030000 le=783 be=251854848
```
