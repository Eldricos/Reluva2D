# Star Trek: New Worlds: `.gmf` maps

Status: **first look**. Stored in the planet archives (`alnwrld`, `alpwrld`,
`deswrld`, `icewrld`, `redwrld`, `vlcwrld`) with chunk tag `0x11`, 3 to 7 per
archive, mostly 380–470 KB (`kurt64.gmf` in `icewrld` is 166 KB). Each
archive also holds that planet's ground tiles (`geo/map/alp/alpin_01.geo`
…, 93 in `alpwrld`) and terrain textures (`lbp/txr/map/alp/green01`,
`cliff01`, `brid01`, …). Worked out from the start of `coverd2.gmf`, October
2026. `reluva-tool gmf-info <file>` walks the layout below.

All values little-endian.

```
0x00   u16 count of type-21 records   376 in coverd2, 12 in kurt64
0x02   u16 count of type-20 records   376, 12
0x04   u16 count of type-19 records   749, 6
0x06   u16 grid width, u16 height     64 x 64, 32 x 32
0x0a   the records, in that order: u16 type, u16 length, u16 ? (0 so far),
       then length bytes
...    data of unknown layout up to the end of the file
```

| Type | Length | Notes |
|---|---|---|
| 21 | 148–155 | one per tile (see below): small items `00 00 00 k 00 80 00`, many `02 ff ff ff ff ff` |
| 20 | 338–362 | one per tile: u32 its own number (0, 1, …), u32 0, u32 ?, u32 `0x18000`, u16 ?, then 8 × 8 byte grids (5, plus 0–3 8-byte items); values such as 2, 8, `0b`, `0d`, `0e`, perhaps terrain textures per cell |
| 19 | 22 | a 2 × 2 **block** of tiles: 6 bytes (0), then four u32 tile numbers, e.g. (0, 1, 3, 2), (4, 5, 6, 7), (8, 5, 6, 7) |

So the type-21 and type-20 counts are the number of distinct **tiles** (each
with two records), type 19 builds 2 × 2 **blocks** from them, and the header's
grid is in blocks: `kurt64` is 32 × 32 blocks = 64 × 64 tiles, as its name
says. Ground-tile models (`geo/map/...`) also have an 8 × 8 face grid.

After the records (checked on `kurt64`, from the file itself):

```
u32 N                         37092 in kurt64, 39607 in coverd2
N × u32                       lists, each ended by ff ff ff ff; entries are
                              24-bit values with bit 23 set, e.g. 00a0b277
grid w × h × 10 bytes         one entry per grid cell, row by row:
                                u16 block number (0-5 in kurt64)
                                u16 bits 12-15: resource deposit kind (1-6,
                                0 none); bits 0-11: 50-300 (see below)
                                u16 bits 12-15 repeat the kind; rest 0
                                u32 where the cell's list starts in the N u32s
1024 bytes                    a colour table: u32 entries, bytes B, G, R, 0,
                              ending with a zero entry (see Terrain codes)
```

The lists are stored column by column (cell (0, 1) starts at 47, cell (1, 0)
at 1380). `kurt64`'s grid is a field of block 1 with scattered variants
(blocks 2-5) and a patch of block 0, as a test map would be.

### Resource deposits

Bits 12-15 of a grid cell's second value mark a resource deposit, kinds 1
to 6, and the third value's bits 12-15 always repeat it (every `icewrld`
map). The game has six raw resources (dilithium, talgonite, silicon,
kelbonite, magnesite, dolamide), so these are probably they; which kind is
which isn't known. The deposits are hand-painted patches: `j1` (a
multiplayer arena) is covered in large ones of every kind, `f1` (the first
Federation mission) has one cluster of all six plus two small patches, `r12`
only kind 1, `k12` kinds 1-3, and `kurt64` none.

The low 12 bits run 50-300 in steps of 12.5. Inside deposits they vary
(perhaps the amount); elsewhere they are mostly one value per map (50 in
`f1`, 250 in most others) with other values tracing the terrain. Unknown.

### Placed objects

The lists are the objects placed on the map (trees, rocks and the like):
each grid cell's list holds the objects in that block. Each entry is a u32:

| Bits | Meaning |
|---|---|
| 0-3 | x within the block, on a 16 x 16 grid (1024 units a square), from the west |
| 4-7 | y, from the south |
| 8-13 | unknown; evenly spread over 0-63 (a finer position?) |
| 14-15 | unknown variant: 1-3 for types 0 and 2, 0 or 3 for type 1 |
| 16-18 | type |
| 19-22 | evenly spread over 0-15: presumably a turn in sixteenths |
| 23 | always set |

The position was checked by continuity: trees grow in groves, and only this
reading (x low, y high, in map orientation) carries the groves across block
edges (correlation of neighbouring squares 0.21 across an edge, against
0.01-0.03 for the other readings, 0.32 inside a block). Trees are planted on
alternate squares, so they don't overlap (a tree is 2200 units across).
Oddly, no object in any `icewrld` map has x < 8 and y >= 8: one quarter of
every block is always empty, even in `kurt64`'s plain field, so those bits
may mean something else.

