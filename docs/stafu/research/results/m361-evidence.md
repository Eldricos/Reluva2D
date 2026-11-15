# M361 — le fixup page index verification

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m361-le-fixup-page-index-verification.txt`
- Original bytes: 19464
- SHA256: `2fe8a50e3d54ccf8f85ec637e371c800c3e0205fd0d1e222330ab16abf57ca11`
- Original encoding: `utf-16`
- Original line count: 78
- Curated lines retained: 35

## Selected diagnostic observations

- `M361_LAYOUT totalPages=371 pageSize=4096 fixupIndexPhysical=231518 fixupRecordPhysical=233006 dataOrigin=503700`
- `M361_INDEX logical=16 tableIndexZero=15 startZero=16893 endZero=17408 bytesZero=515 tableIndexOne=16 startOne=17408 endOne=17794 bytesOne=386`
- `M361_INDEX logical=17 tableIndexZero=16 startZero=17408 endZero=17794 bytesZero=386 tableIndexOne=17 startOne=17794 endOne=19341 bytesOne=1547`
- `M361_INDEX logical=18 tableIndexZero=17 startZero=17794 endZero=19341 bytesZero=1547 tableIndexOne=18 startOne=19341 endOne=20500 bytesOne=1159`
- `M361_INDEX logical=19 tableIndexZero=18 startZero=19341 endZero=20500 bytesZero=1159 tableIndexOne=19 startOne=20500 endOne=21898 bytesOne=1398`
- `M361_INDEX logical=20 tableIndexZero=19 startZero=20500 endZero=21898 bytesZero=1398 tableIndexOne=20 startOne=21898 endOne=22095 bytesOne=197`
- `M361_INDEX logical=21 tableIndexZero=20 startZero=21898 endZero=22095 bytesZero=197 tableIndexOne=21 startOne=22095 endOne=22235 bytesOne=140`
- `M361_INDEX logical=22 tableIndexZero=21 startZero=22095 endZero=22235 bytesZero=140 tableIndexOne=22 startOne=22235 endOne=22642 bytesOne=407`
- `M361_TABLE_BEGIN name=object_state expectedPage=19 offset=0x334 tableRva=0x12334`
- `M361_RECONSTRUCTION name=object_state mode=zero_based pageShift=-2 targetLogical=17 sourcePhysical=570056 complete=1 originalHex=1c5d5f5e5bc38bc0535657558b6c2414833d28ef1100017d3b8b1d78 patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200 changedBytes=28`
- `M361_RECONSTRUCTION name=object_state mode=zero_based pageShift=-1 targetLogical=18 sourcePhysical=574152 complete=1 originalHex=81ec18000000837d18007e06837d1c007f0ec7451840010000c7451c patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200 changedBytes=25`
- `M361_RECONSTRUCTION name=object_state mode=zero_based pageShift=0 targetLogical=19 sourcePhysical=578248 complete=1 originalHex=e089c1f7d18b451421483ceb08837dd82074d8ebdf8b45f80145fce9 patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200 changedBytes=28`
- `M361_RECONSTRUCTION name=object_state mode=zero_based pageShift=1 targetLogical=20 sourcePhysical=582344 complete=1 originalHex=e9280100008b45148b504ac1fa108b45148b404cc1f8100fafc283c0 patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200 changedBytes=27`
- `M361_RECONSTRUCTION name=object_state mode=zero_based pageShift=2 targetLogical=21 sourcePhysical=586440 complete=1 originalHex=8b45e88b5070c1fa108b45e80faf909800000081c280000000c1fa08 patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200 changedBytes=27`
- `M361_RECONSTRUCTION name=object_state mode=one_based complete=0`
- `M361_NEIGHBOR_STREAM name=object_state logical=17 mode=zero_based streamIndex=16 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=17 mode=one_based streamIndex=17 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=18 mode=zero_based streamIndex=17 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=18 mode=one_based streamIndex=18 matches=7 slots=6,5,4,3,2,1,0`
- `M361_NEIGHBOR_STREAM name=object_state logical=19 mode=zero_based streamIndex=18 matches=7 slots=6,5,4,3,2,1,0`
- `M361_NEIGHBOR_STREAM name=object_state logical=19 mode=one_based streamIndex=19 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=20 mode=zero_based streamIndex=19 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=20 mode=one_based streamIndex=20 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=21 mode=zero_based streamIndex=20 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=object_state logical=21 mode=one_based streamIndex=21 matches=0 slots=none`
- `M361_TABLE_END name=object_state`
- `M361_TABLE_BEGIN name=global_state expectedPage=20 offset=0x488 tableRva=0x13488`
- `M361_RECONSTRUCTION name=global_state mode=zero_based pageShift=-2 targetLogical=18 sourcePhysical=574492 complete=1 originalHex=8b45148b504ac1fa108b45140faf909800000081c280000000c1fa08 patchedHex=98350200983502006e350200ed340200983502006e35020061350200 changedBytes=27`
- `M361_RECONSTRUCTION name=global_state mode=zero_based pageShift=-1 targetLogical=19 sourcePhysical=578588 complete=1 originalHex=f80145fc8b45fc8b5514894244e9090e00008b4514804815018b4514 patchedHex=98350200983502006e350200ed340200983502006e35020061350200 changedBytes=28`
- `M361_RECONSTRUCTION name=global_state mode=zero_based pageShift=0 targetLogical=20 sourcePhysical=582684 complete=1 originalHex=e883783400750cc745e4ffffffffe9f50300008b45e88b40348b4002 patchedHex=98350200983502006e350200ed340200983502006e35020061350200 changedBytes=28`
- `M361_RECONSTRUCTION name=global_state mode=zero_based pageShift=1 targetLogical=21 sourcePhysical=586780 complete=1 originalHex=eb558b45ec8b401889856cffffff8b45e850e805daffff83c404a1b8 patchedHex=98350200983502006e350200ed340200983502006e35020061350200 changedBytes=28`
- `M361_RECONSTRUCTION name=global_state mode=zero_based pageShift=2 targetLogical=22 sourcePhysical=590876 complete=1 originalHex=00008945f8837df8007509c745f400000000eb3a8b45f86631c03d00 patchedHex=98350200983502006e350200ed340200983502006e35020061350200 changedBytes=26`
- `M361_RECONSTRUCTION name=global_state mode=one_based complete=0`
- `M361_NEIGHBOR_STREAM name=global_state logical=18 mode=zero_based streamIndex=17 matches=0 slots=none`
- `M361_NEIGHBOR_STREAM name=global_state logical=18 mode=one_based streamIndex=18 matches=0 slots=none`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
