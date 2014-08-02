#!/bin/bash

APPLICATIONS=/Applications
FLOWSYNC_APP="$APPLICATIONS/FlowSync.app"
TARGET_APP="$APPLICATIONS/FlowSync Hooked.app"

if [ -e "$TARGET_APP" ]; then
    if [ "$1" == '--force' ]; then
        echo "Removing old target app: $TARGET_APP"
        rm -rf "$TARGET_APP"
    else
        echo "Target already exists: $TARGET_APP" 2>&1
        echo 'Consider using --force'
        exit 1
    fi
fi

echo "Copying '$FLOWSYNC_APP' to '$TARGET_APP'"
cp -a "$FLOWSYNC_APP" "$TARGET_APP"


echo 'Installing hook'
cp build/qtbase/lib/QtNetwork.framework/Versions/5/QtNetwork \
   "$TARGET_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork"

install_name_tool -id \
    '@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork' \
    "$TARGET_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork"

install_name_tool -change \
    '/usr/local/Qt-5.1.1/lib/QtCore.framework/Versions/5/QtCore' \
    '@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore' \
    "$TARGET_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork"

echo 'Done'

echo 'Debug info:'
otool -L "$FLOWSYNC_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork" | grep -i qt
otool -L "$TARGET_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork" | grep -i qt
