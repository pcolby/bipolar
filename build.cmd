@echo off
:: Prerequisites:
:: * MSVC tool chain; ideally version 10.0, such as Visual C++ Express 2010.
::    * Installed.
:: * OpenSSL (ideally version 1.0.1g, or ABI compatible), headers and binaries installed.
::    * http://slproweb.com/download/Win32OpenSSL-1_0_1g.exe
:: * Qt 5.5.1 source, in the "src" subdirectory.
::    * http://download.qt.io/archive/qt/5.5/5.5.1/single/

:: The following are all required; adjust to match your setup.
set MSVC=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC
set OPENSSL=C:\OpenSSL-Win32
set PATCH=C:\Program Files\Git\usr\bin\patch.exe
set QT_VERSION=5.5.1
set ZIP7=%PROGRAMFILES%\7-zip\7z.exe

:: Check the pre-requisites.
if not exist "%OPENSSL%" (
  echo OpenSSL not found. Have you installed OpenSSL for Windows?
  pause
  exit 1
)
if not exist "%PATCH%" (
  echo Patch command not found.
  pause
  exit 1
)

:: Setup the Visual C++ environment.
call "%MSVC%\vcvarsall.bat" x86

:: Extract the Qt source, if not already.
set SRC_DIR=%~dp0\qt-everywhere-opensource-src-%QT_VERSION%
if not exist "%SRC_DIR%" (
  if exist "%SRC_DIR%.7z" (
    set SRC_FILE=%SRC_DIR%.7z
  ) else if exist "%SRC_DIR%.zip" (
    set SRC_FILE=%SRC_DIR%.zip
  ) else (
    echo Qt source not found. Have you downloaded the Qt %QT_VERSION% source?
    pause
    exit 1
  )
)

if not exist "%SRC_DIR%" (
  echo Extracting "%SRC_FILE%"
  "%ZIP7%" x -o"%~dp0" "%SRC_FILE%" > nul
  if errorlevel 1 (
    echo Failed to extract "%SRC_FILE%"
    pause
    exit errorlevel
  )
)

:: Apply our hook and build patches.
if not exist "%SRC_DIR%\qtbase\src\network\access\qnetworkaccessmanager.ori" (
  copy "%SRC_DIR%\qtbase\src\network\access\qnetworkaccessmanager.cpp"^
       "%SRC_DIR%\qtbase\src\network\access\qnetworkaccessmanager.ori"
)
if not exist "%SRC_DIR%\qtbase\qmake\generators\win32\winmakefile.ori" (
  copy "%SRC_DIR%\qtbase\qmake\generators\win32\winmakefile.cpp"^
       "%SRC_DIR%\qtbase\qmake\generators\win32\winmakefile.ori"
)
"%PATCH%" -N -p0 -i qnetworkaccessmanager.patch
if errorlevel 1 pause
"%PATCH%" -N -p0 -i winmakefile.patch
if errorlevel 1 pause

:: Create the build directory, if not already.
set BUILD_DIR=%~dp0\build
if not exist "%BUILD_DIR%" md "%BUILD_DIR%"
pushd "%BUILD_DIR%"

:: Configure the Qt build, if not already.
if not exist "%BUILD_DIR%\Makefile" (
  call "%SRC_DIR%\configure.bat"^
   -confirm-license^
   -no-gui^
   -no-opengl^
   -no-openvg^
   -no-widgets^
   -nomake examples^
   -nomake tools^
   -openssl -I C:\OpenSSL-Win32\include -L C:\OpenSSL-Win32\lib^
   -opensource^
   -platform win32-msvc2010^
   -release^
   -shared^
   -skip multimedia^
   -skip script^
   -skip svg^
   -skip webkit
)

:: Build the Qt network module (among others).
nmake.exe module-qtbase
dir "%BUILD_DIR%\qtbase\lib\Qt5Network.dll"
popd
rem pause
