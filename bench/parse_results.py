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
import csv
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


def parse_csv(path: Path) -> dict[str, str]:
    with path.open("r", encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f))
    if not rows:
        return {}
    return {k: str(v) for k, v in rows[-1].items()}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path, help="benchmark raw text file")
    args = parser.parse_args()

    if args.input.suffix.lower() == ".csv":
        kv = parse_csv(args.input)
    else:
        kv = parse_kv_lines(args.input.read_text(encoding="utf-8"))

    iterations = kv.get("iterations", kv.get("benchmark iterations", "N/A"))
    print("Benchmark Summary")
    print(f"- iterations: {iterations}")
    print(f"- algorithm: {kv.get('algorithm', 'N/A')}")
    print(f"- public_key_size: {kv.get('public_key_size', 'N/A')}")
    print(f"- secret_key_size: {kv.get('secret_key_size', 'N/A')}")
    print(f"- ciphertext_size: {kv.get('ciphertext_size', 'N/A')}")
    print(f"- shared_secret_size: {kv.get('shared_secret_size', 'N/A')}")
    print(f"- avg_keygen_ms: {kv.get('avg_keygen_ms', 'N/A')}")
    print(f"- avg_encaps_ms: {kv.get('avg_encaps_ms', 'N/A')}")
    print(f"- avg_decaps_ms: {kv.get('avg_decaps_ms', 'N/A')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
