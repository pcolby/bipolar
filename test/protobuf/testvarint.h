// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

class TestVarint : public QObject {
    Q_OBJECT

private slots:
    void parseSignedInt_data();
    void parseSignedInt();
    void parseSignedInts_data();
    void parseSignedInts();

    void parseStandardInt_data();
    void parseStandardInt();
    void parseStandardInts_data();
    void parseStandardInts();

    void parseUnsignedInt_data();
    void parseUnsignedInt();
    void parseUnsignedInts_data();
    void parseUnsignedInts();

};
