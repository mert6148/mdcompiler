#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Kullanim: ./export-to-directory.sh <hedef-klasor>" >&2
  exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="$(cd "$(dirname "$1")" 2>/dev/null && pwd)/$(basename "$1")"
mkdir -p "$TARGET_DIR"

cp -R \
  "$ROOT_DIR/CMakeLists.txt" \
  "$ROOT_DIR/src" \
  "$ROOT_DIR/examples" \
  "$ROOT_DIR/r" \
  "$ROOT_DIR/README.md" \
  "$ROOT_DIR/LICENSE" \
  "$TARGET_DIR/"

echo "Dosyalar aktarildi: $TARGET_DIR"