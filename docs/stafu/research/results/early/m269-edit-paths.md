# m269-edit-paths.txt — curated observations

**Source:** `ResearchArchive.zip` / `m269-edit-paths.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `d97c3895688c4ed0e2e31d5f56150084b9cb88e8769310e31677cc11bf11ff23`; **encoding:** `utf-16-le`; **original lines:** 172; **selected lines:** 90.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M269_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M269_SEGMENT resource=BRDGPICA.SPR interval=1 mask=0 segment=1 a=9908 b=9967 bitsA=12 bitsB=22 delta=10
SPR_M269_BITS resource=BRDGPICA.SPR interval=1 mask=0 segment=1 bitsA=100111100111 bitsB=0110110010100111100110
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=0 segment=1 type=INSERT a=9908 b=9967
SPR_M269_EDIT resource=BRDGPICA.SPR interval=1 mask=0 segment=1 distance=11 optimalPaths=226 optimalEdges=121 forcedMatch=0 forcedSub=0 forcedInsert=1 forcedDelete=0
SPR_M269_SCENARIO resource=BRDGPICA.SPR interval=1 mask=0 matched=0 remainingA=12 remainingB=22 monotonic=1
SPR_M269_SEGMENT resource=BRDGPICA.SPR interval=1 mask=1 segment=1 a=9908 b=9967 bitsA=0 bitsB=10 delta=10
SPR_M269_BITS resource=BRDGPICA.SPR interval=1 mask=1 segment=1 bitsA=- bitsB=0110110010
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9967
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9968
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9969
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9970
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9971
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9972
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9973
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9974
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9975
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=1 type=INSERT a=9908 b=9976
SPR_M269_EDIT resource=BRDGPICA.SPR interval=1 mask=1 segment=1 distance=10 optimalPaths=1 optimalEdges=10 forcedMatch=0 forcedSub=0 forcedInsert=10 forcedDelete=0
SPR_M269_CONSTRAINT resource=BRDGPICA.SPR interval=1 mask=1 a=9908 b=9977 length=11 displacement=69
SPR_M269_SEGMENT resource=BRDGPICA.SPR interval=1 mask=1 segment=2 a=9919 b=9988 bitsA=1 bitsB=1 delta=0
SPR_M269_BITS resource=BRDGPICA.SPR interval=1 mask=1 segment=2 bitsA=1 bitsB=0
SPR_M269_FORCED resource=BRDGPICA.SPR interval=1 mask=1 segment=2 type=SUB a=9919 b=9988
SPR_M269_EDIT resource=BRDGPICA.SPR interval=1 mask=1 segment=2 distance=1 optimalPaths=1 optimalEdges=1 forcedMatch=0 forcedSub=1 forcedInsert=0 forcedDelete=0
SPR_M269_SCENARIO resource=BRDGPICA.SPR interval=1 mask=1 matched=11 remainingA=1 remainingB=11 monotonic=1
SPR_M269_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M269_SEGMENT resource=BRDGDATA.SPR interval=1 mask=0 segment=1 a=11181 b=11127 bitsA=74 bitsB=77 delta=3
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11190 b=11135
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11191 b=11136
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11192 b=11137
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11193 b=11138
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=SUB a=11194 b=11139
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11195 b=11140
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11196 b=11141
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11197 b=11142
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11198 b=11143
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=DELETE a=11199 b=11144
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11200 b=11144
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11201 b=11145
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11202 b=11146
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11203 b=11147
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11204 b=11148
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=SUB a=11205 b=11149
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11206 b=11150
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11207 b=11151
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11208 b=11152
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11209 b=11153
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=SUB a=11210 b=11154
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11211 b=11155
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11212 b=11156
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11213 b=11157
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11214 b=11158
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11215 b=11159
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11216 b=11160
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11244 b=11193
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=0 segment=1 type=MATCH a=11245 b=11194
SPR_M269_EDIT resource=BRDGDATA.SPR interval=1 mask=0 segment=1 distance=21 optimalPaths=10710 optimalEdges=159 forcedMatch=25 forcedSub=3 forcedInsert=0 forcedDelete=1
SPR_M269_SCENARIO resource=BRDGDATA.SPR interval=1 mask=0 matched=0 remainingA=74 remainingB=77 monotonic=1
SPR_M269_SEGMENT resource=BRDGDATA.SPR interval=1 mask=1 segment=1 a=11181 b=11127 bitsA=40 bitsB=36 delta=-4
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11190 b=11135
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11191 b=11136
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11192 b=11137
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11193 b=11138
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=SUB a=11194 b=11139
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11195 b=11140
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11196 b=11141
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11197 b=11142
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11198 b=11143
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=DELETE a=11199 b=11144
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11200 b=11144
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11201 b=11145
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11202 b=11146
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11203 b=11147
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11204 b=11148
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=SUB a=11205 b=11149
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11206 b=11150
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11207 b=11151
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11208 b=11152
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11209 b=11153
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=SUB a=11210 b=11154
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11211 b=11155
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11212 b=11156
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11213 b=11157
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11214 b=11158
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11215 b=11159
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11216 b=11160
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=MATCH a=11219 b=11162
SPR_M269_FORCED resource=BRDGDATA.SPR interval=1 mask=1 segment=1 type=DELETE a=11220 b=11163
SPR_M269_EDIT resource=BRDGDATA.SPR interval=1 mask=1 segment=1 distance=11 optimalPaths=30 optimalEdges=56 forcedMatch=24 forcedSub=3 forcedInsert=0 forcedDelete=2
SPR_M269_CONSTRAINT resource=BRDGDATA.SPR interval=1 mask=1 a=11221 b=11163 length=13 displacement=-58
```
