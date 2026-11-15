# STAFU experiment index

This index records milestone coverage and **does not claim all historical TXT outputs are in GitHub**. Script links are by naming convention; check the repository for individual paths.

| Milestones | Investigation | Key retained conclusion | Output archive status |
|---|---|---|---|
| Initial archive/inventory tests | ZIP, sidecar, installed/CD resources | 14,647 entries; matching central directories; resource loader verified | Pending |
| RM_M183–M197 | Palette, dispatcher, Huffman helper | Static helper/call/fixup evidence, no proven SPR decoder | Pending |
| M278–M325 | SPR record2 alignments/boundaries | Competing record hypotheses; no final decoder | Pending |
| M327–M348 | Resource-object reads/seeks | Resource infrastructure candidates | Pending |
| M349–M384 | LE fixups | LE relocation/fixup structure examined | Pending |
| M385–M419 | DOS4GW loader | Loader structure examined | Pending |
| M420–M427 | SPR strings, object2 table, reference | Table candidate; logging function not decoder | Pending |
| M428–M439 | Caller ranking, lifecycle, stream I/O | Generic object and file helpers | Pending |
| M440 | DOS read caller provenance | 23 raw caller candidates | Pending |
| M441 | Stream read consumers | 0x7E684 stream read paths | Pending |
| M442 | Buffered reader consumers | 0x89924 generic buffered transfer | Pending |
| M443 | Stream transfer provenance | Three direct 0x89924 callers | Pending |
| M444 | Buffered control semantics | 0x89801, 0x89D94 and state getters | Pending |
| M445 | Buffered byte consumers | No verified bit decoder; 0x89BF6 indexed getter | Pending |
| M446 | Buffered transfer caller semantics | 0x7BF23, 0x7BFF5, 0x7C016 confirmed | Pending |
| M447 | Buffered control helper classification | 0x8C536 graphical-state candidate; +0x44 and +0x38 getters | Pending |

## Exact recent scripts and outputs

| Milestone | Script in repository | User-run TXT filename |
|---|---|---|
| M440 | `scripts/stafu-m440-dos-read-caller-provenance.py` | `m440-dos-read-caller-provenance.txt` |
| M441 | `scripts/stafu-m441-stream-read-consumers.py` | `m441-stream-read-consumers.txt` |
| M442 | `scripts/stafu-m442-buffered-reader-consumers.py` | `m442-buffered-reader-consumers.txt` |
| M443 | `scripts/stafu-m443-stream-transfer-provenance.py` | `m443-stream-transfer-provenance.txt` |
| M444 | `scripts/stafu-m444-buffered-control-semantics.py` | `m444-buffered-control-semantics.txt` |
| M445 | `scripts/stafu-m445-buffered-byte-consumers.py` | `m445-buffered-byte-consumers.txt` |
| M446 | `scripts/stafu-m446-buffered-transfer-caller-semantics.py` | `m446-buffered-transfer-caller-semantics.txt` |
| M447 | `scripts/stafu-m447-buffered-control-helper-classification.py` | `m447-buffered-control-helper-classification.txt` |

## Preservation checklist
- [x] Research page structure and curated findings
- [x] Explicit uncertainty/confidence policy
- [x] Recent script/output mapping
- [ ] Complete sanitized TXT outputs imported to `results/`
- [ ] Older milestone-by-milestone index and output mapping verified against repository
- [ ] Automated cross-reference of milestone to script, input SHA, and output SHA
