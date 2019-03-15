include(../../common.pri)

TEMPLATE = aux

HOOK_URL = https://github.com/pcolby/bipolar/releases/download/hook-2-osx-x86-64/Qt5Network.dll
POWERSHELL = $$(SYSTEMROOT)/System32/WindowsPowerShell/v1.0/powershell.exe
DOWNLOAD_SCRIPT = "(New-Object Net.WebClient).DownloadFile('$$HOOK_URL', 'Qt5Network.dll')"
hook.target = Qt5Network.dll
win32-msvc*:hook.commands = $$system_quote($$system_path($$POWERSHELL)) \
                            -Command $$system_quote($$DOWNLOAD_SCRIPT)
else:hook.commands = $$shell_quote($$shell_path($$POWERSHELL)) \
                     -Command $$shell_quote($$system_quote($$DOWNLOAD_SCRIPT))

qtlibs.depends = $$OUT_PWD/../../src/$$(CONFIGURATION)/Bipolar.exe
qtlibs.target = qtlibs
qtlibs.commands = windeployqt.exe --dir qtlibs \
                  $$system_quote($$system_path($$OUT_PWD/../../src/$$(CONFIGURATION)/Bipolar.exe))

nsis.depends = $$OUT_PWD/../../src/$$(CONFIGURATION)/Bipolar.exe qtlibs hook
nsis.target = nsis
nsis.commands = $$shell_quote($$system_path(C:/Program Files (x86)/NSIS/makensis.exe)) -V4 Bipolar.nsi

QMAKE_EXTRA_TARGETS = hook nsis qtlibs

QMAKE_SUBSTITUTES += Bipolar.nsi.in gpl-3.0.rtf.in
gpl-3.0.rtf.in.CONFIG = verbatim
