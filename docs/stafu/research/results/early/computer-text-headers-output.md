# computer-text-headers-output.txt — curated observations

**Source:** `ResearchArchive.zip` / `computer-text-headers-output.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `b4c53b21ef4ddfce9c3d89534651d335936dac7fddd4b3e10b0e83cfce5cd93a`; **encoding:** `utf-16-le`; **original lines:** 189; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
computer.db=437414 entries=346 base=0x570
UNRESOLVED entry=277 field=2 header=(0,0) primary=0x00061c5e stored=0x00061d00 absolute=0x62270 status=non-text
acceptedPrefix=36 firstStop=0x0a at=0x62294
00062274: 74 69 6f 6e 3a 09 4b 72 69 64 6e 61 72 20 41 73 |tion:.Kridnar As|
00062284: 74 72 6f 67 61 74 69 6f 6e 20 42 6c 6f 63 6b 0d |trogation Block.|
00062294: 0a 53 65 63 74 6f 72 20 4f 66 66 73 65 74 3a 20 |.Sector Offset: |
000622a4: 34 2e 32 2e 35 0d 53 74 65 6c 6c 61 72 20 4f 66 |4.2.5.Stellar Of|
000622b4: 66 73 65 74 3a 20 38 2e 35 2e 35 0d 50 6c 61 6e |fset: 8.5.5.Plan|
nulOffset=0x6259a bytesToNul=810
unsupported bytes before NUL:
0x0a count=1
Header A frequency:
0: 270
1: 9
2: 26
3: 9
4: 8
5: 4
6: 4
7: 3
8: 1
9: 5
11: 2
14: 1
16: 1
17: 1
23: 1
28: 1
Header B frequency:
0: 281
2: 50
4: 15
Header pair frequency:
(0,0): 219
(0,2): 36
(0,4): 15
(1,0): 5
(1,2): 4
(2,0): 23
(2,2): 3
(3,0): 8
(3,2): 1
(4,0): 6
(4,2): 2
(5,0): 3
(5,2): 1
(6,0): 4
(7,0): 3
(8,0): 1
(9,0): 3
(9,2): 2
(11,0): 2
(14,0): 1
(16,0): 1
(17,2): 1
(23,0): 1
(28,0): 1
Entries with nonzero headers (neighbor context; labels only):
[0] header=(9,2) label="Primary Core" next="USS Enterprise"
[1] header=(3,2) label="USS Enterprise" next="Systems"
[2] header=(17,2) label="Systems" next="Structure"
[3] header=(4,0) label="Structure" next="Hull"
[8] header=(1,0) label="Bridge" next="Duties"
[10] header=(2,0) label="Computers" next="Cores"
[12] header=(1,2) label="Tricorder" next="Medical"
[14] header=(2,0) label="Engineering" next="Warp Drive"
[15] header=(3,0) label="Warp Drive" next="Subspace"
[16] header=(2,0) label="Subspace" next="Anomalies"
[22] header=(3,0) label="Comms" next="Communicator"
[29] header=(2,0) label="Weapons" next="Phasers"
[30] header=(2,0) label="Phasers" next="Shipboard"
[41] header=(2,0) label="Astrogation" next="Sector"
[44] header=(9,0) label="Bridge Crew" next="Picard"
[55] header=(4,2) label="Sciences" next="Astronomy"
[56] header=(4,2) label="Astronomy" next="Planet"
[57] header=(2,0) label="Planet" next="Sol III"
[59] header=(16,0) label="Classes" next="Class A"
[60] header=(0,2) label="Class A" next="Class B"
[61] header=(0,2) label="Class B" next="Class C"
[62] header=(0,2) label="Class C" next="Class D"
[63] header=(0,2) label="Class D" next="Class E"
[64] header=(0,2) label="Class E" next="Class F"
[65] header=(0,2) label="Class F" next="Class G"
[66] header=(0,2) label="Class G" next="Class H"
[67] header=(0,2) label="Class H" next="Class I"
[68] header=(0,2) label="Class I" next="Class J"
[69] header=(0,2) label="Class J" next="Class K"
[70] header=(0,2) label="Class K" next="Class L"
[71] header=(0,2) label="Class L" next="Class M"
[72] header=(0,2) label="Class M" next="Class N"
```
