@if exist "%PROGRAMFILES%\Git\bin\diff1.exe" (
  set DIFF="%PROGRAMFILES%\Git\bin\diff.exe"
) else if exist "%PROGRAMFILES(X86)%\Git\bin\diff.exe" (
  set DIFF="%PROGRAMFILES(X86)%\Git\bin\diff.exe"
) else (
  echo Failed to find diff.exe
  pause
  exit 1
)

@set QT_VERSION=5.1.1

@set SRC_DIR=qt-everywhere-opensource-src-%QT_VERSION%\qtbase\src\network\access
ren "%SRC_DIR%\qnetworkaccessmanager.cpp" "qnetworkaccessmanager.new"
copy "%SRC_DIR%\qnetworkaccessmanager.ori" "%SRC_DIR%\qnetworkaccessmanager.cpp"
%DIFF% -u^
 "%SRC_DIR%\qnetworkaccessmanager.cpp"^
 "%SRC_DIR%\qnetworkaccessmanager.new"^
 > %~dp0\qnetworkaccessmanager.patch
del "%SRC_DIR%\qnetworkaccessmanager.cpp"
ren "%SRC_DIR%\qnetworkaccessmanager.new" "qnetworkaccessmanager.cpp"

@set SRC_DIR=qt-everywhere-opensource-src-%QT_VERSION%\qtbase\qmake\generators\win32
ren "%SRC_DIR%\winmakefile.cpp" "winmakefile.new"
copy "%SRC_DIR%\winmakefile.ori" "%SRC_DIR%\winmakefile.cpp"
%DIFF% -u^
 "%SRC_DIR%\winmakefile.cpp"^
 "%SRC_DIR%\winmakefile.new"^
 > %~dp0\winmakefile.patch
del "%SRC_DIR%\winmakefile.cpp"
ren "%SRC_DIR%\winmakefile.new" "winmakefile.cpp"

@pause