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
#include "../../tools/variant.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QTest>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

// Qt's QDomDocument comparisons are based on references, and always fail when
// comparing two separate documents.  Additionally, the QDomDocument::toString
// and toByteArray functions are both non-deterministic. Hence we have to roll
// our own QDomDocument comparison functions :(

void compare(const QDomNode &a, const QDomNode &b);

void compare(const QDomNode &a, const QDomNamedNodeMap &b)
{
    if (a.namespaceURI().isEmpty()) {
        compare(a, b.namedItem(a.nodeName()));
    } else {
        compare(a, b.namedItemNS(a.namespaceURI(), a.localName()));
    }
}

void compare(const QDomNamedNodeMap &a, const QDomNamedNodeMap &b)
{
    QCOMPARE(a.length(), b.length());
    for (int i = 0; i < a.length(); ++i) {
        compare(a.item(i), b);
        compare(b.item(i), a);
    }
}

void compare(const QDomNodeList &a, const QDomNodeList &b)
{
    QCOMPARE(a.length(), b.length());
    for (int i = 0; i < a.length(); ++i) {
        compare(a.at(i), b.at(i));
    }
}

void fuzzyCompare(const QString &a, const QString &b, bool &compared)
{
    bool aOK, bOK;
    const double aDouble = a.toDouble(&aOK);
    const double bDouble = b.toDouble(&bOK);
    if (aOK && bOK) {
        compared = true;
        QCOMPARE(aDouble, bDouble);
    }
}

void compare(const QDomNode &a, const QDomNode &b)
{
    compare(a.attributes(), b.attributes());
    compare(a.childNodes(), b.childNodes());
    QCOMPARE(a.localName(), b.localName());
    QCOMPARE(a.namespaceURI(), b.namespaceURI());
    QCOMPARE(a.nodeName(), b.nodeName());
    QCOMPARE(a.nodeType(), b.nodeType());
    bool compared = false;
    fuzzyCompare(a.nodeValue(), b.nodeValue(), compared);
    if (!compared) {
        QCOMPARE(a.nodeValue(), b.nodeValue());
    }
    QCOMPARE(a.prefix(), b.prefix());
}

void compare(const QDomDocumentType &a, const QDomDocumentType &b)
{
    compare(static_cast<const QDomNode &>(a), static_cast<const QDomNode &>(b));
    compare(a.entities(), b.entities());
    QCOMPARE(a.internalSubset(), b.internalSubset());
    QCOMPARE(a.name(), b.name());
    QCOMPARE(a.nodeType(), b.nodeType());
    compare(a.notations(), b.notations());
    QCOMPARE(a.publicId(), b.publicId());
    QCOMPARE(a.systemId(), b.systemId());
}

