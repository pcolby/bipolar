include(../../common.pri)

TEMPLATE = aux

hook.target = Qt5Network.dll
hook.commands = $$shell_path($$(SYSTEMROOT)\System32\WindowsPowerShell\v1.0\powershell.exe) -Command \
                $$shell_quote("(New-Object Net.WebClient).DownloadFile('https://github.com/pcolby/bipolar/releases/download/hook-2/Qt5Network.dll', 'Qt5Network.dll')")

qtlibs.depends = $$OUT_PWD/../../src/release/Bipolar.exe
qtlibs.target = qtlibs
qtlibs.commands = windeployqt.exe --dir qtlibs \
                  $$shell_path($$OUT_PWD/../../src/release/Bipolar.exe)

nsis.depends = $$OUT_PWD/../../src/release/Bipolar.exe qtlibs hook
nsis.target = nsis
nsis.commands = $$shell_quote(C:\Program Files (x86)\NSIS\makensis.exe) \
                $$shell_path($$OUT_PWD/Bipolar.nsi)

QMAKE_EXTRA_TARGETS = hook nsis qtlibs

QMAKE_SUBSTITUTES += Bipolar.nsi.in gpl-3.0.rtf.in
gpl-3.0.rtf.in.CONFIG = verbatim
