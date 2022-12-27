#!/usr/bin/env bash
#
# Prerequisites:
# * Xcode
#

set -o errexit -o noclobber -o nounset -o pipefail # or set -Ceuo pipefail
shopt -s inherit_errexit

: "${QT_VERSION:=5.15.1}" # The version used by Polar FlowSync.
: "${QT_NAME:=qt-everywhere-src-$QT_VERSION}"
: "${SCRIPT_DIR:="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"}"
: "${OUTPUT_DIR:=$SCRIPT_DIR}"

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
NETWORK_ACCESS_DIR="$OUTPUT_DIR/$QT_NAME/qtbase/src/network/access/"
[[ -e "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.ori" ]] || {
  echo 'Backing up qnetworkaccessmanager.cpp'
  "$CP" -a \
    "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.cpp" \
    "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.ori"
  echo "Applying qnetworkaccessmanager.patch"
  "$PATCH" --directory "$OUTPUT_DIR/$QT_NAME" --forward --strip 0 < "$OUTPUT_DIR/qnetworkaccessmanager.patch" || {
    rc=$?
    [[ "$rc" -eq 1 ]] || exit "$rc"
    echo 'Assuming patch is already applied and continuing'
  }
}

# Configure the Qt build.
function configure {
    patchSource || return
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
