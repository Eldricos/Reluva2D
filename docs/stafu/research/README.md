# STAFU reverse-engineering research archive

This is the durable research index for the clean-room STAFU engine. It records observations of original *Star Trek: The Next Generation – A Final Unity* resources without copying original game binaries or executable code into the repository.

## How to use this archive
- Read [experiment-index.md](experiment-index.md) to locate a diagnostic by milestone.
- Read the subsystem pages for interpreted findings and unresolved questions.
- Use the linked `scripts/stafu-mNNN-*.py` diagnostics to reproduce an experiment **only if necessary**.
- Keep original game files under ignored local `games/` directories. Do not commit them.
- Static byte matches, candidate control-flow reconstruction, and runtime execution are different evidence levels.

## Evidence status
- **Confirmed static:** directly observed in an input file or verified disassembly; not necessarily observed at runtime.
- **Inferred:** plausible interpretation supported by multiple observations, awaiting validation.
- **Unresolved:** competing hypotheses, ambiguous layout, or missing proof.
- **Rejected/not established:** evidence does not justify the claimed interpretation.

## Source preservation policy
1. Every experiment gets an index entry: milestone, script, input identity, findings, confidence, and output filename.
2. Preserve the **complete text diagnostic output** under `docs/stafu/research/results/` when available and suitable for redistribution; otherwise record its location and a concise evidence excerpt. **Do not copy proprietary game data or long raw executable dumps into results.**
3. Preserve original user-run output files locally until a safe sanitized version has been committed. Conversation uploads alone are not a permanent Git repository archive.
4. Never treat an uncommitted output as archived. Mark missing outputs explicitly.
5. When new evidence changes a conclusion, append a correction and link both experiments; do not silently overwrite history.

## Pages
- [Resource archives](resource-archives.md)
- [SPR record structure](spr-record-structure.md)
- [Palette and rendering](palette-and-rendering.md)
- [Executable layout](executable-layout.md)
- [Resource management](resource-management.md)
- [Stream and file I/O](stream-and-file-io.md)
- [Animation and object state](animation-and-object-state.md)
- [Experiment index](experiment-index.md)

## Current limits
The diagnostic scripts are in GitHub, and research conclusions are summarized here. **Complete original TXT outputs have not yet been systematically imported into the repository.** The first archival pass is a curated technical summary, not a claim that all historical outputs are already preserved.
