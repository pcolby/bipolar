INCLUDEPATH += $$PWD
VPATH += $$PWD
HEADERS += inputspage.h   mainwizard.h   outputspage.h   resultspage.h
SOURCES += inputspage.cpp mainwizard.cpp outputspage.cpp resultspage.cpp

include(gpx/gpx.pri)
include(hrm/hrm.pri)
include(tcx/tcx.pri)
