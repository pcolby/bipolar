/*
    Copyright 2014 Paul Colby

    This file is part of Bipolar.

    Bipolar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Biplar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bipolar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testmessage.h"

#include "../../src/protobuf/message.h"

#include <QFile>
#include <QTest>

void TestMessage::parse_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile file(QFINDTESTDATA("testdata/" name)); \
        file.open(QIODevice::ReadOnly); \
        QTest::newRow(name) << file.readAll() << QVariant(); \
    }

    LOAD_TEST_DATA("golden_message")
    LOAD_TEST_DATA("golden_packed_fields_message")
    LOAD_TEST_DATA("google_message1.dat")
    LOAD_TEST_DATA("google_message2.dat")

    #undef LOAD_TEST_DATA
}

void TestMessage::parse()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QVERIFY2(!data.isEmpty(), "failed to load testdata");

    /// @todo
    //QCOMPARE(ProtoBuf::parseMessage(data), expected);
}
