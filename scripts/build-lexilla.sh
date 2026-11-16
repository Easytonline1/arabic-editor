#!/usr/bin/env bash
# Build Lexilla's static library (third_party/lexilla/bin/liblexilla.a) for Windows x64 with
# mingw-w64, following third_party/lexilla/README ("run lexilla/src/makefile: make").
# Lexilla expects Scintilla as a peer directory (third_party/scintilla) for its headers.
set -euo pipefail
cd "$(dirname "$0")/../third_party/lexilla/src"
PREFIX="${MINGW_PREFIX:-x86_64-w64-mingw32}"
make -j"$(nproc)" \
    CXX="${PREFIX}-g++-posix" \
    AR="${PREFIX}-ar" \
    RANLIB="${PREFIX}-ranlib" \
    ../bin/liblexilla.a
echo "Built: third_party/lexilla/bin/liblexilla.a"
