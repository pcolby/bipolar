# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

include(../common.pri)

# Unset the VERSION variable for MinGW only. Unfortunately, this is necessary
# due to a limitation in MinGW's windres Windows resource compiler, that means
# it cannot handle defines containing spaces. The presence of VERSION variable
# triggers qmake's automatic generation of a Windows resource file, which we
# wish to avoid for MinGW. Note, we don't really *need* spaces in our defines,
# however these unit tests intentionally set the BUILD_USER to 'unit tests' to
# test / verify exactly this sort of behaviour. This also implies that anyone
# building the Bipolar.exe appliction, should either not have spaces in their
# username, or manually override the BUILD_USER value, or use MSVC not MinGW.
win32-g++:unset(VERSION)

TEMPLATE = app
QT += testlib widgets xml xmlpatterns
CONFIG += testcase
SOURCES += test.cpp

# Define the build user (for TCX).
DEFINES += BUILD_USER=$$shell_quote(unit tests)

# Add the embedded resources.
RESOURCES = $$TOPDIR/qrc/app.qrc

# Code coverage reporting (for Linux at least).
unix {
    # Enable gcov compile and link flags.
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_CXXFLAGS_RELEASE -= -O1 -O2 -O3
    QMAKE_RPATHDIR += ../release

    # Generate gcov's gcda files by executing the test program.
    gcov.depends = test
    gcov.target = build/test.gcda
    gcov.commands = ./test

    # Generate an lcov tracefile from gcov's gcda files.
    lcov.depends = build/test.gcda
    lcov.target = build/coverage.info
    lcov.commands = lcov --capture --base-directory ../src --directory build \
                         --output build/coverage.info --quiet; \
                    lcov --remove build/coverage.info '"/usr/include/*/*"' \
                         '"src/*/test*"' '"src/build/*"' src/test.cpp \
                         --output build/coverage.info --quiet

    # Generate HTML coverage reports from lcov's tracefile.
    coverage.depends = build/coverage.info
    coverage.commands += genhtml --output-directory coverage_html \
                         --prefix `readlink -f ../src` --quiet \
                         --title PROJECT_NAME build/coverage.info

    # Include the custom targets in the generated build scripts (eg Makefile).
    QMAKE_EXTRA_TARGETS += coverage gcov lcov

    # Clean up files generated by the above custom targets.
    QMAKE_CLEAN += build/*.gcda build/*.gcno build/coverage.info
    QMAKE_DISTCLEAN += -r coverage_html
}

# Qt 5.3 with MSVC (on AppVeyor at least) needs a stack larger than the 1MB default.
win32:equals(QMAKE_CXX,cl) {
    equals(QT_MAJOR_VERSION,5):equals(QT_MINOR_VERSION,3) {
        message(Setting explicit stack size for $$QMAKE_CXX with $$QT_VERSION)
        QMAKE_LFLAGS += /STACK:2097152
    }
}

INCLUDEPATH += $$PWD
INCLUDEPATH += $$TOPDIR/src
include(polar/v2/v2.pri)
include(protobuf/protobuf.pri)
include(tools/tools.pri)
include($$TOPDIR/src/os/os.pri)
