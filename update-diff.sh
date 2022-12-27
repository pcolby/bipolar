#!/usr/bin/env bash

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

require diff

echo "Updating $SCRIPT_DIR/qnetworkaccessmanager.patch"
pushd "$OUTPUT_DIR/$QT_NAME"
mv ./qtbase/src/network/access/qnetworkaccessmanager.cpp ./qtbase/src/network/access/qnetworkaccessmanager.new
mv ./qtbase/src/network/access/qnetworkaccessmanager.ori ./qtbase/src/network/access/qnetworkaccessmanager.cpp
"$DIFF" -u './qtbase/src/network/access/qnetworkaccessmanager'.{cpp,new} >| "$SCRIPT_DIR/qnetworkaccessmanager.patch" || true
mv ./qtbase/src/network/access/qnetworkaccessmanager.cpp ./qtbase/src/network/access/qnetworkaccessmanager.ori
mv ./qtbase/src/network/access/qnetworkaccessmanager.new ./qtbase/src/network/access/qnetworkaccessmanager.cpp
popd
