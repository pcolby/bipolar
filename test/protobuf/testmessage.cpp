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

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QTest>

Q_DECLARE_METATYPE(ProtoBuf::Message::FieldInfoMap)

ProtoBuf::Message::FieldInfoMap loadFieldInfoMap()
{
    /// @todo
    //QFile fields(QFINDTESTDATA("testdata/golden_message" subTest ".fields.json"));
    //fields.open(QIODevice::ReadOnly);
    return ProtoBuf::Message::FieldInfoMap();
}

void TestMessage::parse_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<ProtoBuf::Message::FieldInfoMap>("fieldInfo");
    QTest::addColumn<QJsonDocument>("expected");

    #define LOAD_TEST_DATA(name, subTest) { \
        QFile dataFile(QFINDTESTDATA("testdata/" name)); \
        dataFile.open(QIODevice::ReadOnly); \
        QFile expected(QFINDTESTDATA("testdata/" name subTest ".expected.json")); \
        expected.open(QIODevice::ReadOnly); \
        QTest::newRow(name) \
            << dataFile.readAll() \
            << loadFieldInfoMap() \
            << QJsonDocument::fromJson(expected.readAll()); \
    }

    LOAD_TEST_DATA("golden_message", "");
    LOAD_TEST_DATA("golden_packed_fields_message", "")
    LOAD_TEST_DATA("google_message1.dat", "")
    LOAD_TEST_DATA("google_message2.dat", "")

    #undef LOAD_TEST_DATA
}

void TestMessage::parse()
{
    QFETCH(QByteArray, data);
    QFETCH(ProtoBuf::Message::FieldInfoMap, fieldInfo);
    QFETCH(QJsonDocument, expected);

    QVERIFY2(!data.isEmpty(), "failed to load testdata");

    // Parse the protobuf message.
    const ProtoBuf::Message message(fieldInfo);
    const QVariantMap result = message.parse(data);
    const QJsonDocument json = QJsonDocument::fromVariant(result);

    // Write the result to a JSON file for optional post-mortem investigation.
    QFile output(QString::fromLatin1("protobuf/testdata/%1.result.json")
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (output.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        output.write(json.toJson().data());
        output.close();
    }

    // Compare the result.
    QCOMPARE(json, expected);
}
