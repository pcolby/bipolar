/*
    Copyright 2014-2019 Paul Colby

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
#include "tools/variant.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTest>

Q_DECLARE_METATYPE(ProtoBuf::Message::FieldInfoMap)

ProtoBuf::Message::FieldInfoMap loadFieldInfoMap(const QString &name, const QString &subTest)
{
    ProtoBuf::Message::FieldInfoMap fields;
    QFile file(QFINDTESTDATA(QString::fromLatin1("testdata/%1%2.fields.csv").arg(name).arg(subTest)));
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        foreach (const QString &line, QString::fromLatin1(file.readAll()).split(QLatin1Char('\n'))) {
            const QStringList parts = line.trimmed().split(QLatin1Char(','));
            if ((parts.size() == 3) && (!parts.at(1).startsWith(QLatin1Char('#')))) {
                ProtoBuf::Message::FieldInfo fieldInfo(parts.at(1));
                #define SET_SCALAR_TYPE_IF_MATCH(type) \
                    if (parts.at(2).compare(QLatin1String(#type), Qt::CaseInsensitive) == 0) { \
                        fieldInfo.scalarType = ProtoBuf::Types::type; \
                    }
                SET_SCALAR_TYPE_IF_MATCH(Double)
                else SET_SCALAR_TYPE_IF_MATCH(Float)
                else SET_SCALAR_TYPE_IF_MATCH(Int32)
                else SET_SCALAR_TYPE_IF_MATCH(Int64)
                else SET_SCALAR_TYPE_IF_MATCH(Uint32)
                else SET_SCALAR_TYPE_IF_MATCH(Uint64)
                else SET_SCALAR_TYPE_IF_MATCH(Sint32)
                else SET_SCALAR_TYPE_IF_MATCH(Sint64)
                else SET_SCALAR_TYPE_IF_MATCH(Fixed32)
                else SET_SCALAR_TYPE_IF_MATCH(Fixed64)
                else SET_SCALAR_TYPE_IF_MATCH(Sfixed32)
                else SET_SCALAR_TYPE_IF_MATCH(Sfixed64)
                else SET_SCALAR_TYPE_IF_MATCH(Bool)
                else SET_SCALAR_TYPE_IF_MATCH(String)
                else SET_SCALAR_TYPE_IF_MATCH(Bytes)
                else SET_SCALAR_TYPE_IF_MATCH(EmbeddedMessage)
                else SET_SCALAR_TYPE_IF_MATCH(Enumerator)
                else SET_SCALAR_TYPE_IF_MATCH(Group)
                else {
                    qWarning() << "unknown type" << parts.at(2) << "from" << file.fileName();
                }
                #undef SET_TYPEHINT_IF_MATCH
                fields[parts.at(0)] = fieldInfo;
            }
        }
    }
    return fields;
}

void TestMessage::initTestCase()
{
    outputDirPath = QString::fromLatin1("%1/%2").arg(QDir::tempPath())
        .arg(QString::fromLocal8Bit(metaObject()->className()));
    qDebug() << "output directory" << QDir::toNativeSeparators(outputDirPath);
    QDir dir;
    if (!dir.mkpath(outputDirPath)) {
        qWarning() << "failed to create output directory" << QDir::toNativeSeparators(outputDirPath);
        outputDirPath.clear();
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
        QTest::newRow(name subTest) \
            << dataFile.readAll() \
            << loadFieldInfoMap(QLatin1String(name), QLatin1String(subTest)) \
            << expectedMap; \
    }

    LOAD_TEST_DATA("golden_message", "");
    LOAD_TEST_DATA("golden_packed_fields_message", "")
    LOAD_TEST_DATA("google_message1.dat", "")
    LOAD_TEST_DATA("google_message1.dat", ".with_fields")
    LOAD_TEST_DATA("google_message2.dat", "")
    LOAD_TEST_DATA("google_message2.dat", ".with_fields")

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

    // Write the result to files for optional post-mortem investigations.
    if (!outputDirPath.isNull()) {
        tools::variant::writeAll(result,
            QString::fromLatin1("%1/%2").arg(outputDirPath)
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    }

    // Compare the result.
    QCOMPARE(result, expected);
}
