#!/bin/bash

while [ $# -gt 0 ]; do
  case "$1" in
    --app)     APP="$2"     ; shift ;;
    --hook)    HOOK="$2"    ; shift ;;
    --version) VERSION="$2" ; shift ;;
  esac
  shift
done

if [[ -z "$APP" || -z "$HOOK" || -z "$VERSION" ]]; then
    echo "Usage: $0 --app <path> --hook <path> --version <a.b.c>" >&2
    exit 1
fi

if [ ! -e "$APP" ]; then
    echo "App not found: $APP" 2>&1
    exit 1
fi

if [ ! -e "$HOOK" ]; then
    echo "Hook not found: $HOOK" 2>&1
    exit 1
fi

if [ -n "$TRAVIS_BUILD_NUMBER" ]; then
    VERSION="$VERSION.$TRAVIS_BUILD_NUMBER"
fi

if [ -e /Volumes/Bipolar ]; then
    echo 'Already mounted: /Volumes/Bipolar' 2>&1
    exit 1
fi

if [ -e Bipolar.app ]; then
    if [ "$1" == '--force' ]; then
        rm -rf Bipolar.app
    else
        echo 'Bipolar.app already exists' 2>&1
        echo 'Consider using --force'
        exit 1
    fi
fi

echo 'Copying Bipolar.app'
cp -a "$APP" .

echo 'Running macdeployqt'
macdeployqt Bipolar.app -dmg || exit

echo 'Coverting disk image to read/writable'
[ -e Bipolar-rw.dmg ] && rm -f Bipolar-rw.dmg
hdiutil convert Bipolar.dmg -format UDRW -o Bipolar-rw.dmg || exit

echo 'Adding hook to disk image'
hdiutil attach Bipolar-rw.dmg     || exit
mkdir /Volumes/Bipolar/Hook       || exit
cp "$HOOK" /Volumes/Bipolar/Hook/ || exit
otool -L '/Volumes/Bipolar/Hook/QtNetwork' | grep -i qt # Debug info only.

# @todo Consider removing this.  Current Qt/OSX buildchain set the ID to
# '@rpath/QtNetwork.framework/Versions/5/QtNetwork', which should work fine.
# Whereas back in 2014 it would typically be set to an absolute path on the
# build machine. Anyway, forcing it to be based on '@executable_path/...'
# still seems to work, but we can probably drop it sometime, with testing.
install_name_tool -id \
    '@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork' \
    '/Volumes/Bipolar/Hook/QtNetwork' || exit

# @todo Consider removing this.  It actually does nothing now, since current Qt
# versions set the relevant name to '@rpath/QtCore.framework/Versions/5/QtCore'
# so the following `-change` doesn't find anything to change anymore.
install_name_tool -change \
    '/usr/local/Qt-5.5.1/lib/QtCore.framework/Versions/5/QtCore' \
    '@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore' \
    '/Volumes/Bipolar/Hook/QtNetwork' || exit

otool -L '/Volumes/Bipolar/Hook/QtNetwork' | grep -i qt # Debug info only.
cp install.command /Volumes/Bipolar/Hook || exit
cp README.txt /Volumes/Bipolar/  || exit
hdiutil detach /Volumes/Bipolar/ || exit

echo 'Converting final disk image'
FINALNAME="Bipolar-$VERSION.dmg"
[ -e "$FINALNAME" ] && rm -f "$FINALNAME"
hdiutil convert Bipolar-rw.dmg -format UDZO -o "$FINALNAME"

echo 'Cleaning up Bipolar.app'
rm -rf Bipolar.app Bipolar.dmg Bipolar-rw.dmg
