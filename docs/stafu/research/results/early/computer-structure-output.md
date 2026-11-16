# computer-structure-output.txt — curated observations

**Source:** `ResearchArchive.zip` / `computer-structure-output.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `40fc77b73a869cefc9d34d66c42cc3cc4dc0b4641e28c3b84a9e570b302b430a`; **encoding:** `utf-16-le`; **original lines:** 624; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
entries=346 tableEnd=0x56c base=0x570 secondaryRefs=9
Nine-word primary-reference candidate:
abs=0x584 target=0x00006a69 index=1 header=(3,2) label="USS Enterprise"
abs=0x588 target=0x0000f5d8 index=55 header=(4,2) label="Sciences"
abs=0x58c target=0x0002c646 index=162 header=(2,2) label="Starfleet"
abs=0x590 target=0x0002e756 index=166 header=(9,2) label="Cultures"
abs=0x594 target=0x00035c95 index=185 header=(7,0) label="Local Area"
abs=0x598 target=0x0003717d index=192 header=(4,0) label="Missions"
abs=0x59c target=0x00063489 index=285 header=(11,0) label="Tactical"
abs=0x5a0 target=0x000675db index=319 header=(3,0) label="Destinations"
abs=0x5a4 target=0x000689e5 index=338 header=(7,0) label="Analyses"
Words immediately after primary table (potential navigation header):
abs=0x56c word=0x0006a736
abs=0x570 word=0x00020009
abs=0x574 word=0x00000091
abs=0x578 word=0x0000009e
abs=0x57c word=0x000000b9
abs=0x580 word=0x00000101
abs=0x584 word=0x00006a69 PRIMARY[1] "USS Enterprise"
abs=0x588 word=0x0000f5d8 PRIMARY[55] "Sciences"
abs=0x58c word=0x0002c646 PRIMARY[162] "Starfleet"
abs=0x590 word=0x0002e756 PRIMARY[166] "Cultures"
abs=0x594 word=0x00035c95 PRIMARY[185] "Local Area"
abs=0x598 word=0x0003717d PRIMARY[192] "Missions"
abs=0x59c word=0x00063489 PRIMARY[285] "Tactical"
abs=0x5a0 word=0x000675db PRIMARY[319] "Destinations"
abs=0x5a4 word=0x000689e5 PRIMARY[338] "Analyses"
abs=0x5a8 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5ac word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b0 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b4 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b8 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5bc word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5c0 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5c4 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5c8 word=0x00000000 PRIMARY[0] "Primary Core"
Selected metadata records and following 64 bytes:
[0] label="Primary Core" primary=0x00000000 metadata=0x570 header=(9,2) parent=ROOT
abs=0x580 word=0x00000101
abs=0x584 word=0x00006a69 PRIMARY[1] "USS Enterprise"
abs=0x588 word=0x0000f5d8 PRIMARY[55] "Sciences"
abs=0x58c word=0x0002c646 PRIMARY[162] "Starfleet"
abs=0x590 word=0x0002e756 PRIMARY[166] "Cultures"
abs=0x594 word=0x00035c95 PRIMARY[185] "Local Area"
abs=0x598 word=0x0003717d PRIMARY[192] "Missions"
abs=0x59c word=0x00063489 PRIMARY[285] "Tactical"
abs=0x5a0 word=0x000675db PRIMARY[319] "Destinations"
abs=0x5a4 word=0x000689e5 PRIMARY[338] "Analyses"
abs=0x5a8 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5ac word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b0 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b4 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5b8 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x5bc word=0x00000000 PRIMARY[0] "Primary Core"
[1] label="USS Enterprise" primary=0x00006a69 metadata=0x6fd9 header=(3,2) parent=0
abs=0x6fe9 word=0x00006ccf
abs=0x6fed word=0x00007151 PRIMARY[2] "Systems"
abs=0x6ff1 word=0x0000db5a PRIMARY[44] "Bridge Crew"
abs=0x6ff5 word=0x0000f3cc PRIMARY[54] "Support Craft"
abs=0x6ff9 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x6ffd word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7001 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7005 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7009 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x700d word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7011 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7015 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7019 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x701d word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7021 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0x7025 word=0x00000000 PRIMARY[0] "Primary Core"
[55] label="Sciences" primary=0x0000f5d8 metadata=0xfb48 header=(4,2) parent=0
abs=0xfb58 word=0x0000f6ce
abs=0xfb5c word=0x0000ffce PRIMARY[56] "Astronomy"
abs=0xfb60 word=0x0002800c PRIMARY[140] "Biology"
abs=0xfb64 word=0x00028f8c PRIMARY[143] "Archaeology"
abs=0xfb68 word=0x00029d94 PRIMARY[144] "Technology"
abs=0xfb6c word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb70 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb74 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb78 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb7c word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb80 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb84 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb88 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb8c word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb90 word=0x00000000 PRIMARY[0] "Primary Core"
abs=0xfb94 word=0x00000000 PRIMARY[0] "Primary Core"
[162] label="Starfleet" primary=0x0002c646 metadata=0x2cbb6 header=(2,2) parent=55
abs=0x2cbc6 word=0x0002c82f
```
