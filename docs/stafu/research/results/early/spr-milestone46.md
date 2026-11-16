# spr-milestone46.txt — curated observations

**Source:** `ResearchArchive.zip` / `spr-milestone46.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `a94fe45f173027e9032d80f6cc87bd371eadf5dc2a08a66daec3930e2db8697a`; **encoding:** `utf-16-le`; **original lines:** 10; **selected lines:** 10.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
DNAR relativeEvent=1 events=17,40 chunkOffsets=7148,53196 wordsA=24466,8466 wordsB=23466,8466 deltas=-1000,0 changed=1
rawA=92 5f 00 00 12 21 00 00 rawB=aa 5b 00 00 12 21 00 00
neighbor=before tags=MOCS,MOCS sizes=24,24 values=617,46665 shift=46048
neighbor=after tags=MOCS,MOCS sizes=24,24 values=485,46533 shift=46048
DNAR relativeEvent=9 events=25,48 chunkOffsets=7296,53344 wordsA=23466,8466 wordsB=43466,15466 deltas=20000,7000 changed=1
rawA=aa 5b 00 00 12 21 00 00 rawB=ca a9 00 00 6a 3c 00 00
neighbor=before tags=MOCS,MOCS sizes=24,24 values=765,46813 shift=46048
neighbor=after tags=MOCS,MOCS sizes=24,24 values=633,46681 shift=46048
SUMMARY dnarPairs=2 differing=2 otherEventPairs=15 expectedTranslation=46048
DNAR word interpretations remain unknown; deltas are descriptive, not pointer validation.
```
