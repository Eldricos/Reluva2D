# Star Trek: New Worlds: `.stb` string tables

Status: **understood** for the start of `rom01.stb`; `reluva-tool stb-dump`
checks the whole layout of any file. Reader: `src/engines/newworlds/string_table.cpp`.
Stored in `.cat` archives with chunk tag `0x32`, under `str/eng/...`.

All values little-endian.

```
u32 count
repeated count times:
  u32 id
  UTF-16LE text, terminated by a 0x0000 code unit
```

Example (`str/eng/miss/rom01.stb`, 3168 bytes, count 23):

| id | text |
|---|---|
| 1 | ` 'Et Tu?'` (the mission title; `level.dat` lists R1 as "Et Tu") |
| 23 | `Stardate 8988.4.  The type 6 world Sakuntula has significant deposits of dilithium.` |
| 22 | `~STELLAR CARTOGRAPHY~Tabula Rasa Systems…` |

IDs are not in order. `~` appears to mark headings.

## Open questions

- Whether `count` always equals the number of strings (the parser checks
  that the strings end exactly at the end of the file).
- The meaning of `~` and any other markup.
