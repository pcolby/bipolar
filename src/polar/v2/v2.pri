# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

INCLUDEPATH += $$PWD
VPATH += $$PWD
HEADERS += trainingsession.h
SOURCES += trainingsession.cpp

unix:LIBS += -lz
win32-g++:LIBS += -lz
