VERSION = 0.5.1
SPECIAL_BUILD = Internal
TOPDIR = $$PWD

# Setup build number, where available.
if (isEmpty(VER_BUILD)): VER_BUILD = $$(APPVEYOR_BUILD_NUMBER)
if (isEmpty(VER_BUILD)): VER_BUILD = 0
if (isEmpty(VER_PAT)): {
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
