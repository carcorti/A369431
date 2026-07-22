# A369431 Canonical Grouping — External Review Integration

**Date:** 2026-07-20
**Artifact class:** external-review integration, not an independent peer review.
**Integrator:** Codex

## Frozen Candidate

| Artifact | SHA-256 |
|---|---|
| `src/a369431.c` | `6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64` |
| `src/Makefile` | `e0bf20962233c7c1fc8191404ceffb361755bd109175e883ff0239044e86ec24` |
| `src/a369431` | `03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143` |
| `outputs/A369431_canonical_grouping_change_report.md` | `0c9374b8438e525a6ddc79a22575ccf27cf709a903adcc05905cdc68e5253311` |
| `outputs/A369431_canonical_grouping_validation_evidence_ledger.md` | `aa76f0c236ad5600cddb273c4b9f5c39908d393a5f1196f975467f8aeda197e8` |

Integrated external reviews:

| Reviewer | File | SHA-256 | Scope / executed evidence | Result |
|---|---|---|---|---|
| ChatGPT | `src_review/08_ChatGPT_code_review.md` | `b4f6d7c061eec65a07b4ad15e25182a7dd1e1ceb70d8b7b47561b16a09c1c79a` | Targeted grouping/publication-readiness review; source/Makefile identity; GCC build; selftest; `n=21`/`n=22` invalid group rejection; static reasoning on kernel, units, schemas, resume, no-clobber, telemetry. | `READY FOR TARGETED CANONICAL RUN`; no reproducible defect. |
| Claude Pro | `src_review/08_Claude_code_review.md` | `2e6bc6c6f53d6d8d32cb407a818c15cbece460ccb21996debcf92b1190670c59` | Targeted grouping/publication-readiness review; GCC/Clang/sanitizer/cppcheck; grouped known-term checks `n=12` and `n=16`; CLI matrix; independent Python brute force through `n=9`; RSS/throughput in review environment. | `READY FOR TARGETED CANONICAL RUN`; no new defect; one inherited cosmetic item remains deferred. |

## Integrated Verdict

Both mandatory reviewers approve the frozen canonical grouping candidate.
No source or Makefile change is required.

The code-review loop for the targeted grouping/publication-readiness boundary
should stop here. The next action is an operator decision about preserving or
archiving the superseded `n=21` artifacts before any canonical rerun. Codex
must not launch campaign commands.

## Production Code Gate

```text
Production Code Gate:
- canonical artifact: PASS
- explicit user preferences: PASS
- publication-readiness: PASS for code/review identity
- validation identity stability: PASS for future canonical runs; old n=21 remains pre-canonical
- review/run freeze readiness: PASS after review 08
- decision: GO for Carlo-owned targeted canonical run, not automatic campaign execution by Codex
```

## Finding Triage

| ID | Raised by | Finding | Decision | Rationale |
|---|---|---|---|---|
| 08-1 | ChatGPT, Claude Pro | `n=21` is now 4 operational groups and `n=22` is now 8 operational groups. | Verified correct. | Both reviewers verified the map and group rejection. ChatGPT confirms `n=21` groups `0..3` and `n=22` groups `0..7`; Claude reports the same and additionally checks adjacent invalid groups for `n=23`/`n=24`. |
| 08-2 | ChatGPT, Claude Pro | Grouping change is value-invariant and does not alter the mathematical kernel or the 66 internal recovery units. | Verified correct. | Grouping only partitions existing durable units selected by `unit_in_group()`. Aggregation still sums all 66 units. Claude additionally verified grouped known terms `a(12)=232830` and `a(16)=24584436`; ChatGPT confirms kernel/schema non-interference by static source audit and selftest. |
| 08-3 | ChatGPT, Claude Pro | Prior `n=21` aggregate was produced by the superseded source/binary identity and is therefore pre-canonical evidence, not a public exact-current-candidate artifact. | Accepted. | This is a publication-identity rule, not a code defect. A clean GitHub/Zenodo/OEIS story should use fresh canonical `n=21` artifacts, or explicitly label the prior artifacts as historical/pre-canonical evidence. |
| 08-4 | Claude Pro; inherited from 07-1/06-4 | Invalid target such as `run 25 0` can print the fallback group range `0..65`. | Deferred / non-blocking. | Exit status is 2 before provenance hashing, manifest opening, counting, resume, or output. Both prior and current reviews treat this as cosmetic. It is not a reason to change the frozen hash. |

No critical, high, medium, mathematical, numeric, persistence, telemetry,
manifest, aggregate, publication-identity, or operational cadence defect is
open against the frozen candidate.

