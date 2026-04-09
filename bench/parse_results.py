#!/usr/bin/env python3
"""
Simple parser for pqc_cli benchmark output.

Input example lines:
benchmark iterations=1000
avg_keygen_ms=0.123
avg_encaps_ms=0.456
avg_decaps_ms=0.789
"""

from __future__ import annotations

import argparse
from pathlib import Path


def parse_kv_lines(text: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in text.splitlines():
        line = line.strip()
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        out[key.strip()] = value.strip()
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="benchmark raw text file")
    args = parser.parse_args()

    kv = parse_kv_lines(args.input.read_text(encoding="utf-8"))
    print("Benchmark Summary")
    print(f"- iterations: {kv.get('benchmark iterations', 'N/A')}")
    print(f"- avg_keygen_ms: {kv.get('avg_keygen_ms', 'N/A')}")
    print(f"- avg_encaps_ms: {kv.get('avg_encaps_ms', 'N/A')}")
    print(f"- avg_decaps_ms: {kv.get('avg_decaps_ms', 'N/A')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
