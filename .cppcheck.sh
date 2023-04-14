#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

cppcheck --version
RC=0
for platform in native unix{32,64} win{32{A,W},64} unspecified; do
  echo "$platform:"
  cppcheck -DBUILD_USER=test --enable=all --error-exitcode=1 \
           -I{src,src/{polar/v2,protobuf,threads,widgets},test} \
           --language=c++ --platform="$platform" --std=c++{03,11,14,17,20} \
           --suppress={missingIncludeSystem,unusedFunction} \
           --quiet "$@" src test | grep -Eve '^Checking' | uniq --skip-fields=1
  RC=$((RC+$?))
done
exit $RC
