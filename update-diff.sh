#!/usr/bin/env bash

set -o errexit -o noclobber -o nounset -o pipefail
shopt -s inherit_errexit

: "${QT_VERSION:=5.15.2}" # The version used by Polar FlowSync.
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

for patchName in qnetworkaccessmanager winmakefile; do
  patchFile="$SCRIPT_DIR/$patchName.patch"
  echo "Updating $patchFile"
  pushd "$OUTPUT_DIR/$QT_NAME" > /dev/null
  targetFile="$(sed -Ene '1s/^---[[:blank:]]+([^[:blank:]]+)[[:blank:]].*$/\1/p' "$patchFile")"
  for fileName in "$targetFile" "${targetFile%.*}.ori"; do
    [[ -s "$fileName" ]] || { echo "File does not exist, or is empty: $fileName" >&2; exit 1; }
  done
  mv "$targetFile" "${targetFile%.*}.new"
  mv "${targetFile%.*}.ori" "$targetFile"
  "$DIFF" -u "${targetFile%.*}".{cpp,new} >| "$patchFile" || true
  mv "$targetFile" "${targetFile%.*}.ori"
  mv "${targetFile%.*}.new" "$targetFile"
  popd > /dev/null
done
