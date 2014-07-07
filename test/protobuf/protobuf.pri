VPATH += $$PWD
HEADERS += testfixnum.h   testmessage.h   testvarint.h
SOURCES += testfixnum.cpp testmessage.cpp testvarint.cpp

include(../../src/protobuf/protobuf.pri)
