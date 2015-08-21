include(../../common.pri)

TEMPLATE = aux

hook.target = $$OUT_PWD/QNetwork
hook.commands = curl -o $$shell_quote($$OUT_PWD/QNetwork) \
                https://github.com/pcolby/bipolar/releases/download/hook-2/QtNetwork

dmg.depends = $$OUT_PWD/../../src/release/Bipolar.app hook
dmg.target = dmg
dmg.commands = $$shell_quote($$PWD/build.sh) \
               --app $$shell_quote($$OUT_PWD/../../src/release/Bipolar.app) \
               --hook $$shell_quote($$OUT_PWD/QNetwork) \
               --version $$shell_quote($$VERSION)

QMAKE_EXTRA_TARGETS = hook dmg

QMAKE_SUBSTITUTES = install.command.in README.txt.in
install.command.in.CONFIG = verbatim
README.txt.in.CONFIG = verbatim
