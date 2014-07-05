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

#include "testvarint.h"

#include "../../src/protobuf/varint.h"

#include <QTest>

#include <limits>

void TestVarint::parseSignedInt_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    // Examples from https://developers.google.com/protocol-buffers/docs/encoding#types
    QTest::newRow("0")
        << QByteArray("\x00", 1)
        << QVariant(0);
    QTest::newRow("1")
        << QByteArray("\x01")
        << QVariant(-1);
    QTest::newRow("2")
        << QByteArray("\x02")
        << QVariant(1);
    QTest::newRow("3")
        << QByteArray("\x03")
        << QVariant(-2);
    QTest::newRow("4294967294")
        << QByteArray("\xFE\xFF\xFF\xFF\x0F")
        << QVariant(Q_INT64_C(2147483647));
    QTest::newRow("4294967295")
            << QByteArray("\xFF\xFF\xFF\xFF\x0F")
        << QVariant(Q_INT64_C(-2147483648));

    // Some edge cases.
    QTest::newRow("0")
        << QByteArray("\x0", 1)
        << QVariant(0);
    QTest::newRow("sint64::min")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01")
        << QVariant(std::numeric_limits<qint64>::min());
    QTest::newRow("sint64::max")
        << QByteArray("\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01")
        << QVariant(std::numeric_limits<qint64>::max());
}

void TestVarint::parseSignedInt()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseSignedVarint(data), expected);
}

void TestVarint::parseSignedInts_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(0) << QVariant(-1) << QVariant(1) << QVariant(-2);
    QTest::newRow("0;-1;1;-2")
        << QByteArray("\x00\x01\x02\x03", 4)
        << list;

    list.clear();
    list << QVariant(Q_INT64_C(2147483647)) << QVariant(1) << QVariant(Q_INT64_C(-2147483648));
    QTest::newRow("2147483647;1;-2147483648")
        << QByteArray("\xFE\xFF\xFF\xFF\x0F" "\x02" "\xFF\xFF\xFF\xFF\x0F")
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestVarint::parseSignedInts()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseSignedVarints(data), expected);
    QCOMPARE(ProtoBuf::parseSignedVarints(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseSignedVarints(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseSignedVarints(data, size), expected.mid(0, size));
    }
}

void TestVarint::parseUnsignedInt_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    // Examples from https://developers.google.com/protocol-buffers/docs/encoding#varints
    QTest::newRow("0000 0001")
        << QByteArray("\x01")
        << QVariant(1);
    QTest::newRow("1010 1100 0000 0010")
        << QByteArray("\xAC\x02")
        << QVariant(300);

    // Some edge cases.
    QTest::newRow("uint64::min")
        << QByteArray("\x0", 1)
        << QVariant(std::numeric_limits<quint64>::min());
    QTest::newRow("uint63::max") // aka sint64::max
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00", 10)
        << QVariant(std::numeric_limits<qint64>::max());
    QTest::newRow("uint64::max")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01")
        << QVariant(std::numeric_limits<quint64>::max());
    QTest::newRow("uint64::max+1")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x02")
        << QVariant(std::numeric_limits<qint64>::max());
    QTest::newRow("uint64::max+2")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x03")
        << QVariant(std::numeric_limits<quint64>::max());
}

void TestVarint::parseUnsignedInt()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseUnsignedVarint(data), expected);
}

void TestVarint::parseUnsignedInts_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(1) << QVariant(2) << QVariant(3);
    QTest::newRow("1;2;3")
        << QByteArray("\x01\x02\x03")
        << list;

    list.clear();
    list << QVariant(300) << QVariant(1) << QVariant(300);
    QTest::newRow("300;1;300")
        << QByteArray("\xAC\x02" "\x01" "\xAC\x02")
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestVarint::parseUnsignedInts()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseUnsignedVarints(data), expected);
    QCOMPARE(ProtoBuf::parseUnsignedVarints(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseUnsignedVarints(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseUnsignedVarints(data, size), expected.mid(0, size));
    }
}
