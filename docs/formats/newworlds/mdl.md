# Star Trek: New Worlds: `.mdl` model descriptions

Status: **first look**. Stored in `misc.cat` with chunk tag `0x2c` (522 files,
e.g. `mdl/kln/k_bop.mdl`). Worked out from the start of `k_bop.mdl` (the
Klingon Bird of Prey) and the strings in it, October 2026.

All values little-endian.

## Start of the file

```
0x00   char[32]   title, e.g. "klnbirdofpreymetamodel"
0x20   u32        12 in k_bop (a count?)
0x24   entries of char[32] name, u32 value, ending with "terminate", -1
```

In `k_bop` the values are part numbers of `geo/kln/k_bop.geo` (3 parts):

| Name | Value |
|---|---|
| `main` | 0 |
| `turret_1`, `gun_1` | 1, 2 |
| `turret_2`, `gun_2` | 3, 4 |
| `turret_3`, `gun_3` | 5, 6 |
| `muzzle_flash_1`, `_2`, `_3` | 2, 4, 6 (each on its gun) |
| `terminate` | −1 |

`k_bop.geo` has only 3 parts but the names go up to 6, so the numbers may
count nodes rather than `.geo` parts. Not resolved yet.

After the table: u32 0x818, then 32-bit values such as `0x009a0b0c`,
`0x009a0150`, `0x00990328`, …, mostly 64 apart. They look like memory
addresses saved by the tool that wrote the file, so they are probably
meaningless.

## Other strings in `k_bop.mdl`

Further name lists, each ending with `terminate`:

- `turret_axis_1..3`, `gun_elevate_1..3`, `gun_muzzle_1..3`: probably the
  points the turrets turn about and the guns elevate about, and where shots
  leave.
- `muzzle_flash_1..3`.
- `normal`, `dofpreyva` (likely the end of a longer name), `low detail`:
  probably the model's detail levels or states.

Floats follow, likely positions for the points above.

## Buildings

`fss_a.mdl` (title "fedsciencestatmetamodel") has the same layout with other
names: docking points `BAY_0`..`BAY_5`, each with a `BAY_n_LOCK`;
`VEHICLE_DOCK` with `_CAPTURE`, `_LOCK`, `_ARRIVAL`, `_RELEASE` and
`_DEPARTURE` points; a `PORTAL`; and the states `constructed` and
`deconstructed` (so a `_d` model is probably the deconstructed version). The
same point names appear in the trailer of `fss_a.geo`.

## Open questions

- No texture name appears in `k_bop.mdl`; textures are found by the
  model's name (see [geo.md](geo.md#textures)).
- The layout after the first table.
