#!/bin/bash
# Builds stdc/a/libjson.a as a universal (arm64 + x86_64) libc++ archive from the
# pinned official libjson 7.6.1 release, compiled with the committed
# include/libjson headers and JSONOptions.h laid on top so the options match
# what the plugin was written against. The committed headers are not modified.
#
# The other two archives in stdc/a (libmimetic.a, libiconv.a) are already
# universal and committed, so they are not built here.
#
# Same recipe as 4d-plugin-iipsrv/scripts/build-macos-deps.sh (libjson part).
#
# usage: scripts/build-macos-deps.sh [output-dir]   (default: stdc/a)
# WORK (default build/deps) holds the download and objects; a rerun reuses them.
# Needs: Xcode command line tools, curl, unzip.

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/stdc/a}"
case "$OUT" in /*) ;; *) OUT="$ROOT/$OUT" ;; esac
WORK="${WORK:-$ROOT/build/deps}"
MIN="${MACOSX_DEPLOYMENT_TARGET:-11.5}"

JSON_URL=https://downloads.sourceforge.net/project/libjson/libjson_7.6.1.zip
JSON_SHA=07267a3951038ee2e02d26cc41bf8e275668c38f751240d3e78dc979182e7376

# keep Homebrew headers and libraries out of the build
unset CPATH LIBRARY_PATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH || true
export MACOSX_DEPLOYMENT_TARGET="$MIN"

mkdir -p "$WORK/dl" "$OUT"

# fetch <url> <sha256> <file>: download with retries, verify the checksum
fetch() {
  local f="$WORK/dl/$3" i
  if [ -f "$f" ] && echo "$2  $f" | shasum -a 256 -c - >/dev/null 2>&1; then
    return 0
  fi
  for i in 1 2 3; do
    curl -fsSL --retry 3 -o "$f.tmp" "$1" && break
    [ "$i" = 3 ] && { echo "::error::download failed: $1"; return 1; }
    echo "download $1 failed, retrying"; sleep 10
  done
  echo "$2  $f.tmp" | shasum -a 256 -c - || { echo "::error::checksum mismatch: $1"; rm -f "$f.tmp"; return 1; }
  mv "$f.tmp" "$f"
}

fetch "$JSON_URL" "$JSON_SHA" libjson_7.6.1.zip

if [ ! -f "$WORK/libjson-build/libjson.a" ]; then
  echo "::group::libjson"
  rm -rf "${WORK:?}/libjson-src"; mkdir -p "$WORK/libjson-src"
  (cd "$WORK/libjson-src" && unzip -q "$WORK/dl/libjson_7.6.1.zip")
  J="$WORK/libjson-src/libjson"
  cp -R "$ROOT/include/libjson/." "$J/"
  # JSON_MAP spells the key "const x"; with x = void * that is "const void *", not the
  # map's "void * const", which current libc++ rejects. "x const" is the intended type.
  sed -i "" -e 's/std::pair<const x, y>/std::pair<x const, y>/' "$J/_internal/Source/JSONDefs.h"
  grep -q 'std::pair<x const, y>' "$J/_internal/Source/JSONDefs.h"
  rm -rf "${WORK:?}/libjson-build"; mkdir -p "$WORK/libjson-build"
  for f in "$J"/_internal/Source/*.cpp; do
    clang++ -arch arm64 -arch x86_64 -mmacosx-version-min="$MIN" -O2 -DNDEBUG -fPIC \
      -Werror=implicit-function-declaration \
      -I"$J" -c "$f" -o "$WORK/libjson-build/$(basename "${f%.cpp}").o"
  done
  # Apple's libtool archives fat objects directly
  libtool -static -o "$WORK/libjson-build/libjson.a" "$WORK"/libjson-build/*.o
  echo "::endgroup::"
fi
cp "$WORK/libjson-build/libjson.a" "$OUT/libjson.a"

for A in arm64 x86_64; do
  lipo "$OUT/libjson.a" -verify_arch "$A" || { echo "::error::libjson.a lacks $A"; exit 1; }
done
echo "libjson.a: $(lipo -archs "$OUT/libjson.a")"
