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

    // Latitude samples from actual FlowSync data.
    QTest::newRow("mzoo:route:2:1:1")
        << QByteArray("\342es\302q\344B\300")
        << QVariant(-37.784721666666667);

    // Longitude samples from actual FlowSync data.
    QTest::newRow("mzoo:route:3:1:1")
        << QByteArray("\242\336y\317n\036b@")
        << QVariant(144.951026666666667);
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

    // Latitude samples from actual FlowSync data.
    list.clear();
    list << -37.7847216666666667 << -37.7847216666666667 << -37.7847416666666667
         << -37.7847533333333333 << -37.7847583333333333 << -37.7847633333333333
         << -37.78477            << -37.784775           << -37.78478
         << -37.784785           << -37.7847916666666667 << -37.7847983333333333
         << -37.7848033333333333 << -37.7848116666666667 << -37.7848183333333333;
    QTest::newRow("mzoo:route:2:1")
        << QByteArray("\342es\302q\344B\300\342es\302q\344B\300"
                      ")\0229jr\344B\300\375\013\027\314r\344B\300"
                      "\017w\010\366r\344B\300 \342\371\037s\344B\300"
                      "\343p\346Ws\344B\300\365\333\327\201s\344B\300"
                      "\006G\311\253s\344B\300\030\262\272\325s\344B\300"
                      "\332@\247\rt\344B\300\235\317\223Et\344B\300"
                      "\257:\205ot\344B\300\"\355l\265t\344B\300"
                      "\344{Y\355t\344B\300")
        << list;

    // Longitude samples from actual FlowSync data.
    list.clear();
    list << 144.951026666666667 << 144.951026666666667 << 144.951021666666667
         << 144.951011666666667 << 144.951001666666667 << 144.95099
         << 144.950976666666667 << 144.95096           << 144.950945
         << 144.95093           << 144.950913333333333 << 144.950898333333333
         << 144.950883333333333 << 144.95087           << 144.950858333333333;
    QTest::newRow("mzoo:route:3:1")
        << QByteArray("\242\336y\317n\036b@\242\336y\317n\036b@"
                      "\335\203\375\304n\036b@T\316\004\260n\036b@"
                      "\313\030\014\233n\036b@V\232\224\202n\036b@"
                      "\365R\236fn\036b@\274y\252Cn\036b@ni5$n\036b"
                      "@!Y\300\004n\036b@\350\177\314\341m\036b@"
                      "\232oW\302m\036b@M_\342\242m\036b"
                      "@\354\027\354\206m\036b@w\231tnm\036b@")
        << list;
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

    // Barometric altitude samples from actual FlowSync data.
    QTest::newRow("mzoo:samples:6:1:1") << QByteArray("UTyB") << QVariant(62.332355f);
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

    // Barometric altitude samples from actual FlowSync data.
    list.clear();
    list << 63.39916992f << 63.39916992f << 62.332355f   << 62.332355f   << 62.332355f
         << 62.332355f   << 62.332355f   << 62.332355f   << 62.332355f   << 62.332355f
         << 62.332355f   << 62.332355f   << 62.332355f   << 62.332355f   << 62.332355f;
    QTest::newRow("mzoo:samples:6:5")
         << QByteArray("\300\230}B\300\230}BUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyBUTyB")
         << list;
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
    ///        Note, currently FlowSync uses no fixed-length ints.
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
