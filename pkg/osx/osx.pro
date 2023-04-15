# SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: GPL-3.0-or-later

include(../../common.pri)

TEMPLATE = aux

hook.target = $$OUT_PWD/QtNetwork
hook.commands = curl -L -o $$shell_quote($$OUT_PWD/QtNetwork) \
                https://github.com/pcolby/bipolar/releases/download/hook-3-rc2/QtNetwork-macos-12

dmg.depends = $$OUT_PWD/../../src/release/Bipolar.app hook
dmg.target = dmg
dmg.commands = $$shell_quote($$PWD/build.sh) \
               --app $$shell_quote($$OUT_PWD/../../src/release/Bipolar.app) \
               --hook $$shell_quote($$OUT_PWD/QtNetwork) \
               --version $$shell_quote($$VERSION)

QMAKE_EXTRA_TARGETS = hook dmg

QMAKE_SUBSTITUTES = install.command.in README.txt.in
install.command.in.CONFIG = verbatim
README.txt.in.CONFIG = verbatim
