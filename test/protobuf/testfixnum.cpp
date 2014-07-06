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

#include "testfixnum.h"

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
    QTest::newRow("0;123.456789;-123.456789")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x0B\x0B\xEE\x07\x3C\xDD\x5E\x40"
                      "\x0B\x0B\xEE\x07\x3C\xDD\x5E\xC0", 24)
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

void TestFixnum::parseFloat_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0.0")
        << QByteArray("\x00\x00\x00\x00", 4)
        << QVariant(0.0f);

    QTest::newRow("123.456")
        << QByteArray("\x79\xE9\xF6\x42")
        << QVariant(123.456f);

    QTest::newRow("-123.456")
        << QByteArray("\x79\xE9\xF6\xC2")
        << QVariant(-123.456f);

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseFloat()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<float>(data), expected);
}

void TestFixnum::parseFloats_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(0) << QVariant(123.456f) << QVariant(-123.456f);
    QTest::newRow("0;123.456;-123.456")
        << QByteArray("\x00\x00\x00\x00" "\x79\xE9\xF6\x42" "\x79\xE9\xF6\xC2", 12)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseFloats()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<float>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<float>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<float>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<float>(data, size), expected.mid(0, size));
    }
}

void TestFixnum::parseSigned32_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0")
        << QByteArray("\x00\x00\x00\x00", 4)
        << QVariant(0);

    QTest::newRow("123456789")
        << QByteArray("\x15\xCD\x5B\x07")
        << QVariant(123456789);

    QTest::newRow("-123456789")
        << QByteArray("\xEB\x32\xA4\xF8")
        << QVariant(-123456789);

    QTest::newRow("min")
        << QByteArray("\x00\x00\x00\x80", 4)
        << QVariant(std::numeric_limits<qint32>::min());

    QTest::newRow("max")
        << QByteArray("\xFF\xFF\xFF\x7F")
        << QVariant(std::numeric_limits<qint32>::max());

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseSigned32()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<qint32>(data), expected);
}

void TestFixnum::parseSigned32s_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(0) << QVariant(123456789) << QVariant(-123456789);
    QTest::newRow("0;123456789;-123456789")
        << QByteArray("\x00\x00\x00\x00" "\x15\xCD\x5B\x07" "\xEB\x32\xA4\xF8", 12)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseSigned32s()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint32>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint32>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint32>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<qint32>(data, size), expected.mid(0, size));
    }
}

void TestFixnum::parseSigned64_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00", 8)
        << QVariant(0);

    QTest::newRow("1234567890123456789")
        << QByteArray("\x15\x81\xE9\x7D\xF4\x10\x22\x11")
        << QVariant(Q_INT64_C(1234567890123456789));

    QTest::newRow("-1234567890123456789")
        << QByteArray("\xEB\x7E\x16\x82\x0B\xEF\xDD\xEE")
        << QVariant(Q_INT64_C(-1234567890123456789));

    QTest::newRow("min")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x80", 8)
        << QVariant(std::numeric_limits<qint64>::min());

    QTest::newRow("max")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F")
        << QVariant(std::numeric_limits<qint64>::max());

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseSigned64()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<qint64>(data), expected);
}

void TestFixnum::parseSigned64s_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(0)
         << QVariant(Q_INT64_C(1234567890123456789))
         << QVariant(Q_INT64_C(-1234567890123456789));
    QTest::newRow("0;1234567890123456789;-1234567890123456789")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x15\x81\xE9\x7D\xF4\x10\x22\x11"
                      "\xEB\x7E\x16\x82\x0B\xEF\xDD\xEE", 24)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseSigned64s()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint64>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint64>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<qint64>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<qint64>(data, size), expected.mid(0, size));
    }
}

void TestFixnum::parseUnsigned32_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0")
        << QByteArray("\x00\x00\x00\x00", 4)
        << QVariant(0);

    QTest::newRow("123456789")
        << QByteArray("\x15\xCD\x5B\x07")
        << QVariant(123456789);

    QTest::newRow("min")
        << QByteArray("\x00\x00\x00\x00", 4)
        << QVariant(std::numeric_limits<quint32>::min());

    QTest::newRow("max")
        << QByteArray("\xFF\xFF\xFF\xFF", 4)
        << QVariant(std::numeric_limits<quint32>::max());

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseUnsigned32()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<quint32>(data), expected);
}

void TestFixnum::parseUnsigned32s_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(std::numeric_limits<quint32>::min())
         << QVariant(123456789)
         << QVariant(std::numeric_limits<quint32>::max());
    QTest::newRow("min;123456789;max")
        << QByteArray("\x00\x00\x00\x00" "\x15\xCD\x5B\x07" "\xFF\xFF\xFF\xFF", 12)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseUnsigned32s()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint32>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint32>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint32>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<quint32>(data, size), expected.mid(0, size));
    }
}

void TestFixnum::parseUnsigned64_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("0")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00", 8)
        << QVariant(0);

    QTest::newRow("1234567890123456789")
        << QByteArray("\x15\x81\xE9\x7D\xF4\x10\x22\x11")
        << QVariant(Q_UINT64_C(1234567890123456789));

    QTest::newRow("min")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00", 8)
        << QVariant(std::numeric_limits<quint64>::min());

    QTest::newRow("max")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF")
        << QVariant(std::numeric_limits<quint64>::max());

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseUnsigned64()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseFixedNumber<quint64>(data), expected);
}

void TestFixnum::parseUnsigned64s_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantList>("expected");

    QVariantList list;

    list.clear();
    list << QVariant(std::numeric_limits<quint64>::min())
         << QVariant(Q_UINT64_C(1234567890123456789))
         << QVariant(std::numeric_limits<quint64>::max());
    QTest::newRow("min;1234567890123456789;max")
        << QByteArray("\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x15\x81\xE9\x7D\xF4\x10\x22\x11"
                      "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 24)
        << list;

    /// @todo  Add some examples from actual FlowSync data too.
}

void TestFixnum::parseUnsigned64s()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint64>(data), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint64>(data, expected.size()), expected);
    QCOMPARE(ProtoBuf::parseFixedNumbers<quint64>(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        QCOMPARE(ProtoBuf::parseFixedNumbers<quint64>(data, size), expected.mid(0, size));
    }
}
