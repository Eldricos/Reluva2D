# M432 — resource data boundaries

> Curated output metadata and selected diagnostic status/summary lines; **not a complete original TXT backup**.

- Original filename: `m432-resource-data-boundaries.txt`
- Original bytes: 211254
- SHA256: `34bf9252eca04e62edaffe44078d0f3a34f7d973fedf89dde3f7ddec6fe40ede`
- Original encoding: `utf-16`
- Original line count: 813
- Curated lines retained: 35

## Selected diagnostic observations

- `M432_OBJECT1 start=0x7af94 end=0x15382e mappingAssumedContiguous=1`
- `M432_FUNCTION_SUMMARY name=stream_position_wrapper decoded=129 calls=12 returns=3`
- `M432_CALLEE function=stream_position_wrapper site=0x897d1 target=0x7e638`
- `M432_CALLEE function=stream_position_wrapper site=0x897f4 target=0x7e5a4`
- `M432_CALLEE function=stream_position_wrapper site=0x89832 target=0x7e410`
- `M432_CALLEE function=stream_position_wrapper site=0x8984f target=0x8ff50`
- `M432_CALLEE function=stream_position_wrapper site=0x89868 target=0x8ff50`
- `M432_CALLEE function=stream_position_wrapper site=0x89870 target=0x7af04`
- `M432_CALLEE function=stream_position_wrapper site=0x8987e target=0x7c2d0`
- `M432_CALLEE function=stream_position_wrapper site=0x898b9 target=0x8f790`
- `M432_CALLEE function=stream_position_wrapper site=0x898db target=0x8ff50`
- `M432_CALLEE function=stream_position_wrapper site=0x898f4 target=0x8ff50`
- `M432_CALLEE function=stream_position_wrapper site=0x898fc target=0x7af04`
- `M432_CALLEE function=stream_position_wrapper site=0x8990a target=0x7c2d0`
- `M432_LIMIT name=stream_position_wrapper linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1`
- `M432_FUNCTION_SUMMARY name=resource_lifecycle decoded=120 calls=4 returns=2`
- `M432_CALLEE function=resource_lifecycle site=0x8a545 target=0x8ff50`
- `M432_CALLEE function=resource_lifecycle site=0x8a566 target=0x11be64`
- `M432_CALLEE function=resource_lifecycle site=0x8a593 target=0x11be64`
- `M432_CALLEE function=resource_lifecycle site=0x8a63b target=0x93813`
- `M432_LIMIT name=resource_lifecycle linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1`
- `M432_FUNCTION_SUMMARY name=state_transition_helper decoded=149 calls=4 returns=4`
- `M432_CALLEE function=state_transition_helper site=0x93575 target=0x930a4`
- `M432_CALLEE function=state_transition_helper site=0x935aa target=0x930a4`
- `M432_CALLEE function=state_transition_helper site=0x93611 target=0x930a4`
- `M432_CALLEE function=state_transition_helper site=0x936d7 target=dword ptr [ebp + 0x18]`
- `M432_LIMIT name=state_transition_helper linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1`
- `M432_FUNCTION_SUMMARY name=lower_level_a decoded=93 calls=0 returns=0`
- `M432_LIMIT name=lower_level_a linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1`
- `M432_FUNCTION_SUMMARY name=lower_level_b decoded=99 calls=0 returns=1`
- `M432_LIMIT name=lower_level_b linearSweep=1 windowNotFunctionBoundary=1 reachabilityUnverified=1`
- `M432_DIRECT_CALLS name=stream_position_wrapper target=0x897b3 rawCount=14`
- `M432_CALLSITE name=stream_position_wrapper file=0x7eba4 localAnchor=0x7eb64`
- `M432_CALLSITE name=stream_position_wrapper file=0x7ebe6 localAnchor=0x7eba6`
- `M432_CALLSITE name=stream_position_wrapper file=0x7ec28 localAnchor=0x7ebe8`

## Evidence limitations

These are filtered diagnostic status/summary observations. Original detailed disassembly, binary windows and full result rows are not reproduced here. Recheck the original TXT when a specific claim requires those omitted details.
