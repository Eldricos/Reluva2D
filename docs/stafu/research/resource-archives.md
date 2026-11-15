# Resource archives and installed-resource detection

**Evidence level:** confirmed static observations from STAFU archive/inventory diagnostics; no assertion about original installer implementation.

- Installed-game candidate detection: `STTNG.EXE`, `STTNG.OVL`, `ASTRO.DB`, `COMPUTER.DB`.
- CD candidate detection: `STTNG.ZIP`, `STTNG.DIR`, `STTNGINS.ZIP`, `TITLE.FVF`.
- `STTNG.ZIP` and `STTNG.DIR` each enumerate **14,647** entries. Their central directory bytes were reported identical (size **848,552**; declared offset **475,613,967**), despite standard ZIP versus sidecar layout.
- Compression methods observed: method 0 for **134** entries, method 8 for **14,513** entries.
- Representative decompression/CRC checks: `COMPUTER.DB` method 0, 437,414 bytes, CRC32 `490fc598`; `ASTRO.DB` method 8, 78,582 compressed to 314,871 bytes, CRC32 `4dee6013`; `PICARD.SPR` method 8, 563,683 compressed to 772,639 bytes, CRC32 `aabe43b4`.
- Resource loader was verified against installed files and CD fallback, including `ASTRO.DB` and `PICARD.SPR`.
- Installed-versus-archive inventory used uncompressed size + CRC32 as a matching criterion. **Matching does not prove installer behavior.** `AUDIO.LOG` and `CREATE.LOG` were unmatched in the observed installed inventory.

**Open:** original installer copying behavior and full file-to-archive mapping should not be inferred from CRC matches alone.

**Provenance:** early `archive-probe`, `archive-test`, `archive-list`, `archive-compare`, `resource-test` and resource inventory diagnostics; historical output files not yet imported.
