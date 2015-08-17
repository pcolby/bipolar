@echo off

:: Make sure the NSIS, git and Qt binaries are in the path.
path=%path%;C:\Program Files\NSIS;C:\Program Files (x86)\NSIS
path=%path%;C:\Qt\5.3\msvc2013\bin

:: Gather the Qt dependencies.
if exist qtlibs rm -rf qtlibs
windeployqt.exe --dir qtlibs ..\..\release\Bipolar.exe
if errorlevel 1 (
  echo windeployqt.exe failed
  pause
  exit errorlevel
)

makensis win32.nsi

pause
