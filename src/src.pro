# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

VERSION = 0.4.1
if (isEmpty(VER_BUILD)): VER_BUILD = $$(APPVEYOR_BUILD_NUMBER)
if (isEmpty(VER_BUILD)): VER_BUILD = 0
win32:VERSION = $$VERSION"."$$VER_BUILD

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Define the build user (for TCX).
win32:DEFINES += $$shell_quote(BUILD_USER=$$(USERNAME))
else: DEFINES += $$shell_quote(BUILD_USER=$$(USER))

# Add the embedded resources.
RESOURCES = ../qrc/app.qrc
macx: {
    ICON  = $$PWD/../qrc/icon/Bipolar.icns
    QMAKE_INFO_PLIST = $$PWD/../qrc/Info.plist
    QMAKE_SUBSTITUTES += ../qrc/Info.plist.in
}
win32: {
    RC_FILE = ../qrc/Bipolar.rc
    QMAKE_SUBSTITUTES += ../qrc/Bipolar.rc.in
}

# Neaten the output directories.
CONFIG(debug,debug|release) DESTDIR = debug
CONFIG(release,debug|release) DESTDIR = release
MOC_DIR = $$DESTDIR/tmp
OBJECTS_DIR = $$DESTDIR/tmp
RCC_DIR = $$DESTDIR/tmp
UI_DIR = $$DESTDIR/tmp

include(src.pri)
