include(../../common.pri)

FOO = "c:\\a\\b/c d"
BAR = "\"c:\\a\\b/c d\""
message("ori" $$FOO)
message("shp" $$shell_path($$FOO))
message("syp" $$system_path($$FOO))
message("shq" $$shell_quote($$FOO))
message("syq" $$system_quote($$FOO))
message("ori" $$BAR)
message("shp" $$shell_path($$BAR))
message("syp" $$system_path($$BAR))
message("shq" $$shell_quote($$BAR))
message("syq" $$system_quote($$BAR))

TEMPLATE = aux

hook.target = Qt5Network.dll
hook.commands = $$system_quote($$system_path($$(SYSTEMROOT)/System32/WindowsPowerShell/v1.0/powershell.exe)) -Command \
                $$system_quote("(New-Object Net.WebClient).DownloadFile('https://github.com/pcolby/bipolar/releases/download/hook-2/Qt5Network.dll', 'Qt5Network.dll')")

qtlibs.depends = $$OUT_PWD/../../src/release/Bipolar.exe
qtlibs.target = qtlibs
qtlibs.commands = windeployqt.exe --dir qtlibs \
                  $$system_quote($$system_path($$OUT_PWD/../../src/release/Bipolar.exe))

nsis.depends = $$OUT_PWD/../../src/release/Bipolar.exe qtlibs hook
nsis.target = nsis
nsis.commands = $$system_quote($$system_path(C:/Program Files (x86)/NSIS/makensis.exe)) \
                $$system_quote($$system_path($$OUT_PWD/Bipolar.nsi))

QMAKE_EXTRA_TARGETS = hook nsis qtlibs

QMAKE_SUBSTITUTES += Bipolar.nsi.in gpl-3.0.rtf.in
gpl-3.0.rtf.in.CONFIG = verbatim
