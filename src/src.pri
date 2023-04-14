# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

INCLUDEPATH += $$PWD
VPATH += $$PWD
SOURCES += main.cpp
include(os/os.pri)
include(polar/polar.pri)
include(protobuf/protobuf.pri)
include(threads/threads.pri)
include(widgets/widgets.pri)
