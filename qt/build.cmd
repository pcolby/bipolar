@echo off

::http://download.qt-project.org/archive/qt/5.1/5.1.1/single/
::http://slproweb.com/download/Win32OpenSSL-1_0_1g.exe

:: SSL 1.0.1.7 (1.0.1g)

:: The following are all required; adjust to match your setup.
set QT_VERSION=5.1.1
set ZIP7=%PROGRAMFILES%\7-zip\7z.exe
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

:: usage: call:configureWinSDK x86|x64|ia64 debug|release [/vista^|/xp^|/2003^|/2008^|/win7]
:configureWinSDK
if "%WIN_SDK%" EQU "" goto :EOF
set TARGET_ARCH=/%1
set MODE=/%2
if "%3" EQU "" ( set TARGET_OS=/xp ) else set TARGET_OS=%3
call "%SET_ENV%" %MODE% %TARGET_ARCH% %TARGET_OS%
goto :EOF

:: usage: call:extractsource build_dir
:extractSource
set SRC_DIR=%~dp0src
set SRC_FILE=%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.zip
if exist "%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.7z" (
  set SRC_FILE=%SRC_DIR%\qt-everywhere-opensource-src-%QT_VERSION%.7z
)
if not exist "%~1" call:extract "%SRC_FILE%" "%~1"
goto :EOF

:: usage: call:configure build_dir
:configureX
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

:: usage: call:build x86|x64|ia64 debug|release
:build
@echo ==== Building %~1 %~2 ====
::call:configureWinSDK %~1 %~2
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
set BUILD_DIR=%~dp0build\qt-%QT_VERSION%-%~1-%~2
if not exist "%BUILD_DIR%" call:extractSource "%BUILD_DIR%"
set BOOST_DIR=%BUILD_DIR%\qt-build
if not exist "%BUILD_DIR%\b2.exe" call:configureX "%BUILD_DIR%"
::set INSTALL_DIR=%BUILD_DIR%\install
::if not exist "%INSTALL_DIR%" call:buildBoost %BOOST_DIR% %INSTALL_DIR% %~1 %~2
goto :EOF

:main
if not exist "%~dp0build" md "%~dp0build"
call:build x86 release
pause