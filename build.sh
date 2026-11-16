#!/usr/bin/env bash
# يبني build/editor.exe لويندوز 64 بت باستخدام mingw-w64
set -euo pipefail
cd "$(dirname "$0")"
CC="${CC:-x86_64-w64-mingw32-gcc}"
mkdir -p build
"$CC" -municode -mwindows -O2 -Wall -Wextra -s \
    -o build/editor.exe src/main.c
echo "Built: build/editor.exe"
