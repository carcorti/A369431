#!/usr/bin/env python3
"""Validate the A369431 b-file and optionally compare a PermPAL raw response."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

INITIAL = [1, 1, 2, 6]
CANONICAL_C = {21: 8321204878, 22: 26674199972}


def read_bfile(path: Path) -> list[int]:
    raw = path.read_bytes()
    if not raw.endswith(b"\n\n") or raw.endswith(b"\n\n\n"):
        raise ValueError("b-file must end in exactly one blank line (two LF bytes)")
    values: list[int] = []
    # Remove only the mandatory empty final line before parsing data records.
    for expected_n, line in enumerate(raw[:-1].decode("ascii").splitlines()):
        fields = line.split()
        if len(fields) != 2 or int(fields[0]) != expected_n or not fields[1].isdigit():
            raise ValueError(f"invalid row {expected_n}: {line!r}")
        values.append(int(fields[1]))
    return values


def permpal_values(path: Path) -> list[int]:
    text = path.read_text(encoding="utf-8")
    match = re.search(r"Counting sequence:<br />\s*([0-9, ]+)<br /><br />", text)
    if match is None:
        raise ValueError("PermPAL counting sequence not found")
    return [int(field.strip()) for field in match.group(1).split(",")]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("bfile", type=Path)
    parser.add_argument("--permpal-raw", type=Path)
    args = parser.parse_args()

    values = read_bfile(args.bfile)
    if values[:4] != INITIAL:
        raise ValueError("incorrect initial values")
    for n in range(4, len(values)):
        expected = 5 * values[n - 1] - 7 * values[n - 2] + 4 * values[n - 3]
        if values[n] != expected:
            raise ValueError(f"recurrence mismatch at n={n}")
    for n, expected in CANONICAL_C.items():
        if len(values) <= n or values[n] != expected:
            raise ValueError(f"canonical C mismatch at n={n}")

    print(f"b-file: PASS ({len(values)} rows, n=0..{len(values)-1})")
    print("canonical C endpoints: PASS (n=21,22)")
    if args.permpal_raw is not None:
        external = permpal_values(args.permpal_raw)
        if values[: len(external)] != external:
            raise ValueError("PermPAL coefficient mismatch")
        print(f"PermPAL: PASS ({len(external)} coefficients, n=0..{len(external)-1})")


if __name__ == "__main__":
    main()
