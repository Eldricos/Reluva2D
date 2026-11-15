# M407 — indirect export call validation

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m407-indirect-export-call-validation.txt`
- Original bytes: 25858
- SHA256: `c489308e67ddfae8d92d8ffffca3f6f9c25a0431b1eac987b0f80fbaee1e3f4a`
- Original encoding: `utf-16`
- Original line count: 123
- Curated lines retained: 13

## Selected diagnostic observations

- `M407_EXPORT name=VMFABIND dsOffset=0x1a04 callCount=2 sites=0x297bf,0x29deb`
- `M407_EXPORT name=VMFMA dsOffset=0x1a10 callCount=1 sites=0x284f3`
- `M407_EXPORT name=VMFILEIO dsOffset=0x1a08 callCount=1 sites=0x2ce6d`
- `M407_EXPORT name=DVX_Global_Seg_Associate dsOffset=0x3290 callCount=2 sites=0x288f5,0x2cc53`
- `M407_EXPORT name=DVX_Global_Seg_Alloc dsOffset=0x32ac callCount=2 sites=0x28a70,0x2cbc7`
- `M407_EXPORT name=DVX_Global_Seg_Broadcast dsOffset=0x32b0 callCount=2 sites=0x2cc8b,0x2cd19`
- `M407_EXPORT name=DVX_Dos4gx_Get_Option dsOffset=0x32a8 callCount=2 sites=0x2f2b7,0x2f478`
- `M407_EXPORT name=VMALOCINST dsOffset=0x3294 callCount=2 sites=0x2b427,0x2c2a5`
- `M407_EXPORT name=VSUPERRESIZE dsOffset=0x3298 callCount=0 sites=none`
- `M407_EXPORT name=VSUPERFREE dsOffset=0x329c callCount=1 sites=0x28f06`
- `M407_EXPORT name=DOS4G_GETENV dsOffset=0x32a0 callCount=1 sites=0x2f16e`
- `M407_EXPORT name=DOS4G_GET_APPPATH dsOffset=0x32a4 callCount=1 sites=0x27a16`
- `M407_TOTAL exports=12 indirectFarCallMatches=17`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
