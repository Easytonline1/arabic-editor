#!/usr/bin/env bash
# Build Scintilla's static library (third_party/scintilla/bin/libscintilla.a) for Windows x64
# with mingw-w64, following third_party/scintilla/README ("cd scintilla/win32; mingw32-make").
# Here we cross-compile from Linux, so the tool names are passed to make explicitly.
set -euo pipefail
cd "$(dirname "$0")/../third_party/scintilla/win32"
PREFIX="${MINGW_PREFIX:-x86_64-w64-mingw32}"
make -j"$(nproc)" \
    CXX="${PREFIX}-g++-posix" \
    AR="${PREFIX}-ar" \
    RANLIB="${PREFIX}-ranlib" \
    WINDRES="${PREFIX}-windres" \
    ../bin/libscintilla.a
echo "Built: third_party/scintilla/bin/libscintilla.a"
