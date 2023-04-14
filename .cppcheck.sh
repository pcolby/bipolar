#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

RC=0
for platform in unix{32,64} win{32{A,W},64}; do
  echo "$platform:"
  cppcheck -DBUILD_USER=test --enable=all --error-exitcode=1 \
           -I{src,src/{polar/v2,protobuf,threads,widgets},test} \
           --platform=$platform --suppress={missingIncludeSystem,unusedFunction} \
           --std=c++{03,11,14,17,20} --quiet .
  RC=$((RC+$?))
done
exit $RC
