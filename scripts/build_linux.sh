#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
CMAKE_BIN="${CMAKE_BIN:-cmake}"

"${CMAKE_BIN}" -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
"${CMAKE_BIN}" --build "${BUILD_DIR}" -j
