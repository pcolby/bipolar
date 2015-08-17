include(../common.pri)

# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += widgets xml

# Define the build user (for TCX).
win32:DEFINES += $$shell_quote(BUILD_USER=$$(USERNAME))
else: DEFINES += $$shell_quote(BUILD_USER=$$(USER))

# Add the embedded resources.
RESOURCES = $$TOPDIR/qrc/app.qrc
macx {
    ICON  = $$TOPDIR/qrc/icon/Bipolar.icns
    QMAKE_INFO_PLIST = $$OUT_PWD/../qrc/Info.plist
    QMAKE_SUBSTITUTES += $$TOPDIR/qrc/Info.plist.in
}
win32 {
    RC_FILE = $$OUT_PWD/../qrc/Bipolar.rc
    QMAKE_SUBSTITUTES += $$TOPDIR/qrc/Bipolar.rc.in
}

include(src.pri)
