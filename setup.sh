#!/usr/bin/env bash
# يثبت Ghidra 12.1.4 و REA (rea-agents@6.9.0) و mingw-w64 و Wine على شاشة Xvfb الافتراضية.
# الهدف: الانتهاء في أقل من 5 دقائق (التنزيلات وتثبيت الحزم تعمل بالتوازي).
set -euo pipefail

GHIDRA_VERSION=12.1.4
GHIDRA_DATE=20260921
GHIDRA_ZIP="ghidra_${GHIDRA_VERSION}_PUBLIC_${GHIDRA_DATE}.zip"
GHIDRA_URL="https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_${GHIDRA_VERSION}_build/${GHIDRA_ZIP}"
GHIDRA_INSTALL_DIR="/opt/ghidra_${GHIDRA_VERSION}_PUBLIC"
REA_VERSION=6.9.0
DISPLAY_NUM=99

START=$(date +%s)
log() { printf '[setup %3ss] %s\n' "$(( $(date +%s) - START ))" "$*"; }

if [ "$(id -u)" -ne 0 ]; then
  echo "شغّل السكربت كـ root: sudo bash setup.sh" >&2
  exit 1
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
export DEBIAN_FRONTEND=noninteractive

# 1) تنزيل Ghidra في الخلفية بينما تُثبَّت الحزم
if [ ! -x "$GHIDRA_INSTALL_DIR/support/analyzeHeadless" ]; then
  log "تنزيل Ghidra ${GHIDRA_VERSION} في الخلفية"
  curl -fsSL --retry 3 -o "$TMP/$GHIDRA_ZIP" "$GHIDRA_URL" &
  GHIDRA_PID=$!
else
  log "Ghidra موجود مسبقًا في $GHIDRA_INSTALL_DIR"
  GHIDRA_PID=
fi

# 2) تثبيت REA من npm في الخلفية
log "تثبيت rea-agents@${REA_VERSION} في الخلفية"
npm install -g --no-fund --no-audit "rea-agents@${REA_VERSION}" >"$TMP/npm.log" 2>&1 &
NPM_PID=$!

# 3) حزم النظام: JDK 21 لـ Ghidra، و mingw-w64، و Wine (64 بت)، و Xvfb، و procps لـ REA
log "تثبيت حزم apt"
apt-get update -qq
apt-get install -y -qq --no-install-recommends \
  openjdk-21-jdk-headless unzip procps xvfb xauth \
  mingw-w64 wine wine64 >"$TMP/apt.log" 2>&1 || { cat "$TMP/apt.log"; exit 1; }
log "اكتمل apt"

# 4) فك Ghidra
if [ -n "$GHIDRA_PID" ]; then
  wait "$GHIDRA_PID"
  log "فك Ghidra إلى /opt"
  unzip -q -o "$TMP/$GHIDRA_ZIP" -d /opt
fi
test -x "$GHIDRA_INSTALL_DIR/support/analyzeHeadless"

wait "$NPM_PID" || { cat "$TMP/npm.log"; exit 1; }
log "اكتمل تثبيت REA: $(command -v rea)"

# 5) متغيرات البيئة الدائمة
cat >/etc/profile.d/rea-ghidra.sh <<EOF
export GHIDRA_INSTALL_DIR=$GHIDRA_INSTALL_DIR
export DISPLAY=:$DISPLAY_NUM
export WINEDEBUG=-all
EOF

# 6) شاشة Xvfb الافتراضية :99 و تهيئة Wine عليها
if ! pgrep -f "Xvfb :$DISPLAY_NUM" >/dev/null; then
  log "تشغيل Xvfb على :$DISPLAY_NUM"
  nohup Xvfb ":$DISPLAY_NUM" -screen 0 1280x1024x24 -nolisten tcp >/tmp/xvfb.log 2>&1 &
  for _ in $(seq 1 50); do [ -e "/tmp/.X11-unix/X$DISPLAY_NUM" ] && break; sleep 0.1; done
fi
export DISPLAY=":$DISPLAY_NUM" WINEDEBUG=-all
log "تهيئة Wine (wineboot)"
timeout 120 wineboot --init >/dev/null 2>&1 || log "تحذير: wineboot لم ينته بنجاح"
wineserver -w 2>/dev/null || true

# 7) تحقق سريع
log "التحقق"
x86_64-w64-mingw32-gcc --version | head -1
wine --version
cat >"$TMP/hello.c" <<'EOF'
#include <stdio.h>
int main(void) { puts("hello from wine"); return 0; }
EOF
x86_64-w64-mingw32-gcc -o "$TMP/hello.exe" "$TMP/hello.c"
wine "$TMP/hello.exe"
rea --version || true
GHIDRA_INSTALL_DIR="$GHIDRA_INSTALL_DIR" rea doctor --json >"$TMP/doctor.json" 2>/dev/null || true
if grep -Eq '"providerVersion": *"'"$GHIDRA_VERSION"'"' "$TMP/doctor.json"; then
  log "rea doctor: مزود Ghidra ${GHIDRA_VERSION} متاح"
else
  log "تحذير: rea doctor لم يجد Ghidra (راجع: rea doctor --json)"
fi

log "انتهى الإعداد. GHIDRA_INSTALL_DIR=$GHIDRA_INSTALL_DIR  DISPLAY=:$DISPLAY_NUM"
