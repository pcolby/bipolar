#!/bin/bash
#
# A really lazy script to help with post-release version number
# bumps. Note, this uses GNU sed, so use it on *nix, not OSX.
#

if [ $# -ne 2 ]; then
    echo "Usage: `basename $0` from-version to-version" 2>&1
    exit 1
fi

SRC_VERSION=( `echo "$1" | tr ',.' ' '` )
DST_VERSION=( `echo "$2" | tr ',.' ' '` )
if [[ ${#SRC_VERSION[@]} -ne 3 || ${#DST_VERSION[@]} -ne 3 ]]; then
    echo "Versions must be formatted like: 1.2.3" 2>&1
    exit 2
fi

SRC_FILTER="${SRC_VERSION[0]}([,. ]+)${SRC_VERSION[1]}([,. ]+)${SRC_VERSION[2]}([,. <]+)"
DST_FILTER="${DST_VERSION[0]}\1${DST_VERSION[1]}\2${DST_VERSION[2]}\3"

sed -i -re "s/$SRC_FILTER/$DST_FILTER/g" \
    Bipolar.rc.in \
    Info.plist.in \
    ../pkg/nsis/win32.nsi.in \
    ../pkg/osx/build.sh || exit

echo "Tip: Don't forget the SpecialBuild in Bipolar.rc.in and Info.plist.in"
