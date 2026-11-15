# M447 curated evidence

**Source:** user-run `m447-buffered-control-helper-classification.txt` (516 lines), received 2026-11-15. **This is a curated summary, not a verbatim backup.**

**Input verification:** `STTNG.OVL`, 2,008,455 bytes, SHA256 `65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697`; `hashMatch=True`. Diagnostic finished `M447_DONE readOnly=1`.

**Script:** `scripts/stafu-m447-buffered-control-helper-classification.py`; introduction commit `8ba55ae11e7fa392a575f3d9eb4ef0761c9d27fe`.

| Function file offset | Decoded CFG instructions | Raw E8 caller matches | Key static observations |
|---|---:|---:|---|
| 0x89801 | 85 | 1 (0x7BF08) | Checks global 0x12F78, calls stream acquisition 0x7E410, allocates buffer via 0x8F790, updates 0x9EE68/0x9EE70/0x12F7C |
| 0x8C536 | 211 | 59 | Validates index using object +0x94, table at +0x88, updates index at +0x8C, position-like +0x44, flags +0x38, calls 0x8CFD3 |
| 0x7C34C | 16 | 1 (0x7C028) | Returns 32-bit `[object+0x44]` |
| 0x7C36C | 16 | 34 | Returns 32-bit `[object+0x38]` |

**Caller evidence:** at 0x7BF08 caller passes 0x9F4 to 0x89801. At 0x7BFAC and 0x7C005, 0x8C536 receives index 0 and 1 respectively. At 0x7C028, result of 0x7C34C is compared against a local sum. At 0x7C04C, result of 0x7C36C is tested with mask 0x04. Another call at 0x7C034 invokes 0x90474 and compares its result with 0x1B.

**Interpretation:** buffered stream initialization and likely graphical/animation object state. Neither SPR decompression nor runtime execution of these paths is proven. Raw E8 scans and bounded CFG remain static evidence.

**Retention:** this evidence note avoids embedding instruction bytes from the proprietary executable. Full M447 output remains outside this repository pending a vetted archival decision.
