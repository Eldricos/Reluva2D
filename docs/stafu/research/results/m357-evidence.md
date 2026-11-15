# M357 — relocated jump table reconstruction

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m357-relocated-jump-table-reconstruction.txt`
- Original bytes: 36868
- SHA256: `19119dbd2e2df3b7cbdc39beefb9ec9e6cdb44675ea7c62c50b4b396fb82c77d`
- Original encoding: `utf-16`
- Original line count: 165
- Curated lines retained: 35

## Selected diagnostic observations

- `M357_LAYOUT pages=371 pageSize=4096 objects=2 dataOrigin=503700`
- `M357_TABLE_BEGIN name=object_state tableRva=0x12334 logicalPage=19 fixupBytes=1159`
- `M357_CHAIN name=object_state complete=1 reverseNineByte=1 positions=885,876,867,858,849,840,831`
- `M357_TARGET_MAP name=object_state state=1 mode=object_relative rva=0x1239b physical=0x8d32f`
- `M357_TARGET_MAP name=object_state state=1 mode=linear_minus_object_base rva=0x1239b physical=0x8d32f`
- `M357_TARGET_MAP name=object_state state=1 mode=linear_as_rva rva=0x2239b physical=0x9d32f`
- `M357_TARGET_MAP name=object_state state=2 mode=object_relative rva=0x1238e physical=0x8d322`
- `M357_TARGET_MAP name=object_state state=2 mode=linear_minus_object_base rva=0x1238e physical=0x8d322`
- `M357_TARGET_MAP name=object_state state=2 mode=linear_as_rva rva=0x2238e physical=0x9d322`
- `M357_TARGET_MAP name=object_state state=3 mode=object_relative rva=0x123ca physical=0x8d35e`
- `M357_TARGET_MAP name=object_state state=3 mode=linear_minus_object_base rva=0x123ca physical=0x8d35e`
- `M357_TARGET_MAP name=object_state state=3 mode=linear_as_rva rva=0x223ca physical=0x9d35e`
- `M357_TARGET_MAP name=object_state state=4 mode=object_relative rva=0x123f8 physical=0x8d38c`
- `M357_TARGET_MAP name=object_state state=4 mode=linear_minus_object_base rva=0x123f8 physical=0x8d38c`
- `M357_TARGET_MAP name=object_state state=4 mode=linear_as_rva rva=0x223f8 physical=0x9d38c`
- `M357_TARGET_MAP name=object_state state=5 mode=object_relative rva=0x123eb physical=0x8d37f`
- `M357_TARGET_MAP name=object_state state=5 mode=linear_minus_object_base rva=0x123eb physical=0x8d37f`
- `M357_TARGET_MAP name=object_state state=5 mode=linear_as_rva rva=0x223eb physical=0x9d37f`
- `M357_TARGET_MAP name=object_state state=6 mode=object_relative rva=0x123ca physical=0x8d35e`
- `M357_TARGET_MAP name=object_state state=6 mode=linear_minus_object_base rva=0x123ca physical=0x8d35e`
- `M357_TARGET_MAP name=object_state state=6 mode=linear_as_rva rva=0x223ca physical=0x9d35e`
- `M357_TARGET_MAP name=object_state state=7 mode=object_relative rva=0x12429 physical=0x8d3bd`
- `M357_TARGET_MAP name=object_state state=7 mode=linear_minus_object_base rva=0x12429 physical=0x8d3bd`
- `M357_TARGET_MAP name=object_state state=7 mode=linear_as_rva rva=0x22429 physical=0x9d3bd`
- `M357_RECONSTRUCTED name=object_state originalHex=e089c1f7d18b451421483ceb08837dd82074d8ebdf8b45f80145fce9 patchedHex=9b2302008e230200ca230200f8230200eb230200ca23020029240200`
- `M357_TABLE_END name=object_state`
- `M357_TABLE_BEGIN name=global_state tableRva=0x13488 logicalPage=20 fixupBytes=1398`
- `M357_CHAIN name=global_state complete=1 reverseNineByte=1 positions=1285,1276,1267,1258,1249,1240,1231`
- `M357_TARGET_MAP name=global_state state=0 mode=object_relative rva=0x13598 physical=0x8e52c`
- `M357_TARGET_MAP name=global_state state=0 mode=linear_minus_object_base rva=0x13598 physical=0x8e52c`
- `M357_TARGET_MAP name=global_state state=0 mode=linear_as_rva rva=0x23598 physical=0x9e52c`
- `M357_TARGET_MAP name=global_state state=1 mode=object_relative rva=0x13598 physical=0x8e52c`
- `M357_TARGET_MAP name=global_state state=1 mode=linear_minus_object_base rva=0x13598 physical=0x8e52c`
- `M357_TARGET_MAP name=global_state state=1 mode=linear_as_rva rva=0x23598 physical=0x9e52c`
- `M357_TARGET_MAP name=global_state state=2 mode=object_relative rva=0x1356e physical=0x8e502`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
