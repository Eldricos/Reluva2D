# Stream and file I/O

**Input:** reference `STTNG.OVL` SHA256 `65b54cd24a5588ed5056685cf5687b8159c169e26c6835910546e97f047f0697`.

## Static findings
- DOS wrappers: 0x11A9F0 open (INT 21h AH=3Dh), 0x11ACD0 read (AH=3Fh), 0x11AE03 seek (AH=42h).
- Stream helpers: 0x7E410 acquisition, 0x7E5A4 cleanup, 0x7E638 position manipulation, 0x7E684 stream read, 0x7D560 resource buffer load, 0x7E034 lookup, 0x7D7AC dispatcher.
- M440: 23 raw E8 callers to DOS read wrapper, including 0x7E7B3, 0x7D61E and 0x7D7F3. Counts are **raw call-pattern matches**, not all validated runtime callers.
- M441: 0x7E684 handles memory-backed copying and file-backed seek/read paths; raw inbound candidates at 0x89794, 0x899A9, 0x89A7E.
- M442: 0x89924 performs buffered transfers; observed globals 0x9EE68 buffer pointer, 0x9EE6C remaining, 0x9EE70 position/state, 0x12F78 stream index, 0x12F7C stream state.
- M443–M446: three direct callers of 0x89924 at 0x7BF23, 0x7BFF5, 0x7C016; requests 0x64 (100) and 0xC8 (200) bytes. One 200-byte call is in a loop limited to 10 iterations; another is followed by an object-position comparison.
- M447: 0x89801 initializes stream/buffer state via 0x7E410 and allocator 0x8F790; its raw caller at 0x7BF08 passes 0x9F4. The control path uses graphical-object helper 0x8C536, getters 0x7C34C and 0x7C36C.

**Interpretation:** generic buffered I/O feeding higher-level graphical/event control. **Not established** as an SPR decoder.

## Reproduction and outputs
Scripts: `scripts/stafu-m440-*.py` through `scripts/stafu-m447-*.py` (see [index](experiment-index.md)). Reference output filenames follow `mNNN-<diagnostic-name>.txt`; the complete user-run TXT files are **not yet committed**.

## Open
Find the SPR-specific resource-to-decoder call chain, not merely generic I/O wrappers.
