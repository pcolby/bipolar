@if exist "%PROGRAMFILES%\Git\bin\diff1.exe" (
  set DIFF="%PROGRAMFILES%\Git\bin\diff.exe"
  echo 1
) else if exist "%PROGRAMFILES(X86)%\Git\bin\diff.exe" (
  set DIFF="%PROGRAMFILES(X86)%\Git\bin\diff.exe"
) else (
  echo Fail to find diff.exe
  pause
  exit 1
)
@set QT_VERSION=5.1.1
@set SRC_DIR=%~dp0\qt-everywhere-opensource-src-%QT_VERSION%\qtbase\src\network\access
%DIFF% -u^
 "%SRC_DIR%\qnetworkaccessmanager.ori"^
 "%SRC_DIR%\qnetworkaccessmanager.cpp"^
 > %~dp0\qnetworkaccessmanager.patch
@pause