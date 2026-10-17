# Star Trek: New Worlds: install layout

Source: directory listings of a clean install of the English Windows retail
release, and of the same install with an unofficial fan patch applied
(compared file by file, October 2026).

## Identification

| File | Size | MD5 of first 5000 bytes |
|---|---|---|
| `STNW.exe` | 7,143,424 | `e7bfa98b3f045471789d84089f4b8eca` |
| `glblgame.cat` | 6,883,984 | `9b5c322aef388f8e58e72b81bd9a91ad` |
| `rommisc.cat` | 309,162 | `ed1ff0cd696da786cf19656551647c43` |

`STNW.exe` and `glblgame.cat` are identical in the clean and fan-patched installs.

## Game data

- `*.cat`: archives (format still to be documented). Names follow a pattern:
  - per faction: `fed` (Federation), `kli` (Klingon), `rom` (Romulan), `for`
    (presumably neutral/foreign), each with `actor`, `build`, `gui`, `misc`
  - per planet type: `alnwrld`, `alpwrld`, `deswrld`, `icewrld`, `redwrld`, `vlcwrld`
  - shared: `glblgame`, `glblgui`, `misc`
- `game.ini`, `level.dat`, `olevel.dat`, `kapla.rse`: small settings and data files.
- `Media/`: standard MP3 and WAV files (voice per faction in `FedVO`,
  `KlingVO`, `RomVO`; `Music`; `Misc` sound effects and tutorial voice).
- `SFC2.BIK` plus the movies on the CD: Bink video. A clean install doesn't
  copy the `MOVIES` folder to the hard disk.

## Not needed by Reluva

Miles Sound System (`Mss32.dll`, `*.flt`, `*.m3d`, `*.asi`), electronic
registration (`EReg*`, `Reg*`, `ERHttp*`), the defunct Mplayer online service
(`mplaynow*`, `MPLAYNOW/`, `NewWrlds.mpi`), `kapromo.exe` (promo trailer),
`DX7Aeng.exe` (DirectX 7 installer), and `BINKPLAY.EXE`.

## Fan patch differences

Changed: `fedactor.cat`, `foractor.cat`, `kliactor.cat`, `romactor.cat`,
`rombuild.cat`, `alpwrld.cat`, `game.ini`, `level.dat`, `olevel.dat`,
`kapla.rse`, `Readme.txt`, `BINKPLAY.EXE`, `Media/StarTrek.wav`.
Added: `vars.ini`, `screen.png`. The executable is unchanged.
