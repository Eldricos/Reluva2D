# STTNG.OVL executable layout and loader

**Input identity:** `STTNG.OVL`, 2,008,455 bytes, SHA256 `65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697`.

**Confirmed static observations from earlier diagnostics:**
- Embedded MZ image: 62,580 bytes; header 0x200; entry file offset 0x4152; CS 0x1BD, IP 0x2382; 78 relocations.
- DOS4GW loader marker `BW` observed after MZ image at file offset 0xF474.
- Embedded LE header file offset 0x38194.
- LE object 1: base 0x10000, size 0xD889A (886,938 bytes), 217 pages.
- LE object 2: base 0xF0000, size 0x1821B0, 154 pages.
- LE page map file offset 0x38288; fixup page table 0x3885E; fixup records 0x38E2E; data origin 0x7AF94; object 2 data origin 0x153F94.
- M349–M384 examined LE fixups; M385–M419 examined DOS4GW loader behavior. These investigations are distinct from verified SPR decompression.

**Caution:** physical file offsets are not runtime addresses. Any linear mapping assumptions must be stated for each disassembly.

**Provenance:** RM_M190–M197, M349–M419 diagnostics; detailed TXT outputs pending archival.
