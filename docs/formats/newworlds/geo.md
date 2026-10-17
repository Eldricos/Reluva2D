# Star Trek: New Worlds: `.geo` models

Status: **core geometry understood** (faces, texture coordinates, vertices,
normals, edges); several blocks are still skipped. Worked out from complete
byte dumps of five small models, October 2026:

| Model | Bytes | What it is |
|---|---|---|
| `geo/rom/rb_mid.geo` | 980 | beam section: a two-sided rectangle |
| `geo/fed/fs_s.geo` | 1268 | flat outline: a hexagon on one side, 4 triangles on the other |
| `geo/kln/kwb_s.geo` | 1440 | two separate flat pieces |
| `geo/map/aln/concrete.geo` | 1452 | terrain tile: flat 8192 × 8192 square |
| `geo/map/aln/lod/alien_14.geo` | 1476 | terrain tile: square sloping up 3264 units |

Reader: `src/engines/newworlds/geo.cpp`. Check every model with
`reluva-tool geo-survey *.cat`; export one to OBJ with
`reluva-tool geo-info <file> <out.obj>`. Stored in `.cat` archives with chunk
tag `0x17`; the size in the archive's copied metadata never matches, so these
were presumably converted when packed.

All values little-endian; floats are IEEE 32-bit.

```
faces          face count × face; face 0's header is the file header:
                 u32 ?        file header: 952 / 416 / 1072 / 328 (fs_s, rb_mid, kwb_s, terrain)
                              other faces: the previous face's part
                 u32 ?        file header: the face count; other faces: up to
                              49 seen (perhaps a material index)
                 u32 corners
                 u32 part     which part the face belongs to (see Parts)
                 corners × 32 bytes: u32 vertex, f32 u, f32 v (both × 65536),
                 5 × u32 extra: extra[0] is the corner's normal, an index into
                 the part's second normal list (smooth shading); in the last
                 corner, extra[4] picks the texture
bounds         u32 last face's part, u32 ?, u32 part count,
               u32 attachment count (2 in rb_mid, 4 in fcarb, 6 in rms_d)
               f32 centre x, f32 centre y, f32 radius
               u32 1, 0, u32 ? (1 in rb_mid, else 0)
...            attachment data, layout unknown (116 bytes in rb_mid). Its
               length can be odd (fsb_b), so what follows is not 4-byte aligned
part data      u32 size (reaches the end of the file in mn_s), u32 1,
               u32 part count, then per part, in part order:
  (parts > 0)    u32 0        end of the previous part
  vertices       u32 ? (0; 1 in ptank_l's part 1), u32 count,
                 count × (f32 x, y, z)
  normal lists   repeated while present: u32 1, u32 count, count × unit vector
  edges          u32 0 = none (terrain tiles), or u32 1, u32 group count, then
                 per group: u32 count, count × (u32 part, v0, v1, faceA, faceB)
...            after the last part (mn_s): u32 2, then records of a position
               (a part 0 vertex), a vector (0, 0, −4096), u32 0, 3, 1, and an
               index (0, 4, 6); then u32 3, 2, 8, 9, 0, 0, 2 before the names
...            terrain tiles: u32 1, u32 64, then per cell of an 8 × 8 grid a
               count and that many face indices; then u32 8, 8, ?, bounds, and
               the faces again (in hole_02, with the same texture coordinates:
               not a second texture layer)
trailer        names ("main", "terminate", "muzzle_flash", "front_lock",
               "pickup"),
               layout not studied
```

## Parts

Some models are built from separate pieces, each with its own part number
(0, 1, …) in its faces' headers. `mn_s` has 2 parts, `fcarb` 3, `rms_d` 9.
Faces are grouped by part, though not in part order: `rms_d` goes 0, 6, 7, 5,
8, 4, 3, 2, 1. Each part numbers its vertices from 0: part 1 of `mn_s` uses
vertices 0–5, and part 0 uses vertices 0–10. In the corners of a part above 0,
the last u32 of the 20 trailing bytes holds the part number, except in the
face's last corner. Each part has its own vertices, normal lists and edges, stored one part
after another; each edge record starts with its part's number, and its faces
(numbered across the whole model) belong to that part. Part vertices are in
model coordinates: the two parts of `mn_s` (a flat outline: body and wings,
then the tail) meet without any offset.

## Textures

No file names a model's textures; they are found by name. `k_bop`
(`kliactor.cat`) goes with `lbp/txr/kln/k_bop_a`, `k_bop_b` and `k_bop_g`.
Each face picks one with the **last value of its last corner**
(`extra[4]` in the reader): in `k_bop`, 0 is the central body (`k_bop_a`),
1 both wings (`k_bop_b`), and 2 three spots on the body (`k_bop_g`, the
small 128 × 128 one). So the textures are numbered in name order.

