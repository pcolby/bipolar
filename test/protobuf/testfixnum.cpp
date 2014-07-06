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

#include "TestFixnum.h"

#include "../../src/protobuf/fixnum.h"

#include <QTest>

#include <limits>

void TestFixnum::parseDouble_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0.0")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00", 8)
        << QVariant(0.0);

    QTest::newRow("123.456789")
        << QByteArray("\x0B\x0B\xEE\x07\x3C\xDD\x5E\x40")
        << QVariant(123.456789);

    QTest::newRow("-123.456789")
        << QByteArray("\x0B\x0B\xEE\x07\x3C\xDD\x5E\xC0")
        << QVariant(-123.456789);

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseDouble()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<double>(data), expected);
}

void TestFixnum::parseDoubles_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(0) << QVariant(123.456789) << QVariant(-123.456789);
    QTest::newRow("0;-1;1;-2")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00" "\x0B\x0B\xEE\x07\x3C\xDD\x5E\x40" "\x0B\x0B\xEE\x07\x3C\xDD\x5E\xC0", 24)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseDoubles()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<double>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<double>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<double>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<double>(data, size), expected.mid(0, size));
    }
}
