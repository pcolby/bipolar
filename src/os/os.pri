DEPENDPATH += src/os
VPATH += $$PWD

HEADERS += abstractversioninfo.h   versioninfo.h
SOURCES += abstractversioninfo.cpp

win32 {
  HEADERS += fileversioninfo.h   flowsynchook.h
  SOURCES += fileversioninfo.cpp flowsynchook.cpp
  LIBS += -lversion
}
