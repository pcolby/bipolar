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

:: usage: call:configure build_dir src_dir
:configure
goto :EOF

:main

:: Setup the Visual C++ environment.
call "%MSVC%\vcvarsall.bat" x86

:: Extract the Qt source, if not already.
set SRC_DIR=%~dp0\qt-everywhere-opensource-src-%QT_VERSION%
if not exist "%SRC_DIR%\configure.bat" (
  if exist "%SRC_DIR%.7z" (
    call:extract "%SRC_DIR%.7z" "%~dp0"
  ) else if exist "%SRC_DIR%.zip" (
    call:extract "%SRC_DIR%.zip" "%~dp0"
  ) else (
    echo Qt source not found. Have you downloaded the Qt %QT_VERSION% source?
    pause
    exit 1
  )
)

:: Create the build directory, if not already.
set BUILD_DIR=%~dp0\build
if not exist "%BUILD_DIR%" md "%BUILD_DIR%"
pushd "%BUILD_DIR%"

call "%SRC_DIR%\configure.bat"^
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

:: Build the base Qt modules.
::nmake.exe module-qtbase

popd

pause