void compare(const QDomDocument &a, const QDomDocument &b)
{
    compare(a.doctype(), b.doctype());
    compare(a.documentElement(), b.documentElement());
    QCOMPARE(a.nodeType(), b.nodeType());
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

void TestTrainingSession::parseCreateExercise_data()
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

    LOAD_TEST_DATA("training-sessions-19946380-exercises-19896844-create");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseCreateExercise()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseCreateExercise(fileName);

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
            .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::parseCreateSession_data()
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

    LOAD_TEST_DATA("training-sessions-19946380-create");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseCreateSession()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseCreateSession(fileName);

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
            .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::parseLaps_data()
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

    LOAD_TEST_DATA("training-sessions-3-exercises-1-laps");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseLaps()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseLaps(fileName);

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
            .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
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
    LOAD_TEST_DATA("training-sessions-19946380-exercises-19896844-route");

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

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
            .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::parseSamples_data()
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

    LOAD_TEST_DATA("training-sessions-1-exercises-1-samples");
    LOAD_TEST_DATA("training-sessions-2-exercises-1-samples");
    LOAD_TEST_DATA("training-sessions-19946380-exercises-19896844-samples");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseSamples()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseSamples(fileName);

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
             .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::parseZones_data()
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

    LOAD_TEST_DATA("training-sessions-2-exercises-1-zones");
    LOAD_TEST_DATA("training-sessions-19946380-exercises-19896844-zones");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::parseZones()
{
    QFETCH(QString, fileName);
    QFETCH(QVariantMap, expected);

    QVERIFY2(!fileName.isEmpty(), "failed to find testdata");

    // Parse the route (protobuf) message.
    const polar::v2::TrainingSession session;
    const QVariantMap result = session.parseZones(fileName);

    // Write the result to files for optional post-mortem investigations.
    tools::variant::writeAll(result,
        QString::fromLatin1("polar/v2/testdata/%1.result")
            .arg(QString::fromLatin1(QTest::currentDataTag())));

    // Compare the result.
    QCOMPARE(result, expected);
}

void TestTrainingSession::toGPX_data()
{
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<QByteArray>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile expectedFile(QFINDTESTDATA("testdata/" name ".gpx")); \
        QString baseName(expectedFile.fileName()); \
        baseName.chop(4); \
        expectedFile.open(QIODevice::ReadOnly); \
        QTest::newRow(name) << baseName << expectedFile.readAll(); \
    }

    LOAD_TEST_DATA("training-sessions-1");
    LOAD_TEST_DATA("training-sessions-2");
    LOAD_TEST_DATA("training-sessions-3");
    LOAD_TEST_DATA("training-sessions-19946380");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::toGPX()
{
    QFETCH(QString, baseName);
    QFETCH(QByteArray, expected);

    // Parse the route (protobuf) message.
    polar::v2::TrainingSession session(baseName);
    QVERIFY(session.parse(baseName));
    QDomDocument gpx = session.toGPX(QDateTime::fromString(
        QLatin1String("2014-07-15T12:34:56Z"), Qt::ISODate));

    // Write the result to an XML file for optional post-mortem investigations.
#ifdef Q_OS_WIN
    QFile file(QString::fromLatin1("polar/v2/testdata/%1.result.gpx")
#else
    QFile file(QString::fromLatin1("../polar/v2/testdata/%1.result.gpx")
#endif
        .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        file.write(gpx.toByteArray(2));
    }
    file.close();

    // Compare the generated document against the expected result.
    QDomDocument expectedDoc;
    expectedDoc.setContent(expected);
    compare(gpx, expectedDoc);

    // Validate the generated document against the relevant XML schema.
    gpx.documentElement().removeAttribute(QLatin1String("xsi:schemaLocation"));
    QFile xsd(QFINDTESTDATA("schemata/gpx.xsd"));
    QVERIFY(xsd.open(QIODevice::ReadOnly));
    QXmlSchema schema;
    QVERIFY(schema.load(&xsd, QUrl::fromLocalFile(xsd.fileName())));
    QXmlSchemaValidator validator(schema);
    QVERIFY(validator.validate(gpx.toByteArray()));
}

void TestTrainingSession::toHRM_data()
{
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<QStringList>("expected");

    #define LOAD_TEST_DATA(name, expectedCount) { \
        QString baseName; \
        QStringList expected; \
        for (int count = 0; count < expectedCount; ++count) { \
            QFile expectedFile(QFINDTESTDATA( \
                QString::fromLatin1("testdata/" name ".%1.hrm").arg(count))); \
            expectedFile.open(QIODevice::ReadOnly); \
            expected.append(QString::fromLatin1(expectedFile.readAll())); \
            if (baseName.isEmpty()) { \
                baseName = expectedFile.fileName(); \
                baseName.chop(6); \
            } \
        } \
        QTest::newRow(name) << baseName << expected; \
    }

    LOAD_TEST_DATA("training-sessions-19946380", 1);

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::toHRM()
{
    QFETCH(QString, baseName);
    QFETCH(QStringList, expected);

    // Parse the route (protobuf) message.
    polar::v2::TrainingSession session(baseName);
    QVERIFY(session.parse(baseName));
    const QStringList hrm = session.toHRM();

    // Write the result to a text file for optional post-mortem investigations.
    for (int index = 0; index < hrm.length(); ++index) {
#ifdef Q_OS_WIN
        QFile file(QString::fromLatin1("polar/v2/testdata/%1.result.%2.hrm")
#else
        QFile file(QString::fromLatin1("../polar/v2/testdata/%1.result.%2.hrm")
#endif
            .arg(QString::fromLatin1(QTest::currentDataTag())).arg(index));
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
            file.write(hrm.at(index).toLatin1());
        }
    }

    // Compare the generated HRM string against the expected result.
    QCOMPARE(hrm, expected);
}

void TestTrainingSession::toTCX_data()
{
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<QByteArray>("expected");

    #define LOAD_TEST_DATA(name) { \
        QFile expectedFile(QFINDTESTDATA("testdata/" name ".tcx")); \
        QString baseName(expectedFile.fileName()); \
        baseName.chop(4); \
        expectedFile.open(QIODevice::ReadOnly); \
        QTest::newRow(name) << baseName << expectedFile.readAll(); \
    }

    LOAD_TEST_DATA("training-sessions-1");
    LOAD_TEST_DATA("training-sessions-2");
    LOAD_TEST_DATA("training-sessions-3");
    LOAD_TEST_DATA("training-sessions-19946380");

    #undef LOAD_TEST_DATA
}

void TestTrainingSession::toTCX()
{
    QFETCH(QString, baseName);
    QFETCH(QByteArray, expected);

    // Parse the route (protobuf) message.
    polar::v2::TrainingSession session(baseName);
    QVERIFY(session.parse(baseName));
    QDomDocument tcx = session.toTCX(QLatin1String("Jul 17 2014 21:02:38"));

    // Write the result to an XML file for optional post-mortem investigations.
#ifdef Q_OS_WIN
    QFile file(QString::fromLatin1("polar/v2/testdata/%1.result.tcx")
#else
    QFile file(QString::fromLatin1("../polar/v2/testdata/%1.result.tcx")
#endif
        .arg(QString::fromLatin1(QTest::currentDataTag())));
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        file.write(tcx.toByteArray(2));
    }
    file.close();

    // Compare the generated document against the expected result.
    QDomDocument expectedDoc;
    expectedDoc.setContent(expected);
    compare(tcx, expectedDoc);

    // Validate the generated document against the relevant XML schema.
    tcx.documentElement().removeAttribute(QLatin1String("xsi:schemaLocation"));
    QFile xsd(QFINDTESTDATA("schemata/TrainingCenterDatabasev2.xsd"));
    QVERIFY(xsd.open(QIODevice::ReadOnly));
    QXmlSchema schema;
    QVERIFY(schema.load(&xsd, QUrl::fromLocalFile(xsd.fileName())));
    QXmlSchemaValidator validator(schema);
    QVERIFY(validator.validate(tcx.toByteArray()));
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
