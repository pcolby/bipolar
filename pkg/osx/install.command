#!/bin/bash
#
# Usage: install.command [path-to-FlowSync.app]
#

FLOWSYNC_APP='/Applications/FlowSync.app'
if [ $# -gt 0 ]; then
    FLOWSYNC_APP="$1"
fi

if [ ! -e "$FLOWSYNC_APP" ]; then
    echo "FlowSync not found: $FLOWSYNC_APP" 2>&1
    exit 1
fi

TIMESTAMP=`date -j +%Y%m%d%H%M%S`
TARGET_FILE="$FLOWSYNC_APP/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork"
BACKUP_FILE="$TARGET_FILE.$TIMESTAMP"

echo 'Backing up:'
echo "  From: $TARGET_FILE"
echo "    To: $BACKUP_FILE"
echo -n 'Note, this requires administrator access.'
echo ' You will probably be prompted for a password.'
sudo cp -a "$TARGET_FILE" "$BACKUP_FILE" || exit 2

SELF_DIR=`dirname "$0"`
echo "Installing $SELF_DIR/QtNetwork"
sudo cp -af "$SELF_DIR/QtNetwork" "$TARGET_FILE" || exit 3

echo 'Hook installed successfully.'
