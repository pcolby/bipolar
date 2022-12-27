#!/usr/bin/env bash
#
# Prerequisites:
# * Xcode
#

set -o errexit -o noclobber -o nounset -o pipefail # or set -Ceuo pipefail
shopt -s inherit_errexit

: "${QT_VERSION:=5.15.1}" # The version used by Polar FlowSync.
: "${QT_NAME:=qt-everywhere-opensource-src-$QT_VERSION}"

function require {
  local C
  for c in "$@"; do
    local -u V="${c//[^[:word:]]/_}";
    if [ -v "$V" ]; then continue; fi
    C=$(command -v "$c") || { echo "Required command not found: $c" >&2; exit 1; }
    declare -gr "$V"="$C"
  done
}

require cp curl mkdir patch sed tar

SELF_DIR="$(dirname "${BASH_SOURCE[0]}")"

# Fetch the Qt source, if not already present.
function fetchSource {
    if [ ! -e "$SELF_DIR/$QT_NAME.tar.gz" ]; then
        echo "Fetching $QT_NAME.tar.gz"
        "$CURL" --location \
            "http://download.qt.io/archive/qt/5.5/$QT_VERSION/single/$QT_NAME.tar.gz" \
            --output "$QT_NAME.tar.gz"
    fi
}

# Extract source, if not already extracted.
function extractSource {
    if [ ! -e "$SELF_DIR/$QT_NAME" ]; then
        fetchSource || return
        echo "Extracting $QT_NAME.tar.gz"
        "$TAR" xzf "$SELF_DIR/$QT_NAME.tar.gz" -C "$SELF_DIR" || return
    fi
}

# Patch the source with our hook code.
function patchSource {
    extractSource || return
    NETWORK_ACCESS_DIR="$SELF_DIR/$QT_NAME/qtbase/src/network/access/"
    if [ ! -e "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.ori" ]; then
        echo "Backing up $NETWORK_ACCESS_DIR/qnetworkaccessmanager.cpp"
        "$CP" -a \
            "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.cpp" \
            "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.ori" || return
    fi
    echo "Applying qnetworkaccessmanager.patch"
    "$SED" -e '1,2 s/\\/\//g' "$SELF_DIR/qnetworkaccessmanager.patch" | \
        "$PATCH" --directory "$SELF_DIR/$QT_NAME" --forward --strip 0 ; RC=$?
    if [ $RC -eq 1 ]; then
        echo 'Assuming patch is already applied and continuing.'
        return 0
    fi
    return $RC
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
