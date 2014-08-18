@echo off

:: Make sure the git binaries are in the path.
PATH=%PATH%;C:\Program Files\Git\bin;C:\Program Files (x86)\Git\bin

:: Check that we have the right number of commandline arguments.
if "%1" == "" goto usage
if "%2" == "" goto usage
if "%3" == "" goto usage
goto main
:usage
echo usage: %0 git_dir source_file destination_file
pause
goto :EOF

:main
:: Set the REVISION variable to the current git revision number.
git.exe --git-dir "%1" rev-list --count HEAD > .revision
set /p REVISION=<.revision
del .revision

::rem Apply the revision number to a *copy* of the source file.
sed.exe -e "s/\(\([ 0-9]\+[.,]\)\{3\} *\)0/\1%REVISION%/g" "%2" > "%2.tmp"

::rem If the revision numbers differ, then overwrite the destination.
diff.exe -qs "%2.tmp" "%3"
IF ERRORLEVEL 1 copy "%2.tmp" "%3"
del "%2.tmp"