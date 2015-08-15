# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

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
}
win32:RC_FILE = ../qrc/Bipolar.rc

# Neaten the output directories.
CONFIG(debug,debug|release) DESTDIR = debug
CONFIG(release,debug|release) DESTDIR = release
MOC_DIR = $$DESTDIR/tmp
OBJECTS_DIR = $$DESTDIR/tmp
RCC_DIR = $$DESTDIR/tmp
UI_DIR = $$DESTDIR/tmp

include(src.pri)
