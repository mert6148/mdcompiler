#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX="${1:-$ROOT_DIR/dist}"

cd "$ROOT_DIR"

if ! command -v cmake >/dev/null 2>&1; then
  echo "Hata: cmake bulunamadi." >&2
  exit 1
fi

if ! command -v c++ >/dev/null 2>&1 && ! command -v g++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1; then
  echo "Hata: C++ derleyicisi bulunamadi (c++, g++ veya clang++)." >&2
  exit 1
fi

cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT_DIR/build" --parallel
cmake --install "$ROOT_DIR/build" --prefix "$PREFIX"

echo "Kurulum tamamlandi: $PREFIX"
