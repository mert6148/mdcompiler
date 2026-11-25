#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

if command -v git >/dev/null 2>&1; then
  git submodule sync --recursive 2>/dev/null || true
  git submodule update --init --recursive 2>/dev/null || true
fi

if command -v cmake >/dev/null 2>&1; then
  cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$ROOT_DIR/build" --parallel
else
  echo "Hata: cmake bulunamadi." >&2
  exit 1
fi

if command -v Rscript >/dev/null 2>&1; then
  Rscript -e 'tools::check_packages_in_dir("r", check_args=c("--no-manual"), Ncpus=1)'
fi