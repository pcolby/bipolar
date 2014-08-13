@echo off

:: Make sure the NSIS, git and Qt binaries are in the path.
path=%path%;C:\Program Files\NSIS;C:\Program Files (x86)\NSIS
path=%path%;C:\Program Files\Git\bin;C:\Program Files (x86)\Git\bin
path=%path%;C:\Qt\5.3\msvc2013\bin

:: Gather the Qt dependencies.
if exist qtlibs rm -rf qtlibs
windeployqt.exe --dir qtlibs ..\..\release\Bipolar.exe
if errorlevel 1 (
  echo windeployqt.exe failed
  pause
  exit errorlevel
)

:: Set the REVISION variable to the current git revision number.
git.exe rev-list --count HEAD > .revision
set /p REVISION=<.revision
del .revision

:: Update the NSIS script, and build the installer.
sed.exe -e "s/\(\([ 0-9]\+[.,]\)\{3\} *\)0/\1%REVISION%/g" win32.nsi.in > win32.nsi
makensis win32.nsi

pause