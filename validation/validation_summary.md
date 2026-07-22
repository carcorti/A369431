# A369431 Validation Summary

Date: 2026-07-22

## Scope

This summary covers definitive repository release v1.01 for OEIS A369431,
published at `https://github.com/carcorti/A369431` and archived under
`https://doi.org/10.5281/zenodo.21485629`. The exhaustive C artifacts certify:

```text
a(21) = 8321204878
a(22) = 26674199972
```

No exhaustive campaign for `a(23)` or higher is included.  Separately,
`data/b369431.txt` now contains formula-derived coefficients through `n=400`,
using the rational generating function previously published in PermPAL record
14688.  See `A369431_formula_derived_bfile_validation.md`.

## Method

The production program counts avoiders by formula-free insertion of the next
maximum. From an avoiding permutation of order `m`, it inserts `m+1` in each
valid position. The incremental predicate checks only new forbidden
occurrences containing the inserted maximum. No generic pattern parser and no
recurrence generator are used.

## Production artifacts

```text
src/a369431.c
src/Makefile
data/b369431.txt
data/certified_terms.tsv
tools/generate_b369431.py
tools/generate_b369431.gp
tools/validate_b369431.py
results/a369431_terms.tsv
results/a369431_n21.aggregate.tsv
results/a369431_n22.aggregate.tsv
validation/a369431_n21_manifest.tsv
validation/a369431_n22_manifest.tsv
validation/A369431_formula_derived_bfile_validation.md
paper/A369431.tex
paper/A369431.pdf
```

## Final aggregate

`results/a369431_n22.aggregate.tsv` records:

```text
coverage = groups=8/8;units=66/66;atoms=20/20
leaves   = 26674199972
nodes    = 38768219576
method   = maximum-insertion
```

## Validation checks

The package-level checks include:

- C17 build and selftest from `src`;
- ASan/UBSan selftest from `src`;
- Clang build and selftest when Clang is available;
- static-analysis target when `clang-tidy` and `cppcheck` are available;
- historical b-file equality against `results/a369431_terms.tsv` through
  `n=22` at campaign closure;
- manifest completion and aggregate equality for `n=22`;
- bounded source-level selftest;
- current b-file format, consecutive indices, recurrence, and canonical
  endpoint checks through `n=400`;
- byte-for-byte equality of Python recurrence output and PARI/GP direct formal
  series expansion through `n=400`;
- equality with the 101 displayed PermPAL coefficients through `n=100`.
- two-pass `pdflatex` compilation of `paper/A369431.tex`, followed by
  log, PDF text, metadata, and page-boundary inspection.

Formula-data commands, shown from the public repository root:

```bash
python3 tools/generate_b369431.py --max-n 400 --output /tmp/b369431.python.txt
gp -q -f tools/generate_b369431.gp > /tmp/b369431.gp.txt
cmp /tmp/b369431.python.txt /tmp/b369431.gp.txt
curl -sSL -o /tmp/permpal_14688.html https://www.permpal.com/perms/raw_data/14688
python3 tools/validate_b369431.py data/b369431.txt --permpal-raw /tmp/permpal_14688.html
```

The dated post-result reports retain their original parent-workspace command
paths as historical transcripts.  The public-path mapping and the distinction
between pre-run and post-result status are documented in
`validation/validation_notes.md`.

Observed results:

```text
Current GCC/Clang/sanitizer/static-analysis checks: passed
b-file format and consecutive indices: passed for n=0..400
b-file final blank line: exactly two consecutive LF bytes (0a 0a)
Python/PARI-GP equality: passed for n=0..400
PermPAL displayed-sequence equality: passed for n=0..100
canonical C endpoint equality: passed for n=21,22
n=22 manifest complete units: 66
n=22 aggregate equals complete manifest sum: yes
final PDF: 13 pages; no TeX warnings; References begin on page 12
```

## Checksum table

If any listed file is edited, rerun the checksum sweep and replace this table.
The checksum of this summary itself is intentionally reported by the preparing
agent outside the table to avoid self-reference.

```text
fdae2c68dff01fdb1cec5251851687f0796c0581ec150b553a304ed5d3a9ed63  .gitignore
e5e8422cd43c1ea8d110f8b3d6e2e8a349080e2f0e20c1acb4ef609ecef120e4  CITATION.cff
1db7ea53590c61186548979643d1892af163e23c8254a198be6b1e50ae189911  LICENSE
17388614449a59cfc694948ade3963e12d6818ff408d12e65c631bbbfb878c8b  README.md
f27fe43f6701838eaa1c3d052f428e60a6b6d9a91538d33fd84ca12410fe50e5  data/b369431.txt
b2ca2a31e89a662c2d36d0b84bdcec437689d206f93c2c2f59ca01043b45194f  data/certified_terms.tsv
80937ed55c952cce040de831952944e1d93b36f417a947660b35fd9b208ed7fc  paper/A369431.pdf
e335e284a30e2534d515c4a0525f5903ba76dfa98cc524c9c2fbfbb0d2d48224  paper/A369431.tex
6b892e47e1a426b0a5de40237e5e4686c5915c1195a73679afb81e7186cc91e8  results/a369431_n21.aggregate.tsv
24e595461e9b07f20d69536c5464cc71ca8b4cf0c58a61c1eb74c0c6b8c308e9  results/a369431_n22.aggregate.tsv
696f3aaf574304d8ef4c02876a95361a5b5ea09d6af30f02aeb35683cf81a059  results/a369431_terms.tsv
480e1b9733ee12d977ef174f9a970e435ed9f3f97191e2e330ed4368a6026498  src/Makefile
6b167caa975bc7fddd0d7c090ad84dfd68ec6538c438c68b9ef65050ab0cdf64  src/a369431.c
7cf32fd6c1ea2511d28d1cabcc73c686a0afdcd7b3f40583501da9f53274beb9  tools/generate_b369431.gp
0634e03c3c26c23a576de4d91c5c50e7e3f423c10609665900c646a774e416ae  tools/generate_b369431.py
3accebb4a8bd4114a3196546eefdf1a7c85e1876afdb6658e03352afdb5f9a48  tools/validate_b369431.py
027e2193677bd76a3781f929f1d05bddfbeaf697dc822e6dde88f2838bb3e3c9  validation/A369431_code_external_review_integration_Codex.md
aa76f0c236ad5600cddb273c4b9f5c39908d393a5f1196f975467f8aeda197e8  validation/A369431_code_validation_evidence_ledger.md
bb35a617229f612b125c943475a44e47d1cb940965fad684cb8704ab73be5f7f  validation/A369431_formula_derived_bfile_validation.md
7a4f84dcacbf33e8d7059ee75e25d3766a320964ca0ef5748d243511601e1703  validation/A369431_n21_canonical_post_result_validation_Codex.md
71cdf2d85526b2f4fe74dd174f13b62d22371778427ca51963dbb2b1285947ca  validation/A369431_n22_canonical_post_result_validation_Codex.md
29a8bafe4a4c95692a9107e875759ee6168dbd92096fbbcef83e6cdf25a2f248  validation/a369431_n21_manifest.tsv
2345855e6ce20b871d218e85d0327c291606e4300ae31e626766501947ed903d  validation/a369431_n22_manifest.tsv
647b21d3338a12fbb9218337a78c29876bb754f9c813a8232a36a108b38f4583  validation/validation_notes.md
```
