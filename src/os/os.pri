DEPENDPATH += src/os
VPATH += $$PWD

win32 {
  HEADERS += fileversioninfo.h   flowsynchook.h
  SOURCES += fileversioninfo.cpp flowsynchook.cpp
  LIBS += -lversion
}
