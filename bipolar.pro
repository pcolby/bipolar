# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Define the build user (for TCX).
win32:DEFINES += $$shell_quote(BUILD_USER='"$$(USERNAME)"')
else: DEFINES += $$shell_quote(BUILD_USER='"$$(USER)"')

# Add the embedded resources.
RESOURCES = qrc/app.qrc
macx: {
    ICON  = qrc/icon/Bipolar.icns
    QMAKE_INFO_PLIST = qrc/Info.plist
}
win32:RC_FILE = qrc/Bipolar.rc

# Neaten the output directories.
CONFIG(debug,debug|release) DESTDIR = debug
CONFIG(release,debug|release) DESTDIR = release
MOC_DIR = $$DESTDIR/tmp
OBJECTS_DIR = $$DESTDIR/tmp
RCC_DIR = $$DESTDIR/tmp
UI_DIR = $$DESTDIR/tmp

# Create our custom revbuild target.
win32:revbuild.commands = qrc\\gitrevision.cmd qrc\\Bipolar.rc.in qrc\\Bipolar.rc
else:revbuild.commands = qrc/gitrevision.sh qrc/Info.plist.in qrc/Info.plist
QMAKE_EXTRA_TARGETS += revbuild

# Hook our revbuild target in between qmake's Makefile update and the actual project target.
revbuildhook.depends = revbuild
CONFIG(debug,debug|release):revbuildhook.target = Makefile.Debug
CONFIG(release,debug|release):revbuildhook.target = Makefile.Release
macx:revbuildhook.target = Makefile
QMAKE_EXTRA_TARGETS += revbuildhook

# Fix an issue where Qt's generated Makefile does not depend on QMAKE_INFO_PLIST.
macx: {
    infobuild.target = release/Bipolar.app/Contents/Info.plist
    infobuild.depends = $$QMAKE_INFO_PLIST
    QMAKE_EXTRA_TARGETS += infobuild
}

include(src/src.pri)
