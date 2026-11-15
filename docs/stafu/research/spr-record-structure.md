# SPR record structure: open investigation

**Status:** decoder not identified; record boundaries and bit alignments remain hypotheses.

- M278–M325 compared `PICARD.SPR` record2 shared regions, shifted correspondences, unmatched bit intervals, alternative boundaries, cross-record candidates, independent alignments, and structural transitions.
- Candidate tags encountered in resource work include `TRPS`, `TSIL`, `PMOC`, `MOCS`, `EMIT`. Their semantics must not be assigned from reversed spelling alone.
- M319–M325 were reported passing as individual diagnostic milestones; their detailed numeric evidence must be reattached before promoting any proposed field layout to **confirmed**.
- M326 and subsequent work moved toward resource object access and executable disassembly; no proven SPR decompressor emerged.
- M440–M447 examined a buffered-stream/graphical-state path, which is **not established** as the SPR decoder. See [stream-and-file-io.md](stream-and-file-io.md) and [animation-and-object-state.md](animation-and-object-state.md).

**Next experiment (proposed M448):** test record-boundary interpretations across multiple user-owned SPR files; state explicit rejection criteria and compare predictions to observed data. Avoid assuming a decoder from generic I/O functions.

**Evidence retention gap:** detailed M278–M325 TXT output is not yet checked into this research archive. Recover and sanitize before making stronger claims.
