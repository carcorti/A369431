# A369431 n=22 Canonical Post-Result Validation — Codex

Date: 2026-07-21

## Scope

This is a bounded post-result validation of Carlo's completed canonical
`target=22` run. It does not rerun the campaign and does not use the unproved
recurrence as a generator, filter, or certificate.

This report covers the reviewed canonical maximum-insertion production
candidate:

- one canonical C17 source file;
- one canonical Makefile;
- one append-only manifest TSV for `target=22`;
- one aggregate TSV for `target=22`.

## Production candidate identity

```text
src/a369431.c                     6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64
src/Makefile                      e0bf20962233c7c1fc8191404ceffb361755bd109175e883ff0239044e86ec24
src/a369431                       03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143
outputs/a369431_n22.manifest.tsv  2345855e6ce20b871d218e85d0327c291606e4300ae31e626766501947ed903d
outputs/a369431_n22.aggregate.tsv 24e595461e9b07f20d69536c5464cc71ca8b4cf0c58a61c1eb74c0c6b8c308e9
```

## Aggregate result

`outputs/a369431_n22.aggregate.tsv` records:

```text
sequence  target  coverage                           leaves       nodes
A369431   22      groups=8/8;units=66/66;atoms=20/20 26674199972  38768219576
```

Canonical candidate value:

```text
a(22) = 26674199972
```

## Operational grouping

The eight canonical operational groups completed with these elapsed call
durations reported by Carlo:

```text
group 0: 5572.622248 s
group 1: 3719.428207 s
group 2: 3247.018638 s
group 3: 4133.423381 s
group 4: 3316.819885 s
group 5: 4233.059420 s
group 6: 3628.828352 s
group 7: 3938.697878 s
total:   31789.898009 s
```

The total manifest-complete elapsed time is approximately `8h 49m 49.90s`.
Groups 1--7 are within Carlo's requested approximate 30--90 minute run-block
range. Group 0 ran approximately `1h 32m 52.62s`, slightly above 90 minutes;
this was accepted as a small fixed-group imbalance during an already reviewed
canonical campaign, not as a reason to modify code or restart the run.

## Checks performed

### 1. Bounded selftest

Command:

```bash
src/a369431 selftest
```

Result:

```text
A369431 selftest: roots=ok order-5-units=66 incremental-via-n=9 direct-oracle-via-n=9 insertion-replay-via-n=12 recurrence-audit(n=4..20, checksum-only)=agree
```

### 2. Manifest completion and uniqueness

Command:

```bash
awk -F '\t' 'NR==1 {next} {status[$5]++; if ($5=="complete") {leaves+=$6; nodes+=$7; units[$3]++}} END {print "status_complete",status["complete"]+0; print "status_running",status["running"]+0; print "sum_leaves",leaves; print "sum_nodes",nodes; missing=0; dup=0; for (i=0;i<66;i++) {if (units[i]!=1) {print "unit_count_problem",i,units[i]+0; if (units[i]==0) missing++; if (units[i]>1) dup++}} print "missing_complete_units",missing; print "duplicate_complete_units",dup}' outputs/a369431_n22.manifest.tsv
```

Result:

```text
status_complete 66
status_running 66
sum_leaves 26674199972
sum_nodes 38768219576
missing_complete_units 0
duplicate_complete_units 0
```

### 3. Aggregate equals manifest sum

Command:

```bash
awk -F '\t' 'NR==FNR {if (FNR>1 && $5=="complete") {leaves+=$6; nodes+=$7}; next} FNR==2 {print "aggregate_leaves",$4; print "aggregate_nodes",$5; print "manifest_leaves",leaves; print "manifest_nodes",nodes; print "leaves_match",($4==leaves?"yes":"no"); print "nodes_match",($5==nodes?"yes":"no"); print "coverage",$3; print "source_sha",$6; print "binary_sha",$7; print "manifest_sha_in_output",$8}' outputs/a369431_n22.manifest.tsv outputs/a369431_n22.aggregate.tsv
```

Result:

```text
aggregate_leaves 26674199972
aggregate_nodes 38768219576
manifest_leaves 26674199972
manifest_nodes 38768219576
leaves_match yes
nodes_match yes
coverage groups=8/8;units=66/66;atoms=20/20
source_sha 6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64
binary_sha 03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143
manifest_sha_in_output 2345855e6ce20b871d218e85d0327c291606e4300ae31e626766501947ed903d
```

### 4. Post-hoc recurrence checksum only

Command:

```bash
awk 'BEGIN {a19=809796400; a20=2595858574; a21=8321204878; pred=5*a21-7*a20+4*a19; print "recurrence_checksum_a22",pred; print "candidate",26674199972; print "match",(pred==26674199972?"yes":"no")}'
```

Result:

```text
recurrence_checksum_a22 26674199972
candidate 26674199972
match yes
```

This is not a proof of the value. It is only a post-hoc checksum against an
unproved recurrence.

## Verdict

The completed canonical `n=22` run is internally consistent:

- all 66 internal units are complete exactly once;
- aggregate leaves and nodes equal the complete manifest sums;
- aggregate coverage is complete over groups, internal units, and order-4
  mathematical atoms;
- source and binary hashes match the reviewed canonical production candidate;
- bounded selftest still passes after the run;
- the operational grouping was within the intended human stop/go cadence except
  for a small fixed-group overrun in group 0;
- the unproved recurrence agrees post hoc.

The value

```text
a(22) = 26674199972
```

is therefore the canonical result of the reviewed formula-free
maximum-insertion production run, pending any further independent
external/post-publication validation Carlo chooses to request.
