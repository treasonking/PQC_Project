#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
CMAKE_BIN="${CMAKE_BIN:-cmake}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
GENERATOR="${CMAKE_GENERATOR:-}"
CC_BIN="${CC:-}"
CTEST_BIN="${CTEST_BIN:-ctest}"

args=(-S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
if [[ -n "${GENERATOR}" ]]; then
  args+=(-G "${GENERATOR}")
fi
if [[ -n "${CC_BIN}" ]]; then
  args+=(-DCMAKE_C_COMPILER="${CC_BIN}")
fi

"${CMAKE_BIN}" "${args[@]}"
"${CMAKE_BIN}" --build "${BUILD_DIR}" -j
"${CTEST_BIN}" --test-dir "${BUILD_DIR}" --output-on-failure
"${BUILD_DIR}/pqc_cli" info --alg mlkem-ref --sig-alg mldsa-ref
"${BUILD_DIR}/pqc_cli" benchmark --alg mlkem-ref --sig-alg mldsa-ref --iterations 100 --out bench_result.csv

if command -v python3 >/dev/null 2>&1; then
  python3 bench/parse_results.py bench_result.csv | tee bench_summary.txt
elif command -v python >/dev/null 2>&1; then
  python bench/parse_results.py bench_result.csv | tee bench_summary.txt
else
  echo "[WARN] python not found; skip benchmark summary parse"
fi
