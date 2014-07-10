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

    QFile goldenMessage(QLatin1String("protobuf/testdata/golden_message"));
    goldenMessage.open(QIODevice::ReadOnly);
    QTest::newRow("golden-message") << goldenMessage.readAll() << QVariant();
    goldenMessage.close();

    QFile goldenPackedFieldsMessage(QLatin1String("./protobuf/testdata/golden_packed_fields_message"));
    goldenPackedFieldsMessage.open(QIODevice::ReadOnly);
    QTest::newRow("golden-packed-fields-message") << goldenPackedFieldsMessage.readAll() << QVariant();
    goldenPackedFieldsMessage.close();

    QFile googleMessage1(QLatin1String("../protobuf/testdata/google_message1.dat"));
    googleMessage1.open(QIODevice::ReadOnly);
    QTest::newRow("google-message1") << googleMessage1.readAll() << QVariant();
    googleMessage1.close();

    QFile googlemessage2(QLatin1String("protobuf/testdata/google_message2.dat"));
    googlemessage2.open(QIODevice::ReadOnly);
    QTest::newRow("google-message1") << googlemessage2.readAll() << QVariant();
    googlemessage2.close();
}

void TestMessage::parse()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QVERIFY2(!data.isEmpty(), "failed to load testdata");

    /// @todo
    //QCOMPARE(ProtoBuf::parseMessage(data), expected);
}
