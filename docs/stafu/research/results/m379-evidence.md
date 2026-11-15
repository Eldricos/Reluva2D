# M379 — dispatcher control flow audit

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m379-dispatcher-control-flow-audit.txt`
- Original bytes: 47894
- SHA256: `dde0d97bb0cb6ed6f67710dc85e6b913672d5fc54f81f8ecd981d52e6c761c8f`
- Original encoding: `utf-16`
- Original line count: 299
- Curated lines retained: 35

## Selected diagnostic observations

- `M379_DISPATCH name=object_state jumpRva=0xf579 jumpPhysical=0x8a50d exactSignature=1 tableOperand=0x12334`
- `M379_BACKTRACK name=object_state startRva=0xf519 reachesJump=1 instructionCount=30`
- `M379_BACKTRACK name=object_state startRva=0xf539 reachesJump=1 instructionCount=20`
- `M379_BACKTRACK name=object_state startRva=0xf549 reachesJump=1 instructionCount=15`
- `M379_BACKTRACK name=object_state startRva=0xf559 reachesJump=1 instructionCount=10`
- `M379_BACKTRACK name=object_state startRva=0xf561 reachesJump=1 instructionCount=8`
- `M379_BACKTRACK name=object_state startRva=0xf569 reachesJump=1 instructionCount=5`
- `M379_BACKTRACK name=object_state startRva=0xf571 reachesJump=0 instructionCount=0`
- `M379_CFG_SEED name=object_state seed=0xf100 decoded=135 directEdges=19 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf110 decoded=129 directEdges=19 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf120 decoded=12 directEdges=1 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf519 decoded=25 directEdges=5 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf539 decoded=26 directEdges=5 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf559 decoded=16 directEdges=4 jumpReached=0`
- `M379_CFG_SEED name=object_state seed=0xf569 decoded=20 directEdges=4 jumpReached=1`
- `M379_CFG_SUMMARY name=object_state uniqueDecoded=176 directEdges=27 directInboundToJump=0 inbound=none`
- `M379_DIRECT_TABLE_EDGES name=object_state targetRva=0x12334 count=0 sources=none`
- `M379_DIRECT_TABLE_EDGES name=object_state targetRva=0x12338 count=0 sources=none`
- `M379_DIRECT_TABLE_EDGES name=object_state targetRva=0x12350 count=0 sources=none`
- `M379_DISPATCH name=global_state jumpRva=0x106c1 jumpPhysical=0x8b655 exactSignature=1 tableOperand=0x13488`
- `M379_BACKTRACK name=global_state startRva=0x10661 reachesJump=1 instructionCount=37`
- `M379_BACKTRACK name=global_state startRva=0x10681 reachesJump=1 instructionCount=25`
- `M379_BACKTRACK name=global_state startRva=0x10691 reachesJump=1 instructionCount=20`
- `M379_BACKTRACK name=global_state startRva=0x106a1 reachesJump=1 instructionCount=15`
- `M379_BACKTRACK name=global_state startRva=0x106a9 reachesJump=1 instructionCount=12`
- `M379_BACKTRACK name=global_state startRva=0x106b1 reachesJump=1 instructionCount=10`
- `M379_BACKTRACK name=global_state startRva=0x106b9 reachesJump=1 instructionCount=5`
- `M379_CFG_SEED name=global_state seed=0x10200 decoded=111 directEdges=16 jumpReached=0`
- `M379_CFG_SEED name=global_state seed=0x10210 decoded=86 directEdges=11 jumpReached=0`
- `M379_CFG_SEED name=global_state seed=0x10220 decoded=46 directEdges=5 jumpReached=0`
- `M379_CFG_SEED name=global_state seed=0x10661 decoded=199 directEdges=28 jumpReached=1`
- `M379_CFG_SEED name=global_state seed=0x10681 decoded=25 directEdges=6 jumpReached=1`
- `M379_CFG_SEED name=global_state seed=0x106a1 decoded=17 directEdges=2 jumpReached=1`
- `M379_CFG_SEED name=global_state seed=0x106b1 decoded=8 directEdges=0 jumpReached=0`
- `M379_CFG_SUMMARY name=global_state uniqueDecoded=325 directEdges=50 directInboundToJump=0 inbound=none`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
