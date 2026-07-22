#!/usr/bin/env python3
"""Generate formula-derived coefficients for OEIS A369431.

The recurrence and initial values are taken from PermPAL record 14688:
https://www.permpal.com/perms/basis/0123_0213_0231_1302/

This is not the formula-free exhaustive C enumerator.  In particular, terms
above n=22 produced here must not be described as exhaustive C results.
"""

from __future__ import annotations

import argparse
from pathlib import Path


def coefficients(max_n: int) -> list[int]:
    """Return a(0), ..., a(max_n) using exact Python integers."""
    if max_n < 0:
        raise ValueError("max_n must be nonnegative")
    values = [1, 1, 2, 6][: max_n + 1]
    for n in range(4, max_n + 1):
        values.append(5 * values[n - 1] - 7 * values[n - 2] + 4 * values[n - 3])
    return values


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--max-n", type=int, default=400)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    # Carlo's OEIS b-files require one real blank line after the last data row:
    # the byte stream must therefore end in exactly two consecutive LF bytes.
    rows = "".join(f"{n} {value}\n" for n, value in enumerate(coefficients(args.max_n))) + "\n"
    if args.output is None:
        print(rows, end="")
    else:
        args.output.write_text(rows, encoding="ascii")


if __name__ == "__main__":
    main()
