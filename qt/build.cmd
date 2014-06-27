@echo off
:: Prerequisites:
:: * MSVC tool chain; ideally version 10.0, such as Visual C++ Express 2010.
::    * Installed.
:: * OpenSSL (ideally version 1.0.1g, or ABI compatible), headers and binaries installed.
::    * http://slproweb.com/download/Win32OpenSSL-1_0_1g.exe
:: * Qt 5.1.1 source, in the "src" subdirectory.
::    * http://download.qt-project.org/archive/qt/5.1/5.1.1/single/

:: The following are all required; adjust to match your setup.
set MSVC=C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC
set OPENSSL=C:\OpenSSL-Win32
set QT_VERSION=5.1.1
set ZIP7=%PROGRAMFILES%\7-zip\7z.exe

:: Check the pre-requisites.
if not exist "%OPENSSL%" (
  echo OpenSSL not found. Have you installed OpenSSL for Windows?
  pause
  exit 1
)
if exist "%~dp0src\qt-everywhere-opensource-src-%QT_VERSION%.7z" (
  set QT_SRC_FILE=%~dp0src\qt-everywhere-opensource-src-%QT_VERSION%.7z
) else if exist "%~dp0src\qt-everywhere-opensource-src-%QT_VERSION%.zip" (
  set QT_SRC_FILE=%~dp0src\qt-everywhere-opensource-src-%QT_VERSION%.zip
) else (
  echo Qt source not found. Have you downloaded the Qt %QT_VERSION% source?
  pause
  exit 1
)
goto main

:: usage: call:extract input-file output-dir
:extract
@echo Extracting "%~1" to "%~2"
"%ZIP7%" x -o"%~2" "%~1" > nul
if errorlevel 1 (
  echo Failed to extract "%~1"
  pause
  exit errorlevel
)
goto :EOF

:: usage: call:extractsource build_dir
:extractSource
set SRC_DIR=%~dp0src
set SRC_FILE=%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.zip
if exist "%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.7z" (
  set SRC_FILE=%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.7z
) else if exist "%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.zip" (
  set SRC_FILE=%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.zip
)
if not exist "%~1" call:extract "%SRC_FILE%" "%~1"
goto :EOF

:: usage: call:configure build_dir
:configure
if not exist "%~1qt-build" md "%~1qt-build"
pushd "%~1qt-build"
call "%~1\qt-everywhere-opensource-src-%QT_VERSION%\configure.bat"^
 -opensource^
 -no-gui^
 -no-opengl^
 -no-openvg^
 -no-widgets^
 -nomake examples^
 -nomake tools^
 -openssl -I C:\OpenSSL-Win32\include -L C:\OpenSSL-Win32\lib^
 -platform win32-msvc2010^
 -qt-sql-sqlite^
 -release^
 -shared^
 -skip multimedia^
 -skip script^
 -skip svg^
 -skip webkit
nmake.exe module-qtbase
popd
goto :EOF

:build
call "%MSVC%\vcvarsall.bat" x86
set BUILD_DIR=%~dp0build\qt-%QT_VERSION%-%~1-%~2
if not exist "%BUILD_DIR%" call:extractSource "%BUILD_DIR%"
set BOOST_DIR=%BUILD_DIR%\qt-build
:: @todo Replace the following b2.exe with something relevant to this project, eg Qt5Network.dll
if not exist "%BUILD_DIR%\b2.exe" call:configure "%BUILD_DIR%"
goto :EOF

:main
if not exist "%~dp0build" md "%~dp0build"
call:build x86 release
pause