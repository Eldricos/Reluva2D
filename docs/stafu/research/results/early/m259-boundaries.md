# m259-boundaries.txt — curated observations

**Source:** `ResearchArchive.zip` / `m259-boundaries.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `1f1a4fd4383737b34367d27b33837841e0d9b76ad13d1d36700d54deb56276fc`; **encoding:** `utf-16-le`; **original lines:** 14; **selected lines:** 14.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=1 side=start shift=0 run=462 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=1 side=end shift=0 run=462 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=2 side=start shift=0 run=55 left=61 right=55
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=2 side=end shift=0 run=55 left=64 right=0
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=3 side=start shift=7 run=388 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=3 side=end shift=7 run=388 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=4 side=start shift=30 run=45 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGPICA.SPR gap=4 side=end shift=30 run=45 left=64 right=59
SPR_M259_BOUNDARY resource=BRDGDATA.SPR gap=1 side=start shift=-20 run=215 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGDATA.SPR gap=1 side=end shift=-20 run=215 left=64 right=62
SPR_M259_BOUNDARY resource=BRDGDATA.SPR gap=2 side=start shift=-17 run=314 left=64 right=64
SPR_M259_BOUNDARY resource=BRDGDATA.SPR gap=2 side=end shift=-17 run=314 left=64 right=64
SPR_M259_SUMMARY boundaries=12 crossGapPairs=60
SPR_M259_NOTE MSB-first, observational longest-run boundaries. Boundary-centered 16+16 windows with local shift +/-8. Candidates require >=26/32 matches in both A and B. No codec semantics inferred.
```
