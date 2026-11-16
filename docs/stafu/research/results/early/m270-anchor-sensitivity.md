# m270-anchor-sensitivity.txt — curated observations

**Source:** `ResearchArchive.zip` / `m270-anchor-sensitivity.txt`. **Evidence type:** selected lines from a diagnostic output, not a complete raw-output backup. No original game bytes are included.

**Source SHA256:** `4007db1dd9515c22364dd326bfa154c3f5d02ea21f1b40e1cc8339abd538061b`; **encoding:** `utf-16-le`; **original lines:** 29; **selected lines:** 29.

**Interpretation:** diagnostic observations only. Field names and candidate matches do not establish decoding semantics or runtime behavior.

```text
SPR_M270_INTERVAL resource=BRDGPICA.SPR gap=3 interval=1 a=9908 bitsA=12 b=9967 bitsB=22 deltaBits=10
SPR_M270_PROFILE resource=BRDGPICA.SPR interval=1 substitution=1 insertion=1 deletion=1 unconstrainedCost=11 unconstrainedPaths=226
SPR_M270_ANCHOR resource=BRDGPICA.SPR interval=1 substitution=1 mask=1 constrainedCost=11 penalty=0 constrainedPaths=1 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGPICA.SPR interval=1 substitution=2 insertion=1 deletion=1 unconstrainedCost=12 unconstrainedPaths=1170
SPR_M270_ANCHOR resource=BRDGPICA.SPR interval=1 substitution=2 mask=1 constrainedCost=12 penalty=0 constrainedPaths=3 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGPICA.SPR interval=1 substitution=3 insertion=1 deletion=1 unconstrainedCost=12 unconstrainedPaths=944
SPR_M270_ANCHOR resource=BRDGPICA.SPR interval=1 substitution=3 mask=1 constrainedCost=12 penalty=0 constrainedPaths=2 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_INTERVAL resource=BRDGDATA.SPR gap=2 interval=1 a=11181 bitsA=74 b=11127 bitsB=77 deltaBits=3
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=1 substitution=1 insertion=1 deletion=1 unconstrainedCost=21 unconstrainedPaths=10710
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=1 substitution=1 mask=1 constrainedCost=22 penalty=1 constrainedPaths=5400 anyOptimal=0 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=1 substitution=2 insertion=1 deletion=1 unconstrainedCost=29 unconstrainedPaths=64945953792
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=1 substitution=2 mask=1 constrainedCost=29 penalty=0 constrainedPaths=3375544320 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=1 substitution=3 insertion=1 deletion=1 unconstrainedCost=29 unconstrainedPaths=19958822976
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=1 substitution=3 mask=1 constrainedCost=29 penalty=0 constrainedPaths=1113516480 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_INTERVAL resource=BRDGDATA.SPR gap=2 interval=2 a=11569 bitsA=76 b=11518 bitsB=79 deltaBits=3
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=2 substitution=1 insertion=1 deletion=1 unconstrainedCost=23 unconstrainedPaths=4671744
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=1 mask=1 constrainedCost=23 penalty=0 constrainedPaths=31020 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=1 mask=2 constrainedCost=23 penalty=0 constrainedPaths=1743672 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=1 mask=3 constrainedCost=23 penalty=0 constrainedPaths=13160 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=2 substitution=2 insertion=1 deletion=1 unconstrainedCost=27 unconstrainedPaths=440115840
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=2 mask=1 constrainedCost=29 penalty=2 constrainedPaths=405237600 anyOptimal=0 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=2 mask=2 constrainedCost=27 penalty=0 constrainedPaths=127451520 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=2 mask=3 constrainedCost=29 penalty=2 constrainedPaths=119760120 anyOptimal=0 allOptimal=0 valid=1
SPR_M270_PROFILE resource=BRDGDATA.SPR interval=2 substitution=3 insertion=1 deletion=1 unconstrainedCost=27 unconstrainedPaths=187484142
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=3 mask=1 constrainedCost=29 penalty=2 constrainedPaths=144191250 anyOptimal=0 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=3 mask=2 constrainedCost=27 penalty=0 constrainedPaths=48314250 anyOptimal=1 allOptimal=0 valid=1
SPR_M270_ANCHOR resource=BRDGDATA.SPR interval=2 substitution=3 mask=3 constrainedCost=29 penalty=2 constrainedPaths=38016000 anyOptimal=0 allOptimal=0 valid=1
SPR_M270_SUMMARY intervals=3 scenarios=8 resources=2
SPR_M270_NOTE Substitution costs 1/2/3, insertion/deletion cost 1; exact path counts and enforced-anchor cost penalties. No codec interpretation.
```
