# STAFU Engine Project Plan

**Project:** Clean-room reimplementation of *Star Trek: The Next Generation – A Final Unity* (STAFU) in Reluva2D  
**Repository:** https://github.com/Eldricos/Reluva2D  
**Working branch:** `feature/stafu-engine`  
**Status:** Planning decisions recorded; implementation proceeds incrementally.

## 1. Goal and scope

Build a complete, playable replacement engine for STAFU within the existing Reluva2D C++ project, following the broad compatibility-layer spirit of projects such as ScummVM. The first target is Windows; other platforms may follow later.

Users must supply their own original game data. Reluva2D will provide the engine and a reliable way to import and validate a user's legally obtained game files. The project should preserve and continue to support Reluva2D's existing functionality, including the Testbed and New Worlds viewer.

## 2. Development principles

- **Clean-room implementation:** Implement file-format handling and game behavior independently. Do not copy implementation code from other reverse-engineering projects.
- **Document research:** Keep notes on observed file structures, test results, hypotheses, and the provenance of information. Distinguish verified facts from assumptions.
- **Preserve existing behavior:** Make small changes, build and test frequently, and avoid regressions in existing engines and tools.
- **Follow the actual project architecture:** Use the existing `Engine`, `MetaEngine`, detection, filesystem, and CMake patterns where appropriate. Inspect current interfaces before designing new ones.
- **Avoid premature assumptions:** Do not assume STAFU uses New Worlds formats or that every CD file is needed at runtime. Establish facts from the user's original data and repeatable tests.
- **Keep the work incremental:** Get source detection and inventory working before building the importer; get importing reliable before integrating it into the launcher; investigate resource formats one at a time.
- **Keep originals untouched:** The import process copies source files into Reluva2D-managed storage and does not move, modify, or delete the user's original files.

## 3. Current environment and known facts

- The user has Visual Studio 2022 and has cloned, built, and run Reluva2D successfully.
- The working branch is named `feature/stafu-engine`.
- STAFU code has been added to the local project and compiles/runs after resolving an include-path issue. The local branch may contain changes that are not yet on GitHub.
- The original game was installed using DOSBox because its old installer does not run natively on modern Windows.
- The installed directory is named `STTNG`, which is a generic name and is not sufficient by itself to identify the game.
- The installed directory is approximately **12.5 MB**.
- The original CD contents are approximately **628 MB**.
- The installed files and CD contents retain their original directory structures and have not been modified.
- The CD contents are needed to run the original game, but the exact runtime dependencies and roles of individual files have not yet been established.

## 4. Managed game-data layout

Use the following as the intended Reluva2D-managed layout:

```text
GameData/
└── stafu/
    ├── cd/                 # Imported CD contents, original structure preserved
    └── ...                 # Imported installed-game files and verified subdirectories
```

`GameData/` is illustrative; use the project's actual managed-data root if one already exists. `stafu/` is the engine-specific directory. Keep CD content under `stafu/cd/`, separate from the installed-game files. Preserve directory structure and do not merge the two source trees. Do not assume every source file must ultimately be copied until investigation establishes the requirements.

## 5. Planned user-facing installation flow

The intended long-term experience is an **INSTALL GAME** action in Reluva2D, rather than requiring users to manually run DOSBox and arrange folders.

Potential supported sources:
1. Original physical CD.
2. An ISO or other backup image of a disc the user owns.
3. An existing DOSBox-installed game directory, with the CD contents supplied separately if needed.

The flow should identify and validate the source, inventory it, copy the appropriate files into the managed `stafu/` layout, preserve source structure, verify the results, and report what succeeded or failed. Where practical, the importer should read/mount or extract from a disc image rather than requiring the user to manually extract it. Technical details will be decided after source detection is understood.

A shared installer/import framework may eventually serve other games. First implement the STAFU-specific path and generalize only after this use case works reliably.

## 6. Agreed implementation sequence

### Phase 1 — Source detection and inventory

