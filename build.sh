#!/usr/bin/env bash
#
# Build a patched Qt5Network library for Bipolar. See
# https://github.com/pcolby/bipolar/wiki/Hooks for more information.
#

set -o errexit -o noclobber -o nounset -o pipefail
[[ "${RUNNER_OS:-}" == macOS ]] || shopt -s inherit_errexit

: "${QT_VERSION:=5.15.8}" # The version used by Polar FlowSync.
: "${QT_NAME:=qt-everywhere-src-$QT_VERSION}"
: "${OUTPUT_DIR:=.}"

# Equivalent to `dirname $(readlink -f $0)`, but compatible with MacOS.
function canonicalDirName {
  local path="$1"
  local dir; dir="$(cd -P "$(dirname "$path")" >/dev/null 2>&1 && pwd -P)"
  while [[ -L "$path" ]]; do
    path=$(readlink "$path")
    [[ "$path" == /* ]] || path="$dir/$path"
    dir="$(cd -P "$(dirname "$path")" >/dev/null 2>&1 && pwd -P)"
  done
  echo "$dir"
}

function require {
  local C c
  for c in "$@"; do
    local v="${c//[^[:word:]]/_}";
    if [ -v "${v^^}" ]; then continue; fi
    C=$(command -v "$c") || { echo "Required command not found: $c" >&2; exit 1; }
    declare -gr "${v^^}"="$C"
  done
}

require cp mkdir patch sed tar wget

# Fetch (if not already) and extract (if not already) the Qt source.
[[ -e "$OUTPUT_DIR/$QT_NAME" ]] || {
  qtArchive="$OUTPUT_DIR/$QT_NAME.tar.xz"
  [[ -e "$qtArchive" ]] || {
    echo "Fetching $qtArchive"
    "$WGET" -q -O "$qtArchive" \
      "https://download.qt.io/archive/qt/${QT_VERSION%.*}/${QT_VERSION}/single/${QT_NAME}.tar.xz"
  }
  echo "Extracting $qtArchive"
  "$TAR" xJf "$qtArchive" -C "$OUTPUT_DIR"
}

# Patch the source with our hook code.
networkAccessDir="$OUTPUT_DIR/$QT_NAME/qtbase/src/network/access/"
[[ -e "$networkAccessDir/qnetworkaccessmanager.ori" ]] || {
  echo 'Backing up qnetworkaccessmanager.cpp'
  "$CP" -a \
    "$networkAccessDir/qnetworkaccessmanager.cpp" \
    "$networkAccessDir/qnetworkaccessmanager.ori"
  patchFile="$(canonicalDirName "${BASH_SOURCE[0]}")/qnetworkaccessmanager.patch"
  echo "Applying $patchFile"
  "$PATCH" --directory "$OUTPUT_DIR/$QT_NAME" --forward --strip 0 < "$patchFile" || {
    rc=$?
    [[ "$rc" -eq 1 ]] || exit "$rc"
    echo 'Assuming patch is already applied and continuing'
  }
}

# Configure the Qt build.
[[ "${RUNNER_OS:-}" != macOS ]] || {
  configPlatform='macx-clang'
  configFramework='-framework'
  configSdk='macosx10.10'
}
[[ "${RUNNER_OS:-}" != Windows ]] || {
  configOpenssl='-I C:\OpenSSL-Win32\include -L C:\OpenSSL-Win32\lib'
  configPlatform='win32-msvc'
}
"$MKDIR" -p "$OUTPUT_DIR/build"
# shellcheck disable=SC2046,SC2086 # We disable globbing, and want splitting.
( set -o noglob && cd "$OUTPUT_DIR/build" && "../$QT_NAME/configure" \
  -confirm-license \
  ${configFramework:-} \
  -no-{gui,open{gl,vg},widgets} \
  -nomake examples \
  -nomake tools \
  -opensource \
  -openssl ${configOpenssl:-} \
  ${configPlatform:+-platform "$configPlatform"} \
  -release \
  ${configSdk:+-sdk "$configSdk"} \
  -shared \
  $("$SED" -Ene '/qtbase/!s/^\[submodule "(.*)"\]$/-skip \1/p ' "../$QT_NAME/.gitmodules")
)

# Build Qt.
make -C "$OUTPUT_DIR/build"
