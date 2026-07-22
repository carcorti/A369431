# Validation Evidence Ledger — A369431 Canonical Operational Grouping Candidate

## Frozen identity

| Field | Value |
|---|---|
| Candidate source hash | `6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64` |
| Makefile hash | `e0bf20962233c7c1fc8191404ceffb361755bd109175e883ff0239044e86ec24` |
| Binary hash | `03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143` |
| Sanitizer binary hash | `6585511f01f921595d8eb79dfaba5e1f9a73102f669d24bf4fbfd4c35e613f07` |
| Clang binary hash | `5824860a13700f1ced990054430e3479e444bfc35f886c82f3f1d95a3612f1ca` |
| Reference data hash | `747c3aa1243b565c3d1dda2c99965d79f9d5c62d9a82c062de93edbce0b98459` |
| Mandate hash | `374b5f7d4263a33529f489e62def90c8d4da8518da7e7616e5a5a7eca32b10c0` |
| Project directives hash | `638b2c1dd120b300373ee1c7fa51f2e2b093ae828075e2a816779d887d139844` |
| Target host | Carlo target workspace, Linux environment |
| Candidate change boundary | Fixed operational group counts only: `n=21` 8->4, `n=22` 16->8 |

## Evidence matrix

| Test ID | Required claim / boundary | Status | Primary owner | Named repeat owner | Command or fixture | Output or log | Environment | Repeat policy | Invalidation triggers | Inherited-from row |
|---|---|---|---|---|---|---|---|---|---|---|
| canonical-build-gcc | Production source builds with GCC warning set | E | Codex | — | `make -C src clean all` | Passed, produced `src/a369431` hash above | Current workspace | owner-only | source, Makefile, compiler, flags | — |
| canonical-selftest-gcc | Bounded selftest covers roots, 66 units, incremental/direct oracle, known replay through n=12, recurrence audit through n=20, persistence/group checks | E | Codex | — | `make -C src test` | `A369431 selftest: roots=ok order-5-units=66 incremental-via-n=9 direct-oracle-via-n=9 insertion-replay-via-n=12 recurrence-audit(n=4..20, checksum-only)=agree` | Current workspace | owner-only | source, Makefile, grouping map, predicate, manifest parser | — |
| canonical-asan-ubsan | Sanitizer selftest has no ASan/UBSan finding | E | Codex | — | `make -C src sanitize` | Passed with `ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer disabled by managed ptrace environment | Current workspace | owner-only | source, sanitizer flags, allocator behavior | — |
| canonical-build-clang | Clang build and selftest pass | E | Codex | — | `make -C src clang` | Passed, produced `src/a369431_clang` hash above | Current workspace | owner-only | source, Makefile, clang version/flags | — |
| canonical-static | clang-tidy and cppcheck report no finding under configured checks | E | Codex | — | `make -C src static` | Passed | Current workspace | owner-only | source, static-analysis check set | — |
| canonical-cli-n21-bound | `n=21` now exposes groups 0..3 and rejects group 4 before campaign work | E | Codex | — | `src/a369431 run 21 4` | Exit 2; `run accepts target 4..24 and group 0..3, or next.` | Current workspace | owner-only | grouping map, CLI parsing | — |
| canonical-cli-n22-bound | `n=22` now exposes groups 0..7 and rejects group 8 before campaign work | E | Codex | — | `src/a369431 run 22 8` | Exit 2; `run accepts target 4..24 and group 0..7, or next.` | Current workspace | owner-only | grouping map, CLI parsing | — |
| canonical-known-n20 | Completed known `n=20` aggregate from superseded candidate remains mathematical/throughput evidence but is invalid for exact candidate identity | V | Carlo / target | — | `outputs/a369431_n20.aggregate.tsv` | Source hash differs from current candidate | Target host | rerun only if exact-candidate public calibration is required | source or binary hash change | prior progress candidate |
| canonical-new-n21 | Completed `n=21` aggregate from superseded candidate is pre-canonical validation evidence, not the preferred public artifact identity | V | Carlo / target | — | `outputs/a369431_n21.aggregate.tsv` | Source hash differs from current candidate | Target host | rerun if public canonical `n=21` artifact is required | source or binary hash change | prior progress candidate |
| canonical-tsan | Thread sanitizer | N/A | — | — | — | Serial implementation; no OpenMP | — | invalidate if OpenMP/threading is introduced | threading added | — |
| canonical-subprocess | External process / parser tests | N/A | — | — | — | No subprocess, external verifier, or textual external-log parser | — | invalidate if any child process/parser is added | subprocess/parser added | — |

## Closure

| Mandatory boundary | Valid row | Coverage status |
|---|---|---|
| Changed grouping map compiled into production source | `canonical-selftest-gcc`, `canonical-cli-n21-bound`, `canonical-cli-n22-bound` | covered |
| Build and warning hygiene | `canonical-build-gcc`, `canonical-build-clang`, `canonical-static` | covered |
| Sanitizer smoke | `canonical-asan-ubsan` | covered |
| Makefile unchanged but still production-local | `canonical-build-gcc`, `canonical-build-clang` | covered |
| Exact-candidate public `n=21` aggregate | none yet | missing; old aggregate is pre-canonical evidence |
| Exact-candidate public `n=22` aggregate | none yet | missing; campaign not started |

## Evidence reuse rule

Rows marked `V` must not be presented as exact-candidate campaign results. They
may be cited only as pre-canonical mathematical/throughput evidence unless
Carlo explicitly decides otherwise.
