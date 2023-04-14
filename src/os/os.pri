# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

INCLUDEPATH += $$PWD
VPATH += $$PWD

HEADERS += abstractversioninfo.h   versioninfo.h
SOURCES += abstractversioninfo.cpp

macx {
    HEADERS += bundleinfo.h
    SOURCES += bundleinfo.cpp
    INCLUDEPATH += /System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
    LIBS += -framework Cocoa
}

win32 {
    HEADERS += fileversioninfo.h   flowsynchook.h
    SOURCES += fileversioninfo.cpp flowsynchook.cpp
    LIBS += -lversion
}
