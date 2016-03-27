INCLUDEPATH += $$PWD
VPATH += $$PWD
HEADERS += trainingsession.h
SOURCES += trainingsession.cpp

unix:LIBS += -lz
win32-g++:LIBS += -lz
