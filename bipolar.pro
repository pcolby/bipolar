# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Add the embedded resources.
RESOURCES = qrc/app.qrc
win32:RC_FILE=qrc/Bipolar.rc

# Neaten the output directories.
CONFIG(debug,debug|release) DESTDIR = debug
CONFIG(release,debug|release) DESTDIR = release
MOC_DIR = $$DESTDIR/tmp
OBJECTS_DIR = $$DESTDIR/tmp
RCC_DIR = $$DESTDIR/tmp
UI_DIR = $$DESTDIR/tmp

# Create our custom revbuild target.
win32:revbuild.commands = qrc\\gitrevision.cmd qrc\\Bipolar.rc.in qrc\\Bipolar.rc
## @todo Add Mac, Linux, etc versions here.
QMAKE_EXTRA_TARGETS += revbuild

# Hook our revbuild target in between qmake's Makefile update and the actual project target.
revbuildhook.depends = revbuild
CONFIG(debug,debug|release):revbuildhook.target = Makefile.Debug
CONFIG(release,debug|release):revbuildhook.target = Makefile.Release
QMAKE_EXTRA_TARGETS += revbuildhook

include(src/src.pri)
