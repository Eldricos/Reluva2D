# Palette and rendering findings

**Confirmed static observations:**
- `BRIDGE.IMG`, `DBRIDGE.IMG`, and `color.img` were reported as 256-entry palettes; for the compared standard 128 entries, 2 matched and 126 differed in the reported test.
- `BRIDGE.RM` produced 307,200 pixel indices; reported counts: indices 0–127 = **169,658**, indices 128–255 = **137,542**. Distinct indices: 125 low and 99 high.
- Region diagnostics split `BRIDGE.RM` into 160×120 tiles and measured high-index coverage. This is observational evidence of palette-index usage, not proof of the original renderer's compositing rules.
- RM_M183–M197 traced palette initialization/copy and a decompressor-related dispatcher/Huffman-helper candidate in `STTNG.OVL`. These are static disassembly results; do not equate them with a proven SPR decoder.

**Unresolved:** mapping palette indices to final displayed colors, exact image/palette selection, and whether the traced decompressor is used for SPR records.

**Provenance:** palette coverage/region diagnostic outputs, RM_M183–M197 notes; full outputs not yet imported.
