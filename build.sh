#!/usr/bin/env bash
#
# Prerequisites:
# * Xcode
#

set -o errexit -o noclobber -o nounset -o pipefail # or set -Ceuo pipefail
shopt -s inherit_errexit

: "${QT_VERSION:=5.15.1}" # The version used by Polar FlowSync.
: "${QT_NAME:=qt-everywhere-src-$QT_VERSION}"
: "${OUTPUT_DIR:=.}"

# Equivalent to `dirname $(readlink -f $0)`, but compatible with MacOS.
function canonicalDirName {
  local path="$1"
  local dir="$(cd -P "$(dirname "$path")" >/dev/null 2>&1 && pwd -P)"
  while [[ -L "$path" ]]; do
    path=$(readlink "$path")
    [[ "$path" == /* ]] || path="$dir/$path"
    dir="$(cd -P "$(dirname "$path")" >/dev/null 2>&1 && pwd -P)"
  done
  echo "$dir"
}

function require {
  local C
  for c in "$@"; do
    local -u V="${c//[^[:word:]]/_}";
    if [ -v "$V" ]; then continue; fi
    C=$(command -v "$c") || { echo "Required command not found: $c" >&2; exit 1; }
    declare -gr "$V"="$C"
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

exit

# Configure the Qt build.
function configure {
    ${MKDIR} "$SELF_DIR/build"
    pushd "$SELF_DIR/build"
    "../$QT_NAME/configure" \
        -confirm-license \
        -framework \
        -no-gui \
        -no-opengl \
        -no-widgets \
        -nomake examples \
        -nomake tools \
        -opensource \
        -openssl \
        -platform macx-clang \
        -release \
        -sdk "${MACOS_SDK:-macosx10.10}" \
        -shared \
        -skip multimedia \
        -skip script \
        -skip svg \
        -skip webkit ; RC=$?
    popd
    return $RC
}

# Build Qt.
function build {
    configure || return
    pushd "$SELF_DIR/build"
    make module-qtbase ; RC=$?
    popd
    return $RC
}

build
