#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: `basename $0` source_file destination_file" 2>&1
    exit 1
fi

REVISION=`git rev-list --count HEAD`
if [ $? -ne 0 ]; then exit; fi

sed -e "s/\(\([ 0-9]\{1,\}[.,]\)\{3\} *\)0/\1$REVISION/g" "$1" > "$1.tmp"
if [ $? -ne 0 ]; then exit; fi

diff -qs "$1.tmp" "$2"
if [ $? -ne 0 ]; then mv "$1.tmp" "$2"; else rm -f "$1.tmp"; fi
