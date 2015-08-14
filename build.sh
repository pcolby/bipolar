#!/bin/bash
#
# Prerequisites:
# * Xcode
#

QT_VERSION=5.1.1 # The version used by Polar FlowSync.
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
            "http://download.qt-project.org/official_releases/qt/5.1/$QT_VERSION/single/$QT_NAME.tar.gz" \
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
            "$NETWORK_ACCESS_DIR/qnetworkaccessmanager.ori"
        if [ $? -ne 0 ]; then return; fi
    fi
    echo "Applying qnetworkaccessmanager.patch"
    "$SED" -e '1,2 s/\\/\//g' "$SELF_DIR/qnetworkaccessmanager.patch" | \
        "$PATCH" --forward --strip 0
    if [ $? -eq 1 ]; then
        echo 'Assuming patch is already applied and continuing.'
    fi
}

# Configure the Qt build.
function configure {
    patchSource || return
    ${MKDIR} "$SELF_DIR/build"
    pushd "$SELF_DIR/build"
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
        -sdk macosx10.8 \
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
