VERSION = 0.5.11
SPECIAL_BUILD = Internal
TOPDIR = $$PWD

# Setup build number, where available.
isEmpty(VER_BUILD):VER_BUILD = $$(APPVEYOR_BUILD_NUMBER)
isEmpty(VER_BUILD):VER_BUILD = 0
isEmpty(VER_PAT) {
    # Qmake only automatically assigns VER_PAT on some platforms.
    VER_PARTS = $$split(VERSION, .)
    VER_PAT = $$member(VER_PARTS, 2)
}
win32:VERSION = $$VERSION"."$$VER_BUILD

# Validate the special build (for inclusion in TCX output).
!contains(SPECIAL_BUILD, ^(Internal|Alpha|Beta|Release)) {
    error("SPECIAL_BUILD must be one of: Internal, Alpha, Beta, or Release")
}

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Enable C++11 for older Qt versions (qmake does this by default for Qt 5.6+).
equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,6) {
    CONFIG += c++11
}

# Enable all warnings for all targets.
CONFIG += warn_on

# Treat warnings as errors.
win32-msvc*:QMAKE_CXXFLAGS_WARN_ON += /WX
else:       QMAKE_CXXFLAGS_WARN_ON += -Werror

# Neaten the output directories.
CONFIG(debug,debug|release) DESTDIR = debug
CONFIG(release,debug|release) DESTDIR = release
MOC_DIR = $$DESTDIR/tmp
OBJECTS_DIR = $$DESTDIR/tmp
RCC_DIR = $$DESTDIR/tmp
UI_DIR = $$DESTDIR/tmp

# Workaround a couple of known bugs with C++11 support for Qt with gcc on OSX.
macx-g++:contains(CONFIG, c++11) {
    equals(QT_MAJOR_VERSION,5):equals(QT_MINOR_VERSION,2) {
        # https://bugreports.qt.io/browse/QTBUG-28097
        message(Setting OSX deployment target to 10.7 for for C++11 with $$QMAKE_CXX and $$QT_VERSION)
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        message(Adding '-stdlib=libc++' flag for C++11 with $$QMAKE_CXX and $$QT_VERSION)
        QMAKE_CXXFLAGS += -stdlib=libc++
    }
    # This is necessary for at least Qt 5.5 and 5.7 on OSX (with GCC). But not Qt 5.9.
    equals(QT_MAJOR_VERSION,5):greaterThan(QT_MINOR_VERSION,4):lessThan(QT_MINOR_VERSION,8) {
        message(Adding '-stdlib=libc++' flag for C++11 with $$QMAKE_CXX and $$QT_VERSION)
        QMAKE_CXXFLAGS += -stdlib=libc++
    }
}

# Workaround LLVM bug https://llvm.org/bugs/show_bug.cgi?id=26396 https://bugreports.qt.io/browse/QTBUG-52134
macx:equals(QT_MAJOR_VERSION,5):equals(QT_MINOR_VERSION,5) {
    message(Adding '-Wno-return-stack-address' flag $$QT_VERSION)
    QMAKE_CXXFLAGS += -Wno-return-stack-address
}
