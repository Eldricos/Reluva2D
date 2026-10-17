# Reluva

**R**eimplemented **E**ngines for **L**egacy, **U**nsupported & **V**intage **A**dventures.

A framework for running classic games natively on modern systems by
reimplementing their engines, in the spirit of ScummVM. Games are not
included: you point Reluva at your own installed copy.

## Layout

```
src/common/        platform-independent core: System interface, events,
                   surfaces, config, case-insensitive file lookup, detection
src/backends/sdl3/ the only code that touches SDL (window, rendering, input, audio)
src/launcher/      Dear ImGui game list: add / remove / start games
src/engines/       one directory per engine, plus registry.cpp listing them
  testbed/         built-in test "game" that exercises the backend
  newworlds/       Star Trek: New Worlds: archive, string table, texture and
                   model readers; currently runs as a model and texture viewer
src/tools/         reluva-tool, utilities for studying game data
tests/             unit tests for common/
```

Engines only use `common/` and `engines/engine.h`. They never include SDL.

## Building

Needs CMake 3.24+, Git and a C++20 compiler. SDL3 and Dear ImGui are
downloaded and built automatically.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/reluva
```

See **[docs/BUILDING.md](docs/BUILDING.md)** for step-by-step instructions for
Linux (with each distro's packages) and Windows with Visual Studio 2022.

## Usage

- `reluva` opens the launcher. *Add Game...* scans a folder for supported games.
- `reluva <target>` starts a configured game directly. `reluva testbed`
  runs the test pattern.
- `reluva --hash <files...>` prints detection signatures for an engine's table.
- `reluva-tool` has developer tools for studying game data: `hexdump`,
  `cat-probe`, `cat-list` and `cat-extract` (New Worlds archives).
- In game: **Ctrl+F5** returns to the launcher, **Alt+Enter** toggles fullscreen.
- New Worlds currently opens as a data viewer. **Tab** switches between models,
  textures and maps, **Left/Right** step through them and **F** cycles filters. For
  models, drag with the mouse (or **W/A/S/D**) to rotate, use the wheel (or
  **+/-**) to zoom, **Space** to spin, **P** to show one part at a time, **L**
  for outlines, **U** for texture-coordinate checks, **T** to turn textures off, **G** to change how alpha layers are used, **H** for smooth or flat shading
  (glow, ignored, transparency), **C** to
  colour faces by texture number, **B** to show back faces
  and **R** to reset the view. Maps are built from their ground tiles: drag to
  rotate, use the wheel to zoom, **W/A/S/D** to move, **Q/E** to turn,
  **Left/Right** for the next map, **T** for textures, **G** to colour the ground by its terrain codes or show resource deposits, **O** for the placed trees and rocks, **H** for smooth or flat shading, **K** for
  the walls between levels and **L** for outlines.

The config is saved as `reluva.ini` in SDL's preferences folder
(`~/.local/share/reluva/reluva/` on Linux, `%APPDATA%\reluva\reluva\` on Windows).

## Adding an engine

1. Create `src/engines/<name>/` with an `Engine` and a `MetaEngine` (copy `testbed`).
2. Add a detection table built with `reluva --hash`.
3. Register its accessor in `src/engines/registry.cpp` and add the source to
   `CMakeLists.txt`.

Read [CONTRIBUTING.md](CONTRIBUTING.md) first: only clean-room work is accepted.

## License

Reluva is free software, licensed under the GNU General Public License,
version 3 or (at your option) any later version. See [LICENSE](LICENSE).

Reluva doesn't include any game data. Games and their assets remain the
property of their respective owners.
