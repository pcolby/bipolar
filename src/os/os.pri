DEPENDPATH += src/os
VPATH += $$PWD

win32 {
  HEADERS += fileversioninfo.h   flowsynchook.h
  SOURCES += fileversioninfo.cpp flowsynchook.cpp
  LIBS += -lversion
  RC_FILE=qrc/Bipolar.rc
}

# Create our custom revbuild target.
win32:revbuild.commands = qrc\\gitrevision.cmd qrc\\Bipolar.rc.in qrc\\Bipolar.rc
## @todo Add Mac, Linux, etc versions here.
QMAKE_EXTRA_TARGETS += revbuild

# Hook our revbuild target in between qmake's Makefile update and the actual project target.
revbuildhook.depends = revbuild
CONFIG(debug,debug|release):revbuildhook.target = Makefile.Debug
CONFIG(release,debug|release):revbuildhook.target = Makefile.Release
QMAKE_EXTRA_TARGETS += revbuildhook
