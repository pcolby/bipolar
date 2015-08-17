# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

# Define the application's version.
VERSION = 0.4.1
# Don't forget to update to pkg/* files too; @todo Automate this.
if (isEmpty(VER_BUILD)): VER_BUILD = $$(APPVEYOR_BUILD_NUMBER)
if (isEmpty(VER_BUILD)): VER_BUILD = 0
if (isEmpty(VER_PAT)): {
    # Qmake only automatically assigns VER_PAT on some platforms.
    VER_PARTS = $$split(VERSION, .)
    VER_PAT = $$member(VER_PARTS, 2)
}
win32:VERSION = $$VERSION"."$$VER_BUILD

# The following 'special build' gets included in the application's resources
# at build time (via Bipolar.rc[.in] for Windows, and Info.plist[.in] for OSX).
# It is then embedded (if set) at runtime into generated TCX files. So, if set,
# SPECIAL_BUILD should be one of: Internal, Alpha, Beta, or Release.
SPECIAL_BUILD = Internal
# Don't forget to update to win32.nsi.in too; @todo Automate this.

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Define the build user (for TCX).
win32:DEFINES += BUILD_USER=$$shell_quote($$(USERNAME))
else: DEFINES += BUILD_USER=$$shell_quote($$(USER))

# Add the embedded resources.
RESOURCES = ../qrc/app.qrc
macx: {
    ICON  = $$PWD/../qrc/icon/Bipolar.icns
    QMAKE_INFO_PLIST = $$OUT_PWD/../qrc/Info.plist
    QMAKE_SUBSTITUTES += ../qrc/Info.plist.in
}
win32: {
    RC_FILE = $$OUT_PWD/../qrc/Bipolar.rc
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
