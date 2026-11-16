# computer-boundaries-output.txt — curated observations

**Source:** `ResearchArchive.zip` / `computer-boundaries-output.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `00647a59bdc8dbcf804225fcdea404259e1b1021792822f0783faa1b1fcc03e7`; **encoding:** `utf-16-le`; **original lines:** 87; **selected lines:** 77.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
baseline entries=346 roots=1 maxDepth=12 incomplete=2
unfinished [0] observed=2 expected=9 label="Primary Core"
unfinished [166] observed=3 expected=9 label="Cultures"
Boundary contexts in the unmodified baseline:
Boundary [0]:
[0] depth=0 parent=ROOT header=(9,2) children=2/9 label="Primary Core"
[1] depth=1 parent=0 header=(3,2) children=3/3 label="USS Enterprise"
ancestors with incomplete final child counts (not boundary-time balances):
none (using final totals; not boundary-time balances)
Boundary [55]:
[54] depth=2 parent=1 header=(0,0) children=0/0 label="Support Craft"
[55] depth=1 parent=0 header=(4,2) children=4/4 label="Sciences"
[56] depth=2 parent=55 header=(4,2) children=4/4 label="Astronomy"
ancestors with incomplete final child counts (not boundary-time balances):
[0] 2/9 "Primary Core"
Boundary [162]:
[161] depth=4 parent=155 header=(0,0) children=0/0 label="Chameleon"
[162] depth=2 parent=55 header=(2,2) children=2/2 label="Starfleet"
[163] depth=3 parent=162 header=(2,0) children=2/2 label="Communiques"
ancestors with incomplete final child counts (not boundary-time balances):
[0] 2/9 "Primary Core"
Boundary [166]:
[165] depth=4 parent=163 header=(0,0) children=0/0 label="Morassia"
[166] depth=3 parent=162 header=(9,2) children=3/9 label="Cultures"
[167] depth=4 parent=166 header=(2,0) children=2/2 label="Federation"
ancestors with incomplete final child counts (not boundary-time balances):
[0] 2/9 "Primary Core"
Boundary [192]:
[191] depth=8 parent=185 header=(0,0) children=0/0 label="Political"
[192] depth=8 parent=185 header=(4,0) children=4/4 label="Missions"
[193] depth=9 parent=192 header=(9,0) children=9/9 label="Captain's Logs"
ancestors with incomplete final child counts (not boundary-time balances):
[166] 3/9 "Cultures"
[0] 2/9 "Primary Core"
Boundary [319]:
[318] depth=10 parent=285 header=(0,0) children=0/0 label="Sensors"
[319] depth=4 parent=166 header=(3,0) children=3/3 label="Destinations"
[320] depth=5 parent=319 header=(5,0) children=5/5 label="DS Stations"
ancestors with incomplete final child counts (not boundary-time balances):
[166] 3/9 "Cultures"
[0] 2/9 "Primary Core"
Boundary [338]:
[337] depth=6 parent=335 header=(0,4) children=0/0 label="Starbase 45"
[338] depth=4 parent=166 header=(7,0) children=7/7 label="Analyses"
[339] depth=5 parent=338 header=(0,0) children=0/0 label="Instruments"
ancestors with incomplete final child counts (not boundary-time balances):
[166] 3/9 "Cultures"
[0] 2/9 "Primary Core"
Independent segment resets (counterfactual, NOT established section boundaries):
[0,55) roots=1 maxDepth=6 incomplete=1 [0:1/9]
[55,162) roots=1 maxDepth=5 incomplete=1 [55:3/4]
[162,166) roots=1 maxDepth=2 incomplete=1 [162:1/2]
[166,192) roots=1 maxDepth=9 incomplete=2 [166:1/9] [185:6/7]
[192,319) roots=1 maxDepth=3 incomplete=0
[319,338) roots=1 maxDepth=2 incomplete=0
[338,346) roots=1 maxDepth=1 incomplete=0
Single-cut comparison (both sides independently interpreted):
cut=55 "Sciences"
[0,55) roots=1 maxDepth=6 incomplete=1 [0:1/9]
[55,346) roots=1 maxDepth=11 incomplete=1 [166:3/9]
cut=162 "Starfleet"
[0,162) roots=1 maxDepth=6 incomplete=2 [0:2/9] [55:3/4]
[162,346) roots=1 maxDepth=10 incomplete=1 [166:3/9]
cut=166 "Cultures"
[0,166) roots=1 maxDepth=6 incomplete=2 [0:2/9] [162:1/2]
[166,346) roots=1 maxDepth=9 incomplete=1 [166:3/9]
cut=192 "Missions"
[0,192) roots=1 maxDepth=12 incomplete=3 [0:2/9] [166:1/9] [185:6/7]
[192,346) roots=3 maxDepth=3 incomplete=0
cut=319 "Destinations"
[0,319) roots=1 maxDepth=12 incomplete=2 [0:2/9] [166:1/9]
[319,346) roots=2 maxDepth=2 incomplete=0
cut=338 "Analyses"
[0,338) roots=1 maxDepth=12 incomplete=2 [0:2/9] [166:2/9]
[338,346) roots=1 maxDepth=1 incomplete=0
Interpretation: forced cuts discard pending parent slots. Fewer unfinished
nodes alone cannot prove a boundary; compare labels and parent plausibility.
```
