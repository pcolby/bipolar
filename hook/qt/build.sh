#!/bin/bash

QT_VERSION=5.1.1 # The version used by Polar FlowSync.
QT_NAME="qt-everywhere-opensource-src-$QT_VERSION"

CURL=`which curl`   || { echo 'Failed to find: curl'  2>&1; exit 1; }
PATCH=`which patch` || { echo 'Failed to find: patch' 2>&1; exit 1; }
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
        "$TAR" xzf "$SELF_DIR/$QT_NAME.tar.gz" -C "$SELF_DIR" || return
    fi
}

# Patch the source with our hook code.
function patchSource {
    extractSource || return
    # @todo "$PATCH" ...
}

# Configure the Qt build.
function configure {
    patchSource || return
    # @todo configure
}

# Build Qt.
function build {
    configure || return
    # @todo make
}

build
