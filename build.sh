#!/usr/bin/env bash
# يبني build/editor.exe لويندوز 64 بت باستخدام mingw-w64، مع ربط Scintilla ربطًا ثابتًا
set -euo pipefail
cd "$(dirname "$0")"

PREFIX="${MINGW_PREFIX:-x86_64-w64-mingw32}"
CC="${CC:-${PREFIX}-gcc-posix}"
CXX="${CXX:-${PREFIX}-g++-posix}"
SCI=third_party/scintilla
SCILIB="$SCI/bin/libscintilla.a"

# ابنِ مكتبة Scintilla الثابتة إن لم تكن موجودة
if [ ! -f "$SCILIB" ]; then
    ./scripts/build-scintilla.sh
fi

mkdir -p build
"$CC" -municode -O2 -Wall -Wextra -I "$SCI/include" \
    -c src/main.c -o build/main.o

# الربط بـ g++ لأن Scintilla مكتوبة بـ C++؛ و -static يدمج libstdc++ و libgcc و winpthread
# في الملف التنفيذي فلا يحتاج أي DLL غير مكتبات ويندوز
"$CXX" -municode -mwindows -static -s \
    -o build/editor.exe build/main.o "$SCILIB" \
    -lgdi32 -luser32 -limm32 -lole32 -luuid -loleaut32 -ladvapi32 -lmsimg32

echo "Built: build/editor.exe"
