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

#include "testtrainingsession.h"

#include "../../src/polar/v2/trainingsession.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QTest>

/// @todo  move this to a separate file.

// Replace raw QByteArray data with hex strings. This is needed because the
// JSON export to UTF-8 is inconsitent accross platforms, and accross Qt
// versions unfortunately.  Note, this is done for diagnostic outputs only,
// and does not affect the QVariant comparisons that make up the actual tests.
void sanitize2(QVariant &variant) {
    switch (static_cast<QMetaType::Type>(variant.type())) {
    case QMetaType::QByteArray:
        variant = variant.toByteArray().toHex();
        break;
    case QMetaType::QVariantList: {
            QVariantList list;
            foreach (QVariant item, variant.toList()) {
                sanitize2(item);
                list << item;
            }
            variant = list;
        }
        break;
    case QMetaType::QVariantMap: {
            QVariantMap map(variant.toMap());
            for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
                sanitize2(iter.value());
            }
            variant = map;
        }
        break;
    default:
        ; // Leave variant unmodified.
    }
}

void TestTrainingSession::isGzipped_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile dataFile(QFINDTESTDATA("testdata/" name ".gz")); \
        dataFile.open(QIODevice::ReadOnly); \
        QFile expectedFile(QFINDTESTDATA("testdata/" name)); \
        expectedFile.open(QIODevice::ReadOnly); \
        QTest::newRow(name) << dataFile.readAll() << true; \
    }

    LOAD_TEST_DATA("lorem-ipsum.txt");
    LOAD_TEST_DATA("random-bytes");

    #undef LOAD_TEST_DATA

    QTest::newRow("empty")   << QByteArray() << false;
    QTest::newRow("invalid") << QByteArray("\x11\x22\x33") << false;
}

void TestTrainingSession::isGzipped()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, expected);

    QCOMPARE(polar::v2::TrainingSession::isGzipped(data), expected);
}

void TestTrainingSession::parseRoute_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QVariantMap>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile expectedFile(QFINDTESTDATA("testdata/" name ".expected.var")); \
        expectedFile.open(QIODevice::ReadOnly); \
        QDataStream expectedStream(&expectedFile); \
        QVariantMap expectedMap; \
        expectedStream >> expectedMap; \
        QTest::newRow(name) << QFINDTESTDATA("testdata/" name) << expectedMap; \
    }

    LOAD_TEST_DATA("training-sessions-1-exercises-1-route");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseRoute()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseRoute(fileName);

    // Write the result to a binary file for optional post-mortem investigation.
#ifdef Q_OS_WIN
    QFile binaryFile(QString::fromLatin1("polar/v2/testdata/%1.expected.var")
#else
    QFile binaryFile(QString::fromLatin1("../polar/v2/testdata/%1.result.var")
#endif
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (binaryFile.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QDataStream stream(&binaryFile);
        stream << result;
        binaryFile.close();
    }

    // Write the result to a JSON file for optional post-mortem investigation.
#ifdef Q_OS_WIN
    QFile jsonFile(QString::fromLatin1("polar/v2/testdata/%1.expected.json")
#else
    QFile jsonFile(QString::fromLatin1("../polar/v2/testdata/%1.result.json")
#endif
                 .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (jsonFile.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QVariant saneResult(result);
        sanitize2(saneResult);
        jsonFile.write(QJsonDocument::fromVariant(saneResult).toJson());
        jsonFile.close();
    }

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::unzip_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile dataFile(QFINDTESTDATA("testdata/" name ".gz")); \
        dataFile.open(QIODevice::ReadOnly); \
        QFile expectedFile(QFINDTESTDATA("testdata/" name)); \
        expectedFile.open(QIODevice::ReadOnly); \
        QTest::newRow(name) << dataFile.readAll() << expectedFile.readAll(); \
    }

    LOAD_TEST_DATA("lorem-ipsum.txt");
    LOAD_TEST_DATA("random-bytes");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::unzip()
{
    QFETCH(QByteArray, data);
    QFETCH(QByteArray, expected);

    QVERIFY2(!data.isEmpty(), "failed to load testdata");
    QVERIFY2(!expected.isEmpty(), "failed to load testdata");

    const polar::v2::TrainingSession session;
    QCOMPARE(session.unzip(data), expected);   // Default initial buffer size.
    QCOMPARE(session.unzip(data,1), expected); // Tiny initial buffer size.
}