`reluva-tool geo-textures <file.cat>...` applies the rule below to every model
and lists the ones it finds no textures for (`-a` lists every model's).
Known misses: `landpd_d`, which looks like `landpad` but doesn't share its
name; textures also look misplaced on some models, such as `fpdt` and
`fhf_p`.

Buildings also use **shared textures** with names of their own:
`fedbuild.cat` has `pad`, `door2`, `door3`, `pdoor`, `f_in2`..`f_in4`,
`deckanim` and `webspark`, and `fss_a` numbers 6 textures while only `fss`
and `fss_g` are named after it. No file lists which shared textures a model
uses: their names appear in no other file (the `.mdl` files list docking
points and states, `constructed` and `deconstructed`, but no textures), and
the archive's order doesn't follow the models. So those numbers stay unknown
for now and are drawn untextured.

The viewer's rule, still to be checked on more models: a model uses the
textures named exactly after it or after it plus `_` and a suffix, in name
order, preferring its own folder (`kln`). A model without any, such as a `_l`
version, uses those of its name without the suffix, and a shortened name such
as `landpd_d` those of the texture it abbreviates (`landpad`). The plain
textures take the numbers from 0, and the `_g` light map the last one (as in
`k_bop`); `_s` textures belong to the `_s` models. A model without a match
at all, such as the `_s` outlines, is drawn untextured.

In the other corners of a face, the same value holds the part number (0 in
part 0), so it isn't a per-corner texture choice.

Variants share textures: the `fedbuild.cat` models `frps_a`, `frps_b`,
`frps_c` and their damaged versions `frps_ad`, `frps_bd`, `frps_cd` all use
`lbp/txr/fed/frps` and `frps_g`.

**`_g` textures** (`k_bop_g`, `frps_g`, `kcs_g`, …) have an alpha layer.
Treated as transparency, it makes parts of hulls see-through, and more of
the damaged versions, so it is probably something else, such as a glow or
lighting map. The viewer's G key switches between ignoring it, glow and
transparency. Not settled.

**Ground tiles** (`concrete`, `alien14`, …) find no textures this way. The
planet archives hold terrain textures such as `lbp/txr/map/alp/green01` and
`cliff01` (in `alpwrld.cat`, next to the `.gmf` maps), so the maps
presumably assign them.

## Winding and file names

**Front faces are clockwise.** Drawn with x, y, z as a right-handed, z-up
space, a face's corners run clockwise when seen from outside the model.
Hiding faces that appear anticlockwise on screen leaves solid models such as
`k_bop` whole, while hiding the clockwise ones empties them. That's the Direct3D
convention. Whether the game's space is mirrored relative to this one (which
would flip both the picture and the winding) isn't confirmed yet.

**Name suffixes** (from looking at the models in the viewer):

| Suffix | Meaning |
|---|---|
| `_l` | lower-detail version (e.g. `phtart_l`, `ptank_l`, `mb_l`) |
| `_s` | flat outline at z ≈ 0, presumably the shadow (`fs_s`, `kwb_s`, `mn_s`) |
| `_p` | a separate piece of the main model: `fhf_p` looks like the inside of the `fhf` hangar's bay, seen when its doors open |
| `_d` | probably *deconstructed*: buildings' `.mdl` files list the states `constructed` and `deconstructed` |
| `_a`, `_b`, `_c`, `_r` | variants with small changes; `_ad`, `_bd` are their `_d` versions |

## Evidence

- **Face 0 is in the file header.** In `concrete` the header's `2, 3` are 2
  faces and 3 corners, and the first 3 records form a triangle (1, 3, 0) with
  the second triangle (2, 0, 3) after it. Read that way, `rb_mid` is a
  rectangle (3,1,0,2) and its back (3,2,0,1), and `fs_s` is a hexagon and the
  four triangles of its other side.
- **Corners** carry their own texture coordinates; in `rb_mid` they form a
  consistent planar mapping (u along y, v along x) only if the first field is
  the vertex index.
- **Edges** name two vertices and the two faces either side, 0-based. Every
  edge in all five models passes the reader's check that both vertices lie on
  both faces: e.g. all four edges of `rb_mid` lie between face 0 and face 1
  (front and back), and `fs_s` has the 9 edges between its hexagon and its 4
  triangles.
- **Edge groups**: `kwb_s` has two groups of 5 edges, one per piece
  (vertices 0–3 with faces 0–2, and 4–7 with faces 3–5). Only with two groups
  does it show that each record starts with the 0 rather than ending with it.
- **Unaligned blocks**: in `fsb_b` (7 faces) the attachment data contains
  the bytes `1c 00 0c 00` and ends at an odd offset; the vertex block's
  `1, 1, 0` sits at 0x416 and its count of 8 at 0x422. The reader therefore
  searches for the vertex block byte by byte.
- **Radius** is the farthest vertex from the centre: 488.2 in `fs_s` is the
  distance to (−88.6, −480.1).

## Open questions

- What parts are for (moving pieces? damage states?), and the data after
  the last part.
- The file header's first value, face header values, and the 20 bytes after
  each corner's texture coordinates (sometimes a 1 in the last word).
- Attachment data; the terrain grid's exact layout; the second texture set in
  terrain tiles; the trailer.
- The first normal list: one entry per distinct face direction, probably
  face normals (`hole_01`: 13 for 16 faces), but no face field is known to
  index it.
- Larger 3D unit models haven't been checked yet.
