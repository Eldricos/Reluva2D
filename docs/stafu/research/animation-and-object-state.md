# Animation and graphical object state

**M447 static classification; interpretations remain provisional.**

- Function 0x8C536 has 59 raw E8 caller matches and a bounded CFG with 211 decoded instructions. It accepts an object and index; checks index against `[object+0x94]`, uses a table at `[object+0x88]`, and updates selected index at `[object+0x8C]`.
- It reads and adjusts signed coordinate-like values in the object, sometimes scaled using `[object+0x98]`; manipulates flag bits in `[object+0x38]`; stores a computed value at `[object+0x44]`; calls 0x8CFD3.
- Function 0x7C34C returns `[object+0x44]` (16 decoded instructions; one raw caller at 0x7C028).
- Function 0x7C36C returns `[object+0x38]` (16 decoded instructions; 34 raw caller matches).
- Function 0x89801 initializes a buffered stream (85 decoded instructions; one raw caller at 0x7BF08); it is an I/O helper, not an identified decoder.
- At 0x7C028 the returned `+0x44` value is compared against a sum of local variables. At 0x7C04C the returned `+0x38` flags are tested with bit 0x04. The same routine checks 0x90474 against 0x1B (possibly an Escape-key/event code, **not verified**).

**Inference:** indexed animation/graphical-state selection and position/flag monitoring. Not proof of sprite decompression.

**Evidence:** M447 output `m447-buffered-control-helper-classification.txt` (user-run, not yet committed); script `scripts/stafu-m447-buffered-control-helper-classification.py`. Full output must be sanitized and archived separately.
