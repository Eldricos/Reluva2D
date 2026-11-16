# m275-full-output.txt — curated observations

**Source:** `ResearchArchive.zip` / `m275-full-output.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `8cdb40265b62d2fe21aa05b794a618790af5e66c042a1a3d05e72a04c9ad0b29`; **encoding:** `utf-16-le`; **original lines:** 24; **selected lines:** 23.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
reluva-tool.exe : SPR_M275_ERROR resource=BRDGRIKE.SPR resource not found or unsafe name
At line:1 char:1
+ & $tool -stafu spr-m275-payload-structural-survey $install $cd *> m27 ...
+ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
+ CategoryInfo          : NotSpecified: (SPR_M275_ERROR ... or unsafe name:String) [], RemoteException
+ FullyQualifiedErrorId : NativeCommandError
SPR_M275_RECORD resource=BRDGPICA.SPR record=0 images=36 bytes=17521 referenceLike=29 neighborPrefix4=2 neighborPrefix8=2
SPR_M275_RECORD resource=BRDGPICA.SPR record=1 images=1 bytes=4 referenceLike=1 neighborPrefix4=0 neighborPrefix8=0
SPR_M275_RECORD resource=BRDGPICA.SPR record=2 images=10 bytes=4089 referenceLike=1 neighborPrefix4=8 neighborPrefix8=8
SPR_M275_RECORD resource=BRDGPICA.SPR record=3 images=82 bytes=47447 referenceLike=75 neighborPrefix4=1 neighborPrefix8=0
SPR_M275_RECORD resource=BRDGPICA.SPR record=4 images=8 bytes=32 referenceLike=8 neighborPrefix4=0 neighborPrefix8=0
SPR_M275_RECORD resource=BRDGPICA.SPR record=5 images=52 bytes=90863 referenceLike=44 neighborPrefix4=2 neighborPrefix8=2
SPR_M275_MARKER resource=BRDGPICA.SPR marker=0x0001000D count=13 bytes=6406 minBytes=446 maxBytes=603 size4=0
SPR_M275_MARKER resource=BRDGPICA.SPR marker=0x0002000D count=18 bytes=152918 minBytes=3 maxBytes=12935 size4=0
SPR_M275_MARKER resource=BRDGPICA.SPR marker=0x00030000 count=158 bytes=632 minBytes=4 maxBytes=4 size4=158
SPR_M275_RESOURCE resource=BRDGPICA.SPR records=6 recordsWithImages=6 images=189 referenceLike=158 otherImages=31 invalid=0 neighbors=183 prefix4=13 prefix8=12
SPR_M275_RECORD resource=BRDGDATA.SPR record=0 images=14 bytes=19178 referenceLike=7 neighborPrefix4=6 neighborPrefix8=6
SPR_M275_RECORD resource=BRDGDATA.SPR record=1 images=1 bytes=4 referenceLike=1 neighborPrefix4=0 neighborPrefix8=0
SPR_M275_RECORD resource=BRDGDATA.SPR record=2 images=10 bytes=2274 referenceLike=1 neighborPrefix4=8 neighborPrefix8=8
SPR_M275_RECORD resource=BRDGDATA.SPR record=3 images=33 bytes=699 referenceLike=30 neighborPrefix4=2 neighborPrefix8=2
SPR_M275_MARKER resource=BRDGDATA.SPR marker=0x0002000D count=19 bytes=21999 minBytes=190 maxBytes=5880 size4=0
SPR_M275_MARKER resource=BRDGDATA.SPR marker=0x00030000 count=39 bytes=156 minBytes=4 maxBytes=4 size4=39
SPR_M275_RESOURCE resource=BRDGDATA.SPR records=4 recordsWithImages=4 images=58 referenceLike=39 otherImages=19 invalid=0 neighbors=54 prefix4=16 prefix8=16
```
