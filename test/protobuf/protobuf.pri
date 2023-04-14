# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

VPATH += $$PWD
HEADERS += testfixnum.h   testmessage.h   testvarint.h
SOURCES += testfixnum.cpp testmessage.cpp testvarint.cpp

include(../../src/protobuf/protobuf.pri)
