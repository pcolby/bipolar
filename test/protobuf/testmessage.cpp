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

ProtoBuf::Message::FieldInfoMap loadFieldInfoMap(const QString &name, const QString &subTest)
{
    ProtoBuf::Message::FieldInfoMap fields;
    QFile file(QString::fromLatin1("testdata/%1%2.fields.csv").arg(name).arg(subTest));
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        foreach (const QString &line, QString::fromLatin1(file.readAll()).split(QLatin1Char('\n'))) {
            const QStringList parts = line.split(QLatin1Char(','));
            if ((parts.size() == 3) && (!parts.at(1).startsWith(QLatin1Char('#')))) {
                ProtoBuf::Message::FieldInfo fieldInfo(parts.at(1));
                #define SET_TYPEHINT_IF_MATCH(type) \
                    if (parts.at(2) == QLatin1String(#type)) { \
                        fieldInfo.typeHint = ProtoBuf::Message::Type##type; \
                    }
                SET_TYPEHINT_IF_MATCH(Unknown)
                SET_TYPEHINT_IF_MATCH(Boolean)
                SET_TYPEHINT_IF_MATCH(Bytes)
                SET_TYPEHINT_IF_MATCH(EmbeddedMessage)
                SET_TYPEHINT_IF_MATCH(Enumerator)
                SET_TYPEHINT_IF_MATCH(FloatingPoint)
                SET_TYPEHINT_IF_MATCH(SignedInteger)
                SET_TYPEHINT_IF_MATCH(StandardInteger)
                SET_TYPEHINT_IF_MATCH(String)
                SET_TYPEHINT_IF_MATCH(UnsignedInteger)
                #undef SET_TYPEHINT_IF_MATCH
                fields[parts.at(1)] = fieldInfo;
            }
        }
    }
    return fields;
}

// Replace raw QByteArray data with hex strings. This is needed because the
// JSON export to UTF-8 is inconsitent accross platforms, and accross Qt
// versions unfortunately.  Note, this is done for diagnostic outputs only,
// and does not affect the QVariant comparisons that make up the actual tests.
void sanitize(QVariant &variant) {
    switch (static_cast<QMetaType::Type>(variant.type())) {
    case QMetaType::QByteArray:
        variant = variant.toByteArray().toHex();
        break;
    case QMetaType::QVariantList: {
            QVariantList list;
            foreach (QVariant item, variant.toList()) {
                sanitize(item);
                list << item;
            }
            variant = list;
        }
        break;
    case QMetaType::QVariantMap: {
            QVariantMap map(variant.toMap());
            for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
                sanitize(iter.value());
            }
            variant = map;
        }
        break;
    }
}

void TestMessage::parse_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<ProtoBuf::Message::FieldInfoMap>("fieldInfo");
    QTest::addColumn<QVariantMap>("expected");

    #define LOAD_TEST_DATA(name, subTest) { \
        QFile dataFile(QFINDTESTDATA("testdata/" name)); \
        dataFile.open(QIODevice::ReadOnly); \
        QFile expectedFile(QFINDTESTDATA("testdata/" name subTest ".expected.var")); \
        expectedFile.open(QIODevice::ReadOnly); \
        QDataStream expectedStream(&expectedFile); \
        QVariantMap expectedMap; \
        expectedStream >> expectedMap; \
        QTest::newRow(name) \
            << dataFile.readAll() \
            << loadFieldInfoMap(QLatin1String(name), QLatin1String(subTest)) \
            << expectedMap; \
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
    QFETCH(QVariantMap, expected);

    QVERIFY2(!data.isEmpty(), "failed to load testdata");

    // Parse the protobuf message.
    const ProtoBuf::Message message(fieldInfo);
    const QVariantMap result = message.parse(data);

    // Write the result to a binary file for optional post-mortem investigation.
#ifdef Q_OS_WIN
    QFile binaryFile(QString::fromLatin1("protobuf/testdata/%1.result.var")
#else
    QFile binaryFile(QString::fromLatin1("../protobuf/testdata/%1.result.var")
#endif
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (binaryFile.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QDataStream stream(&binaryFile);
        stream << result;
        binaryFile.close();
    }

    // Write the result to a JSON file for optional post-mortem investigation.
#ifdef Q_OS_WIN
    QFile jsonFile(QString::fromLatin1("protobuf/testdata/%1.result.json")
#else
    QFile jsonFile(QString::fromLatin1("../protobuf/testdata/%1.result.json")
#endif
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (jsonFile.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QVariant saneResult(result);
        sanitize(saneResult);
        jsonFile.write(QJsonDocument::fromVariant(saneResult).toJson());
        jsonFile.close();
    }

    // Compare the result.
    QCOMPARE(result, expected);
}
