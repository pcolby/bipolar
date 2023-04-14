// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>

class TestMessage : public QObject {
    Q_OBJECT

private:
    QString outputDirPath;

private slots:
    void initTestCase();

    void parse_data();
    void parse();

};
