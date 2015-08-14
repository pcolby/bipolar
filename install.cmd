@echo off
:: This is just a convenient dev-time install script. This will be superseded
:: by an appropriate installer wizard for end users.
echo Note: You will most likely need to run this as an administrator.

if exist "C:\Program Files (x86)\Polar\Polar FlowSync" (
  set FLOWSYNC_DIR=C:\Program Files ^(x86^)\Polar\Polar FlowSync
) else if exist "C:\Program Files\Polar\Polar FlowSync" (
  set FLOWSYNC_DIR=C:\Program Files\Polar\Polar FlowSync
) else (
  echo Failed to find Polar FlowSync installation.
  pause
  exit 1
)

set LIB_DIR=%~dp0\build\qtbase\lib
if not exist "%LIB_DIR%\Qt5Network.dll" (
  echo Failed to find custom network hook: %LIB_DIR%\Qt5Network.dll
  pause
  exit 1
)

if not exist "%FLOWSYNC_DIR%\Qt5Network.dll.backup" (
  copy "%FLOWSYNC_DIR%\Qt5Network.dll" "%FLOWSYNC_DIR%\Qt5Network.dll.backup"
)
copy "%LIB_DIR%\Qt5Network.dll" "%FLOWSYNC_DIR%\Qt5Network.dll"
pause