@echo off

:: Make sure the git binaries are in the path.
PATH=%PATH%;C:\Program Files\Git\bin;C:\Program Files (x86)\Git\bin

:: Check that we have the right number of commandline arguments.
if "%1" == "" goto usage
if "%2" == "" goto usage
goto main
:usage
echo usage: %0 source_file destination_file
pause
goto :EOF

:main
:: Set the REVISION variable to the current git revision number.
git.exe rev-list --count HEAD > .revision
set /p REVISION=<.revision
del .revision

::rem Apply the revision number to a *copy* of the source file.
sed.exe -e "s/\(\([ 0-9]\+[.,]\)\{3\} *\)0/\1%REVISION%/g" "%1" > "%1.tmp"

::rem If the revision numbers differ, then overwrite the destination.
diff.exe -qs "%1.tmp" "%2"
IF ERRORLEVEL 1 copy "%1.tmp" "%2"
del "%1.tmp"