Types in `icewrld` (`f1`): 1 x25472 (trees, by their planting and size), 0
x9411 (scattered evenly: probably `rock`), 2 x1083 (probably `bush`), 4 x3
(probably `frn_spec`). Only the trees are sure. `tree_s` is a flat soft
shadow, probably drawn under trees. The scenery textures' alpha layers are
their outlines (transparency), not light maps.

## Tiles and ground models

Checked on the 7 maps of `icewrld.cat`:

- A tile's type-20 header's third value is the number of its ground model
  among the `.geo` files of the map's archive, in archive order (counting from
  0, `lod/` models included). Shifting the numbering by one in either
  direction halves how many edges meet, so this is right.
- **Rotation:** bits 14-15 of the fourth value turn the model anticlockwise
  by that many quarter turns about the tile's centre. Bits 16-17 take three
  values per model family (0-3, 4-7 or 8-11 for the whole code); they aren't
  a mirror or a height offset (tested). Each model appears in groups of 4
  records per rotation (`f1`: most models 16 times), which differ in their
  terrain codes. A few `ramp` tiles with bits 16-17 = 1 would fit better
  turned a further half turn; not enough to be sure.
- **Layout:** a block's four tiles go round the square in record order, at
  (0, 0), (1, 0), (1, 1), (0, 1) tiles from its corner. With rotation, 88.5 %
  of neighbouring tile edges in `f1` meet exactly (heights at 5 points within
  80 units); the next best layout scores 72 % and no rotation 46 %.
- Ground models span -4096..4096 in x and y (a tile is 8192 units, a block
  16384). Heights come in levels built into the models: 0, 3264 and 7360
  (`ice_08` is a plateau at 3264, `ice_09` at 7360; cliffs and ramps join
  them). Most remaining mismatches are flat `ice_19` tiles at 0 beside
  `ice_09` plateaus (1687 of 2900 in `f1`), even inside one block, and
  nothing tested (cell values, block bytes, tile fields, other rotations or
  mirroring) lifts them. The terrain codes (below) agree they are low ground:
  they are real pits in the plateau, so the game must draw walls there.
- **Bit 24** of the fourth value is set only on `hole_02` tiles (in every
  `icewrld` map). `hole_02` is a bowl whose texture (`hole02`) is transparent
  round the hole, so ground must show through there: presumably the bit
  says so. Drawn that way, with the lowland ice texture underneath, the
  tiles look like the frozen-water pits in screenshots of `f1`: blue ice at
  the bottom of smooth, steep sides.
- In each type-21 record, the first four items count 0-3 starting at
  (rotation + 2) mod 4: the tile's sides, numbered in its own frame.

## Terrain codes

A type-20 record's five 8 x 8 grids are **not** textures, as first thought:
`hole_03`, `ramp` and `concrete` tiles, which look nothing alike, carry the
same grids, and `ice_09`'s plateau has the same codes in all five layers.
Each cell holds a code from 1 to 18, or 0 for none. Codes 1, 2, 4, 7, 10,
13, 15 and 18 appear only in a grid's first row and column, the others only
inside it, and they come in pairs (border 15 with inner 9, 2 with 11, 13
with 8, 4 with 12, 18 with 6, 1 with 17, 10 with 5).

The codes index the colour table at the end of the file (the same 18 colours
in every `icewrld` map; entry 0 is magenta). Coloured that way, plateaus are
light blue and lowland dark blue, and the first row and column draw a grid:
a minimap or terrain-type map. Cells are in map orientation (row 0 at the
south, column 0 at the west, after the tile's rotation). Higher layers cover
parts of a tile in staircase shapes; the 8 bytes after the grids are bit
masks of rows (`ff ff fc f0 f0 e0 c0 c0`) matching those shapes. What the
five layers are for (unit types? heights?) isn't known.

### Textures

Neither the map nor the ground models name their textures. Each face numbers
a texture among its model's own (0-6, as for other models; see
[geo.md](geo.md#textures)), with UVs across the whole tile (`hole_01`'s
crater spans 0-1). The planet's textures fit the models' names only in part:
`hole01`-`hole03`, `crack01`, `ramp01`, `concrete`, and `ice01`-`ice08` for
the 26 `ice_` models, several of which number more than one texture. Matching
textures to faces by colour across seams gave no consistent answer. The
assignment is probably held elsewhere (another archive or the game itself).
`cliff07`, which no model's name matches, may be the texture for the walls
between levels.

A ground model's own 8 x 8 grid (after its faces) lists, per cell, the faces
over it; cells overlap, so it's a lookup for heights or collisions, not
textures.

## Open questions

- Which textures the ground models' faces use.
- What the five layers of terrain codes are for.
- How the game builds and textures the walls between levels.
- Bits 16-17 of the tile flags; the rest of the type-21 records; the grid
  entries' second value's low 12 bits; the placed objects' bits
  8-15 and the missing quarter.
- Which deposit kind is which resource.

## Not in the map

Every part of the file is now accounted for: the tile, link and block
records, the placed objects, the grid (blocks and deposits) and the colour
table. The only parts not understood are per tile model (the type-21 links)
or per cell (the low bits above), so units, buildings and starting colonies
can't be stored here: they presumably come from the missions, which
`level.dat` names by a "parser short name" (`F1`, …) but whose scripts
aren't in any archive (probably inside `STNW.exe`). The concrete tiles in
`f1` are a few scattered groups of 1-3 tiles, not a colony's pad.
