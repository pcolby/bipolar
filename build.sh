#!/bin/bash
#
# Prerequisites:
# * Xcode
#

QT_VERSION=5.5.1 # The version used by Polar FlowSync.
QT_NAME="qt-everywhere-opensource-src-$QT_VERSION"

CP=`which cp`       || { echo 'Failed to find: cp'    2>&1; exit 1; }
CURL=`which curl`   || { echo 'Failed to find: curl'  2>&1; exit 1; }
MKDIR=`which mkdir` || { echo 'Failed to find: mkdir' 2>&1; exit 1; }
PATCH=`which patch` || { echo 'Failed to find: patch' 2>&1; exit 1; }
SED=`which sed`     || { echo 'Failed to find: sed'   2>&1; exit 1; }
TAR=`which tar`     || { echo 'Failed to find: tar'   2>&1; exit 1; }

SELF_DIR=`dirname "$0"`

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
        echo "Fixing pre-Qt57 xcodebuild bug"
        find "$SELF_DIR" -name 'default_pre.prf'
        cat "$SELF_DIR/$QT_NAME/qtbase/mkspecs/features/mac/default_pre.prf"
        "$SED" -i.ori -Ee 's/(-find xc)run/\1odebuild/g' "$SELF_DIR/$QT_NAME//qtbase/mkspecs/features/mac/default_pre.prf" || return
        cat "$SELF_DIR/$QT_NAME//qtbase/mkspecs/features/mac/default_pre.prf"
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
        -platform macx-clang-32 \
        -release \
        -sdk macosx10.10 \
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
