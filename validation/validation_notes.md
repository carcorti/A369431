# A369431 Validation Notes

This directory contains structured validation evidence for the raw
GitHub-ready A369431 package.

The production run used one canonical C17 source file and one Makefile. The
algorithm is formula-free: it counts avoiders by insertion of the next maximum.
The recurrence

`a(n) = 5 a(n-1) - 7 a(n-2) + 4 a(n-3)`

was treated only as an unproved post-hoc checksum during the canonical C
campaign.  A later literature search found the exact rational generating
function in PermPAL record 14688.  That external structural result remains
outside the historical C certificate, but it is now the explicitly attributed
generator for b-file rows `23..400`.

## Public validation files

- `a369431_n21_manifest.tsv`: append-only manifest for the canonical `n=21`
  run.
- `a369431_n22_manifest.tsv`: append-only manifest for the canonical `n=22`
  run.
- `A369431_n21_canonical_post_result_validation_Codex.md`: bounded post-result
  validation report for `a(21)`.
- `A369431_n22_canonical_post_result_validation_Codex.md`: bounded post-result
  validation report for `a(22)`.
- `A369431_code_external_review_integration_Codex.md`: integration report for
  the final external code review boundary used before the canonical run.
- `A369431_code_validation_evidence_ledger.md`: validation ledger for the
  canonical code grouping candidate.
- `A369431_formula_derived_bfile_validation.md`: independent Python/PARI-GP,
  PermPAL, endpoint, format, and provenance checks for the b-file through
  `n=400`.

Raw terminal transcripts are not treated as the archival truth boundary. The
archival evidence is the source, Makefile, b-file candidate, aggregate TSVs,
append-only manifests, validation reports, and SHA-256 hashes.

## Historical-path and status map

The dated post-result and code-review reports are preserved as historical
records, so their command transcripts retain the workspace paths that existed
when the checks were performed.  They must be interpreted through this map:

| Historical report locator | Public-package locator | Status |
|---|---|---|
| `outputs/a369431_n21.manifest.tsv` | `validation/a369431_n21_manifest.tsv` | byte-identical canonical manifest |
| `outputs/a369431_n22.manifest.tsv` | `validation/a369431_n22_manifest.tsv` | byte-identical canonical manifest |
| `outputs/a369431_n21.aggregate.tsv` | `results/a369431_n21.aggregate.tsv` | byte-identical canonical aggregate |
| `outputs/a369431_n22.aggregate.tsv` | `results/a369431_n22.aggregate.tsv` | byte-identical canonical aggregate |
| `outputs/A369431_canonical_grouping_validation_evidence_ledger.md` | `validation/A369431_code_validation_evidence_ledger.md` | byte-identical published copy |
| `src_review/...` and other `outputs/...` review/change records | no public-package file | local-only prepublication evidence; identifying SHA-256 values remain in the dated integration report |
| generated `src/a369431` binaries | no public-package file | intentionally excluded; rebuild from `src/a369431.c` and `src/Makefile` |

The Makefile hash embedded in the historical campaign reports identifies the
campaign-time Makefile.  The public `src/Makefile` differs only by normalizing
the output directory from the workspace `../outputs` to the public-package
`../results`; the source, algorithm, grouping, and result schemas are
unchanged.

`A369431_code_validation_evidence_ledger.md` and
`A369431_code_external_review_integration_Codex.md` describe the pre-run code
gate.  Statements there that the exact-current-candidate `n=21` result was
pending or that the `n=22` campaign had not started are historical, not the
current campaign status.  The later `n=21` and `n=22` post-result reports,
public manifests, aggregates, and this summary supersede those status
statements.

## Campaign boundary

The package certifies the exhaustive extension through `a(22)`. It does not
include a maximum-insertion campaign for `a(23)` or higher.  Later b-file rows
are formula-derived and must not be represented as exhaustive campaign output.
