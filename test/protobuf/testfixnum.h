// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

class TestFixnum : public QObject {
    Q_OBJECT

private slots:
    void parseDouble_data();
    void parseDouble();
    void parseDoubles_data();
    void parseDoubles();

    void parseFloat_data();
    void parseFloat();
    void parseFloats_data();
    void parseFloats();

    void parseSigned32_data();
    void parseSigned32();
    void parseSigned32s_data();
    void parseSigned32s();

    void parseSigned64_data();
    void parseSigned64();
    void parseSigned64s_data();
    void parseSigned64s();

    void parseUnsigned32_data();
    void parseUnsigned32();
    void parseUnsigned32s_data();
    void parseUnsigned32s();

    void parseUnsigned64_data();
    void parseUnsigned64();
    void parseUnsigned64s_data();
    void parseUnsigned64s();

};