Build a STAFU-specific source detector that can identify likely:
- original CD contents or a directory copied from the CD;
- extracted ISO contents;
- an existing installed-game directory;
- combinations of installed files and CD contents.

Detection must rely on verifiable file evidence and not just a directory name such as `STTNG`.

Inventory source files before copying. Record at least:
- source category (installed game versus CD);
- normalized relative path;
- file size;
- a content hash suitable for identifying files and verifying copies, using existing project facilities where appropriate.

Keep the installed-game and CD inventories distinct. Decide on hash algorithm and whether to reuse or extend existing project helpers after inspecting the current code. Do not add a dependency or invent an interface without checking the repository first.

**Completion criteria:** A selected source can be scanned repeatably, the inventory makes source files and their relative paths clear, and detection results can explain which evidence was found and what remains uncertain.

### Phase 2 — Import service

Implement a standalone import service that copies validated files into the managed layout.

Requirements:
- preserve original relative paths within each source tree;
- keep installed files and CD files separate;
- verify copy results, including size and content hash where available;
- report inaccessible, missing, or failed files clearly;
- allow a failed or interrupted import to be retried safely;
- avoid silently overwriting unrelated files or an existing import;
- never modify or delete original source files.

Test this service independently before wiring it into the launcher.

**Completion criteria:** A test import produces the expected managed layout, verification succeeds, errors are actionable, and retry behavior is safe.

### Phase 3 — Launcher integration

Once detection and import work independently, add an **INSTALL GAME** action to the Reluva2D launcher. Connect the UI to the detector/import service, present understandable source choices and results, and make the game ready for detection/launch when required data is verified.

Do not make launcher UI the first implementation step; keep the underlying logic testable without it.

### Phase 4 — Resource and behavior investigation

After source detection/import is reliable, investigate the original game files one family at a time:
- record file signatures, sizes, structure, and relationships;
- distinguish verified observations from hypotheses;
- identify which files the game needs and when;
- develop independent readers and tests for confirmed formats;
- avoid assuming compatibility with New Worlds formats without evidence.

Use the original game and repeatable observations to validate behavior. Other projects may help identify names of file families or questions to investigate, but their implementation code must not be copied.

### Phase 5 — Setup/configuration requirements

Determine whether the original installer created configuration or setup files, set options, or performed transformations required by the game. Reproduce only the behavior that investigation demonstrates is necessary. Do not emulate the original installer speculatively.

### Phase 6 — Ongoing testing and platform work

- Build and test incrementally in Visual Studio 2022.
- Retain regression checks for the existing Testbed and New Worlds functionality.
- Test detection, inventory, import verification, failure handling, and retry behavior.
- Focus first on a complete playable Windows implementation; consider other platforms after the core is reliable.

## 7. Immediate next task

Inspect the current local versions of Reluva2D's common filesystem, detection, and hashing helpers, along with the existing engine registration and CMake patterns. Then implement **Phase 1: STAFU source detection and inventory** with the smallest set of changes that fits the project architecture.

Do not start by decoding game resources or building the full installer UI. First establish a trustworthy inventory of the actual installed-game and CD files.

## 8. Decisions not yet made

The following remain open until supported by inspection or tests:
- exact file signatures and rules that positively identify STAFU sources;
- the final hash algorithm and whether existing hash helpers are sufficient;
- the exact set of CD and installed files needed for a playable game;
- how physical discs and ISO images will be accessed on each platform;
- whether original installer-generated configuration or transformations are required;
- the exact launcher UI and the best point to generalize a shared import framework.

Do not treat these open questions as settled facts.

## 9. Repository and licensing notes

Reluva2D is licensed under GPL-3.0-or-later and has clean-room contribution requirements. Follow the repository's current `CONTRIBUTING.md` and licensing terms when implementing and distributing the engine. Clean-room methods are important for project discipline but do not, by themselves, eliminate legal risk.

---

**Working rule:** Verified evidence before assumptions; inventory before import; import before launcher integration; reliable file handling before format decoding; incremental changes before broad refactoring.