## Evidence Reuse And Invalidation

Rows in
`outputs/A369431_canonical_grouping_validation_evidence_ledger.md` remain
applicable to this exact candidate:

| Ledger row | Integration status | Invalidation audit |
|---|---|---|
| `canonical-build-gcc` | inherited and externally repeated | Source/Makefile identity unchanged. |
| `canonical-selftest-gcc` | inherited and externally repeated | Source identity unchanged; grouping selftest covers expected map. |
| `canonical-asan-ubsan` | inherited; repeated by Claude | Sanitizer boundary unchanged. |
| `canonical-build-clang` | inherited; repeated by Claude with Clang 18 substituted for unavailable Clang 17 | Source/Makefile unchanged. |
| `canonical-static` | inherited; Claude repeated cppcheck but could not run clang-tidy in his container | Codex local `clang-tidy`/`cppcheck` evidence remains the primary row; Claude's cppcheck is supplemental. |
| `canonical-cli-n21-bound` | inherited and externally repeated | Grouping map unchanged; `n=21` group 4 rejected. |
| `canonical-cli-n22-bound` | inherited and externally repeated | Grouping map unchanged; `n=22` group 8 rejected. |
| `canonical-known-n20` | remains `V` for exact-candidate campaign identity | Prior known-target aggregate is from superseded source/binary; useful as pre-canonical throughput/mathematical evidence only unless rerun. |
| `canonical-new-n21` | remains `V` for exact-candidate campaign identity | Prior new-term aggregate is from superseded source/binary; useful as pre-canonical evidence only unless Carlo chooses otherwise. |
| `canonical-tsan` | N/A | Serial implementation; invalidate if threading/OpenMP is added. |
| `canonical-subprocess` | N/A | No subprocess/external verifier; invalidate if one is added. |

No reviewer identified an invalidation trigger requiring rerun of expensive
`n=20`, `n=21`, `n=22`, or later campaign computations during review.

## Local Integration Check

Codex performed read-only identity checks plus a bounded selftest after reading
the review reports:

```bash
sha256sum src/a369431.c src/Makefile src/a369431 \
  outputs/A369431_canonical_grouping_change_report.md \
  outputs/A369431_canonical_grouping_validation_evidence_ledger.md \
  src_review/08_ChatGPT_code_review.md src_review/08_Claude_code_review.md

src/a369431 selftest
```

Observed:

```text
src/a369431.c   6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64
src/Makefile    e0bf20962233c7c1fc8191404ceffb361755bd109175e883ff0239044e86ec24
src/a369431     03add459595d2c594a7bfc8a6783c3b215d561881fef5acfe8f54d48bb34a143

A369431 selftest: roots=ok order-5-units=66 incremental-via-n=9 direct-oracle-via-n=9 insertion-replay-via-n=12 recurrence-audit(n=4..20, checksum-only)=agree
```

No local `outputs/a369431_n12.*`, `outputs/a369431_n16.*`, or
`outputs/a369431_n22.*` artifacts were present in the workspace during this
integration. Codex did not run any official campaign.

## Reviewer Panel Closure

The selected panel was the mandatory minimum, Claude Pro and ChatGPT, because
the changed trust boundary was narrow:

- fixed operational group counts;
- publication identity / canonical artifact story;
- unchanged mathematical kernel;
- unchanged manifest, aggregate, no-clobber, telemetry, and internal-unit
  semantics.

A larger panel is not justified for this exact candidate because both reviewers
approved the boundary and no reproducible defect was found. Escalate only if:

- source or Makefile changes again;
- manifest/aggregate schema changes;
- grouping map changes again;
- telemetry/resume/no-clobber semantics change;
- a canonical `n=21` or `n=22` run reveals a reproducible mismatch, manifest
  conflict, severe timing anomaly, or operator-blocking issue.

## Operational Consequence

**Code-review status:** closed for the canonical grouping candidate.

**Code / Makefile changes required now:** none.

**Canonical public artifact status:** `src/a369431.c` plus `src/Makefile` are
ready as the single production source/build pair for Carlo-owned canonical
runs.

Before a canonical `n=21` rerun, the superseded `outputs/a369431_n21.*`
artifacts must be archived or moved out of the active output names; the current
program will correctly reject them as foreign-provenance manifest rows.

The next campaign step requires Carlo's explicit operational decision. Codex
must provide commands only after that decision and must not run the campaign.

## Final Recommendation

**READY FOR TARGETED CANONICAL RUN.**

No additional external review loop is technically required for this frozen
candidate.
