# Create a Qt application.
TARGET = Bipolar
TEMPLATE = app
CONFIG += warn_on
QT += network widgets

# Disable automatic ASCII conversions (best practice for internationalization).
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

# Add the embedded resources.
RESOURCES = qrc/app.qrc

# Neaten the output directories.
CONFIG(debug,debug|release) {
    DESTDIR = build/debug
    MOC_DIR = build/debug/tmp
    OBJECTS_DIR = build/debug/tmp
    RCC_DIR = build/debug/tmp
}
CONFIG(release,debug|release) {
    DESTDIR = build/release
    MOC_DIR = build/release/tmp
    OBJECTS_DIR = build/release/tmp
    RCC_DIR = build/release/tmp
}

include(src/src.pri)
