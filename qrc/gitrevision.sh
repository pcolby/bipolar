#!/bin/bash

if [ $# -ne 3 ]; then
    echo "Usage: `basename $0` git_dir source_file destination_file" 2>&1
    exit 1
fi

REVISION=`git --git-dir "$1" rev-list --count HEAD`
if [ $? -ne 0 ]; then exit; fi

sed -e "s/\(\([ 0-9]\{1,\}[.,]\)\{3\} *\)0/\1$REVISION/g" "$2" > "$2.tmp"
if [ $? -ne 0 ]; then exit; fi

diff -qs "$2.tmp" "$3"
if [ $? -ne 0 ]; then mv "$2.tmp" "$3"; else rm -f "$2.tmp"; fi
