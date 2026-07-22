# OEIS A369431: independent exhaustive verification and formula-derived data

This repository contains the code, data, persistent execution records,
validation reports, and paper supporting a computational study of
[OEIS A369431](https://oeis.org/A369431).  The definitive repository package
version is v1.01.

A369431 counts permutations of the set {1, 2, ..., n} that avoid all four classical patterns:

- 1234
- 1324
- 1342
- 2413

In compact notation:

> a(n) is the number of permutations of {1, 2, ..., n} avoiding 1234, 1324, 1342, and 2413.

## Main results

The independent, formula-free maximum-insertion computation gives:

- a(21) = 8,321,204,878
- a(22) = 26,674,199,972

These two values were obtained by exhaustive enumeration with the C17 program in `src/a369431.c`.

The repository also contains an OEIS-style b-file through n = 400. Its provenance is deliberately split:

- terms from n = 0 through n = 20 reproduce the pre-campaign OEIS data;
- terms n = 21 and n = 22 are independently supported by the finite exhaustive C computation and its validation artifacts;
- terms from n = 23 through n = 400 are derived from the previously published PermPAL rational generating function;
- the formula-derived extension was calculated independently with Python and PARI/GP;
- the two calculations agree for all 401 coefficients from n = 0 through n = 400;
- the first 101 coefficients also agree with the sequence displayed by PermPAL.

No priority is claimed for the generating function, recurrence, or formula-derived coefficients.

## Data files

- [`data/b369431.txt`](data/b369431.txt): OEIS-style b-file through n = 400.
- [`data/certified_terms.tsv`](data/certified_terms.tsv): terms within the finite certification boundary.
- [`results/a369431_terms.tsv`](results/a369431_terms.tsv): consolidated result table.
- [`results/a369431_n21.aggregate.tsv`](results/a369431_n21.aggregate.tsv): final aggregate for n = 21.
- [`results/a369431_n22.aggregate.tsv`](results/a369431_n22.aggregate.tsv): final aggregate for n = 22.

## Repository structure

```text
.
├── README.md
├── CITATION.cff
├── LICENSE
├── .gitignore
├── data/
├── paper/
├── results/
├── src/
├── tools/
└── validation/
```

The paper is distributed in source and rendered form:

- `paper/A369431.tex`
- `paper/A369431.pdf`

The filename is deliberately unversioned; release identity is carried by the
repository and archive metadata.

## Computational method

The exhaustive computation recursively inserts the next maximum value into an already avoiding permutation.

Because the parent permutation already avoids the four forbidden patterns, a newly created forbidden occurrence must contain the inserted maximum. The implementation therefore tests only the four possible pattern configurations involving that new maximum.

Coverage is organized as follows:

- 20 avoiding permutations of order 4 form the mathematical coverage atoms;
- their 66 valid children of order 5 form disjoint durable recovery units;
- the n = 21 campaign uses 4 operational groups;
- the n = 22 campaign uses 8 operational groups.

The grouping affects restart and persistence only. It does not change the mathematical set being counted.

## Implementation

The production implementation is:

- `src/a369431.c`
- `src/Makefile`

The program is serial and written for C17. Counts use checked unsigned 128-bit integer arithmetic supported by GCC and Clang.

The archived command interface documented by the paper is:

```bash
src/a369431 run TARGET GROUP
```

or:

```bash
src/a369431 run TARGET next
```

For the completed campaigns, starting from an empty target manifest:

```bash
src/a369431 run 21 next
```

is invoked successfully four times, and:

```bash
src/a369431 run 22 next
```

is invoked successfully eight times.

A full large campaign is distinct from the bounded validation commands below.

## Build and bounded validation

From the repository root:

```bash
make -C src clean test
make -C src sanitize
make -C src clang
make -C src static
```

These commands test the reviewed implementation using the targets defined in `src/Makefile`.

## Validation evidence

The public validation material is compact and selected. It does not claim to contain complete terminal or run-by-run logs.

Key records include:

- `validation/a369431_n21_manifest.tsv`
- `validation/a369431_n22_manifest.tsv`
- `validation/validation_summary.md`
- `validation/validation_notes.md`
- `validation/A369431_formula_derived_bfile_validation.md`
- `validation/A369431_code_validation_evidence_ledger.md`
- target-specific post-result validation reports in `validation/`

For each target, the append-only manifest contains the durable unit records. The aggregate files record the final sums and provenance identities. The validation boundary is artifact-based: it combines the mathematical coverage argument, reviewed implementation, completed manifests, aggregate equality, bounded direct checks, compiler and sanitizer runs, static analysis, and external LLM-assisted reviews.

The external LLM reviews are code and artifact reviews. They are not independent human peer review and do not constitute a second exhaustive production campaign.

## Hardware and software environment

The reported computation used:

- Linux Mint 22.3
- AMD Ryzen 9 7940HS, 8 cores / 16 threads
- 64 GB DDR5 RAM
- C17 implementation
- GCC/Clang-compatible unsigned 128-bit integer extension
- Python 3.12 for one formula-derived calculation
- PARI/GP 2.15.4 for an independent formal-series calculation

The exact production compiler version, kernel, and operator-supplied build-flag overrides were not retained. Reported elapsed times are therefore provenance telemetry, not portable performance benchmarks.

## Reproducing the formula-derived b-file

Python route:

```bash
python3 tools/generate_b369431.py
```

PARI/GP route:

```bash
gp -q tools/generate_b369431.gp
```

Validation:

```bash
python3 tools/validate_b369431.py data/b369431.txt
```

The precise command-line behavior remains defined by the scripts themselves.

## Paper

The accompanying manuscript is:

- [`paper/A369431.tex`](paper/A369431.tex)
- [`paper/A369431.pdf`](paper/A369431.pdf)

Title:

> Independent Exhaustive Verification of OEIS A369431 through n = 22 and a Formula-Derived b-File through n = 400

Author:

> Carlo Corti

The manuscript is the primary scientific description of the sequence definition, enumeration method, validation boundary, formula-derived extension, limitations, and attribution.

## Code and data availability

The repository contains the C source, Makefile, b-file candidate,
formula-generation scripts, certified-term table, result tables, append-only
manifests, aggregate records, validation reports, citation metadata, license,
and manuscript source and PDF described above.

No per-unit directory or external dataset is required to audit the reported finite computation.

The public GitHub repository is:

- https://github.com/carcorti/A369431

The archived project DOI is:

- https://doi.org/10.5281/zenodo.21485629

## License

The software and accompanying repository materials are released under the MIT License. See [`LICENSE`](LICENSE).

## Citation

Citation metadata are provided in [`CITATION.cff`](CITATION.cff).
