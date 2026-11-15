# Resource management and object structures

**Confirmed static observations / traced candidate functions:**
- M327–M348 examined resource-object read/seek behavior.
- M420–M424 identified 33+ SPR-related strings and a table of **231** consecutive 12-byte entries in object 2, approximately file offsets 0x164554–0x16501C. Interpretation as a resource string/pointer table is **inferred**, not confirmed.
- M425–M427 inspected a reference to `brdgrikr.spr`; the associated function near 0x8FF50 was classified as formatting/logging rather than a proven resource loader.
- M428–M430 ranked raw E8 caller candidates; raw byte matches are not runtime call proof.
- M431–M436 traced lifecycle/state routines near 0x8A3F4 and 0x8A518, generic linked-list construction near 0x93569, allocator 0x930A4, release 0x93287, destructor 0x936A1, and low-level segmented/interrupt helpers near 0x11B850 and 0x11BE64.

**Open:** definitive resource-object ownership, which table entries refer to sprites, and the call chain from named SPR resources to pixel decoding.

**Provenance:** M327–M436 diagnostic scripts and conversation-reviewed outputs; full TXT files not yet archived.
