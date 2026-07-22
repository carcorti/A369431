# A369431 formula-derived b-file validation

Date: 2026-07-21

## Scope and claim boundary

This report validates the extension of `data/b369431.txt` through `n=400`.
It does **not** extend the exhaustive C campaign beyond `n=22` and does not
claim discovery of the generating function, recurrence, or coefficients.

- Rows `0..20` reproduce the local pre-campaign OEIS data.
- Rows `21..22` agree with both the canonical formula-free C aggregates and
  the formula-derived coefficients.
- Rows `23..400` are exact coefficients derived from the rational generating
  function in PermPAL record 14688.

The external source is:

`https://www.permpal.com/perms/basis/0123_0213_0231_1302/`

PermPAL reports

```text
A(x) = (x-1)(x^2-3x+1)/(4x^3-7x^2+5x-1)
```

and hence, from `n=4` onward,

```text
a(n) = 5*a(n-1) - 7*a(n-2) + 4*a(n-3),
```

with initial values `1, 1, 2, 6`.

The endpoint `n=400` is an editorial package boundary.  The denominator has
nonzero constant term, so the rational function has a unique formal expansion
at zero and the recurrence can generate every later coefficient.  Subject to
the correctness of the PermPAL generating function, there is no intrinsic
finite index limit; only computational resources and output size are practical
constraints.

## Independent exact calculations

Two locally available arbitrary-precision systems were used.

1. Python 3.12.3 iterated the recurrence with built-in exact integers using
   `tools/generate_b369431.py`.
2. PARI/GP 2.15.4 directly expanded the rational function as a formal power
   series using `tools/generate_b369431.gp`; it did not iterate the Python
   recurrence implementation.

Commands, run from the public package root:

```text
python3 tools/generate_b369431.py --max-n 400 --output /tmp/b369431.python.txt
gp -q -f tools/generate_b369431.gp > /tmp/b369431.gp.txt
cmp /tmp/b369431.python.txt /tmp/b369431.gp.txt
curl -sSL -o /tmp/permpal_14688.html https://www.permpal.com/perms/raw_data/14688
python3 tools/validate_b369431.py data/b369431.txt --permpal-raw /tmp/permpal_14688.html
```

Result: byte-for-byte equality for all 401 rows, `n=0..400`.

## External and local comparisons

The PermPAL raw-data endpoint was downloaded on 2026-07-21:

`https://www.permpal.com/perms/raw_data/14688`

Its displayed counting sequence contains 101 coefficients.  A parser
extracted those integers and compared them with the generated sequence.

- PermPAL comparison: exact equality for all `n=0..100`.
- Pre-extension b-file comparison: exact equality for all `n=0..22`.
- Canonical C aggregates: exact equality at `n=21` and `n=22`.

Representative generated values are:

```text
a(23) = 85506000010
a(24) = 274095419758
a(25) = 878631898608
a(40) = 34070992412092462900
a(50) = 3903342517082139278233270
a(100) = 77036413219819298481081548963319991342749470386772
```

The exact value `a(400)` has 202 decimal digits and is present in the
b-file.  It is not duplicated here to keep this report readable.

## Format and identity checks

The final file has 401 ASCII data rows, consecutive indices `0..400`, and
exactly two whitespace-separated decimal fields per data row.  It ends with
the mandatory real final blank line: exactly two consecutive LF bytes after
the last data row.  There is no header or internal blank line.

SHA-256 identities:

```text
data/b369431.txt                 f27fe43f6701838eaa1c3d052f428e60a6b6d9a91538d33fd84ca12410fe50e5
tools/generate_b369431.py        0634e03c3c26c23a576de4d91c5c50e7e3f423c10609665900c646a774e416ae
tools/generate_b369431.gp        7cf32fd6c1ea2511d28d1cabcc73c686a0afdcd7b3f40583501da9f53274beb9
tools/validate_b369431.py         3accebb4a8bd4114a3196546eefdf1a7c85e1876afdb6658e03352afdb5f9a48
PermPAL raw response (temporary) dd3a8328fe625bd1418df09d56026019715a50943f72f1a3b98653022422c3ab
```

The temporary downloaded response is identified for audit purposes but is
not redistributed in the package.  The canonical URL is the source of truth.

## Result

**PASS within the stated provenance boundary.**  The b-file through `n=400`
is reproducibly derived from the previously published PermPAL rational
generating function, agrees with all 101 coefficients displayed by PermPAL,
and agrees with the independent formula-free C results at `n=21,22`.  No
exhaustive enumeration above `n=22` was executed or is implied.
