# A369431 n=21 Canonical Post-Result Validation — Codex

Date: 2026-07-20

## Scope

This is a bounded post-result validation of Carlo's completed canonical
`target=21` run. It does not rerun the campaign and does not use the unproved
recurrence as a generator, filter, or certificate.

The previous `n=21` run has been archived as pre-canonical evidence because it
was produced by a superseded source/binary identity. This report covers the
canonical grouping candidate only.

## Production candidate identity

```text
src/a369431.c                     6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64
src/Makefile                      e0bf20962233c7c1fc8191404ceffb361755bd109175e883ff0239044e86ec24
src/a369431                       03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143
outputs/a369431_n21.manifest.tsv  29a8bafe4a4c95692a9107e875759ee6168dbd92096fbbcef83e6cdf25a2f248
outputs/a369431_n21.aggregate.tsv 6b892e47e1a426b0a5de40237e5e4686c5915c1195a73679afb81e7186cc91e8
```

Archived pre-canonical evidence:

```text
outputs/a369431_n21.precanonical_20260720.manifest.tsv d2317ae01275f7cd9adf157c2704a40947457f776a9b226f45f51f8eab0515a2
outputs/a369431_n21.precanonical_20260720.aggregate.tsv 1b471c8f28730000b44210a736d22b02dba90ae96421cafc50a1720557147961
```

## Aggregate result

`outputs/a369431_n21.aggregate.tsv` records:

```text
sequence  target  coverage                           leaves      nodes
A369431   21      groups=4/4;units=66/66;atoms=20/20 8321204878 12094019604
```

Canonical candidate value:

```text
a(21) = 8321204878
```

## Operational grouping

The four canonical operational groups completed with these wall/user-call
durations reported by Carlo:

```text
group 0: 2443.267849 s
group 1: 2222.643238 s
group 2: 1863.867420 s
group 3: 2227.944729 s
total:   8757.72 s
```

All four operational invocations are in Carlo's requested approximate
30--90 minute run-block range.

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
awk -F '\t' 'NR==1 {next} {status[$5]++; if ($5=="complete") {leaves+=$6; nodes+=$7; units[$3]++}} END {print "status_complete",status["complete"]+0; print "status_running",status["running"]+0; print "sum_leaves",leaves; print "sum_nodes",nodes; missing=0; dup=0; for (i=0;i<66;i++) {if (units[i]!=1) {print "unit_count_problem",i,units[i]+0; if (units[i]==0) missing++; if (units[i]>1) dup++}} print "missing_complete_units",missing; print "duplicate_complete_units",dup}' outputs/a369431_n21.manifest.tsv
```

Result:

```text
status_complete 66
status_running 66
sum_leaves 8321204878
sum_nodes 12094019604
missing_complete_units 0
duplicate_complete_units 0
```

### 3. Aggregate equals manifest sum

Command:

```bash
awk -F '\t' 'NR==FNR {if (FNR>1 && $5=="complete") {leaves+=$6; nodes+=$7}; next} FNR==2 {print "aggregate_leaves",$4; print "aggregate_nodes",$5; print "manifest_leaves",leaves; print "manifest_nodes",nodes; print "leaves_match",($4==leaves?"yes":"no"); print "nodes_match",($5==nodes?"yes":"no"); print "coverage",$3; print "source_sha",$6; print "binary_sha",$7; print "manifest_sha_in_output",$8}' outputs/a369431_n21.manifest.tsv outputs/a369431_n21.aggregate.tsv
```

Result:

```text
aggregate_leaves 8321204878
aggregate_nodes 12094019604
manifest_leaves 8321204878
manifest_nodes 12094019604
leaves_match yes
nodes_match yes
coverage groups=4/4;units=66/66;atoms=20/20
source_sha 6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64
binary_sha 03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143
manifest_sha_in_output 29a8bafe4a4c95692a9107e875759ee6168dbd92096fbbcef83e6cdf25a2f248
```

### 4. Post-hoc recurrence checksum only

Command:

```bash
awk 'NR==1 {a18=252621702; a19=809796400; a20=2595858574; pred=5*a20-7*a19+4*a18; print "recurrence_checksum_a21",pred; print "candidate",8321204878; print "match",(pred==8321204878?"yes":"no")}' data/b369431.txt
```

Result:

```text
recurrence_checksum_a21 8321204878
candidate 8321204878
match yes
```

This is not a proof of the value. It is only a post-hoc checksum against an
unproved recurrence.

## Verdict

The completed canonical `n=21` run is internally consistent:

- all 66 internal units are complete exactly once;
- aggregate leaves and nodes equal the complete manifest sums;
- aggregate coverage is complete over groups, units, and order-4 atoms;
- source and binary hashes match the reviewed canonical production candidate;
- bounded selftest still passes after the run;
- operational group durations satisfy Carlo's 30--90 minute stop/go criterion;
- the unproved recurrence agrees post hoc.

The value

```text
a(21) = 8321204878
```

is therefore the canonical result of the reviewed formula-free maximum-insertion
production run, pending any further independent external/post-publication
validation Carlo chooses to request.
