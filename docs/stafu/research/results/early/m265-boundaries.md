# m265-boundaries.txt — curated observations

**Source:** `ResearchArchive.zip` / `m265-boundaries.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `1bc006f4c5164f969220ba6742c7d4bfc5156a6572d4e974540745472a52f15f`; **encoding:** `utf-16-le`; **original lines:** 16; **selected lines:** 16.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M265_EXTEND resource=BRDGPICA.SPR gap=3 seedA=9874 seedB=9933 seedLength=34 left=0 right=0 a=9874 b=9933 length=34 displacement=59
SPR_M265_EXTEND resource=BRDGPICA.SPR gap=3 seedA=9920 seedB=9989 seedLength=388 left=0 right=0 a=9920 b=9989 length=388 displacement=69
SPR_M265_INTERVAL resource=BRDGPICA.SPR gap=3 from=1 to=2 a=9908 b=9967 bitsA=12 bitsB=22 deltaBits=10
SPR_M265_CANDIDATES resource=BRDGPICA.SPR gap=3 from=1 to=2 minRun=12 total=0 reported=0
SPR_M265_EXTEND resource=BRDGDATA.SPR gap=2 seedA=10873 seedB=10819 seedLength=308 left=0 right=0 a=10873 b=10819 length=308 displacement=-54
SPR_M265_EXTEND resource=BRDGDATA.SPR gap=2 seedA=11255 seedB=11204 seedLength=314 left=0 right=0 a=11255 b=11204 length=314 displacement=-51
SPR_M265_EXTEND resource=BRDGDATA.SPR gap=2 seedA=11645 seedB=11597 seedLength=283 left=0 right=0 a=11645 b=11597 length=283 displacement=-48
SPR_M265_INTERVAL resource=BRDGDATA.SPR gap=2 from=1 to=2 a=11181 b=11127 bitsA=74 bitsB=77 deltaBits=3
SPR_M265_CANDIDATES resource=BRDGDATA.SPR gap=2 from=1 to=2 minRun=12 total=1 reported=1
SPR_M265_HIT resource=BRDGDATA.SPR gap=2 from=1 to=2 rank=1 a=11221 b=11163 length=13 displacement=-58
SPR_M265_INTERVAL resource=BRDGDATA.SPR gap=2 from=2 to=3 a=11569 b=11518 bitsA=76 bitsB=79 deltaBits=3
SPR_M265_CANDIDATES resource=BRDGDATA.SPR gap=2 from=2 to=3 minRun=12 total=2 reported=2
SPR_M265_HIT resource=BRDGDATA.SPR gap=2 from=2 to=3 rank=1 a=11573 b=11525 length=13 displacement=-48
SPR_M265_HIT resource=BRDGDATA.SPR gap=2 from=2 to=3 rank=2 a=11619 b=11574 length=13 displacement=-45
SPR_M265_SUMMARY seeds=5 resources=2
SPR_M265_NOTE Exact MSB-first extension within original M251 gaps, no edit costs. Interior candidates are maximal diagonal runs >=12 bits, top 8 by length; not codec operations.
```
