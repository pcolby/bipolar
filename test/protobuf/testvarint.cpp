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

    // GPS altitude samples from actual FlowSync data.
    list.clear();
    list << 58 << 58 << 55 << 52 << 50 << 48 << 46 << 45 << 44 << 43 << 42 << 41 << 40 << 39 << 39;
    QTest::newRow("mzoo:route:4:first") << QByteArray("ttnhd`\\ZXVTRPNN") << list;
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

void TestVarint::parseStandardInt_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariant>("expected");

    // Examples from https://developers.google.com/protocol-buffers/docs/encoding#varints
    // Assuming that values less than 8-bytes are identical to unsigned varints.
    QTest::newRow("0000 0001")
        << QByteArray("\x01")
        << QVariant(1);
    QTest::newRow("1010 1100 0000 0010")
        << QByteArray("\xAC\x02")
        << QVariant(300);

    // Some edge cases.
    QTest::newRow("0")
        << QByteArray("\x0", 1)
        << QVariant(0);
    QTest::newRow("1")
        << QByteArray("\x01")
        << QVariant(1);
    QTest::newRow("-1")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01")
        << QVariant(-1);
    QTest::newRow("int64::min")
        << QByteArray("\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01")
        << QVariant(std::numeric_limits<qint64>::min());
    QTest::newRow("int64::max")
        << QByteArray("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00", 10)
        << QVariant(std::numeric_limits<qint64>::max());
}

void TestVarint::parseStandardInt()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariant, expected);

    QCOMPARE(ProtoBuf::parseStandardVarint(data), expected);
}

void TestVarint::parseStandardInts_data()
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

    // Duration samples from actual FlowSync data.
    list.clear();
    list << 500 << 1000 << 2000 << 3000 << 4000 << 5000 << 6000 << 7000
         << 8000 << 9000 << 10000 << 11000 << 12000 << 13000 << 14000;
    QTest::newRow("mzoo:route:1:first")
        << QByteArray("\364\003\350\007\320\017\270\027\240\037\210\'\360."
                      "\3306\300>\250F\220N\370U\340]\310e\260m")
        << list;

    list.clear();
    list << 6992000 << 6993000 << 6994000 << 6995000 << 6996000 << 6997000
         << 6998000 << 6999000 << 7000000 << 7001000 << 7002000 << 7003000
         << 7004000;
    QTest::newRow("mzoo:route:1:last")
        << QByteArray("\200\341\252\003\350\350\252\003\320\360\252\003"
                      "\270\370\252\003\240\200\253\003\210\210\253\003"
                      "\360\217\253\003\330\227\253\003\300\237\253\003"
                      "\250\247\253\003\220\257\253\003\370\266\253\003"
                      "\340\276\253\003")
        << list;

    // Satellite counts from actual FlowSync data.
    list.clear();
    list << 10 << 10 << 10 << 10 << 10 << 10 << 10 << 9 << 8 << 8 << 8 << 9 << 8 << 8 << 9;
    QTest::newRow("mzoo:route:5:rnd")
        << QByteArray("\n\n\n\n\n\n\n\t\010\010\010\t\010\010\t")
        << list;
}

void TestVarint::parseStandardInts()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantList, expected);

    // Various ways to parse all items.
    //QCOMPARE(ProtoBuf::parseStandardVarints(data), expected);
    //QCOMPARE(ProtoBuf::parseStandardVarints(data, expected.size()), expected);
    //QCOMPARE(ProtoBuf::parseStandardVarints(data, expected.size() * 2), expected);

    // Parse just first n items, where n is from all items down to none.
    for (int size = expected.size(); size >= 0; --size) {
        //QCOMPARE(ProtoBuf::parseStandardVarints(data, size), expected.mid(0, size));
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

    // Duration samples from actual FlowSync data.
    list.clear();
    list << 500 << 1000 << 2000 << 3000 << 4000 << 5000 << 6000 << 7000
         << 8000 << 9000 << 10000 << 11000 << 12000 << 13000 << 14000;
    QTest::newRow("mzoo:route:1:first")
        << QByteArray("\364\003\350\007\320\017\270\027\240\037\210\'\360."
                      "\3306\300>\250F\220N\370U\340]\310e\260m")
        << list;

    list.clear();
    list << 6992000 << 6993000 << 6994000 << 6995000 << 6996000 << 6997000
         << 6998000 << 6999000 << 7000000 << 7001000 << 7002000 << 7003000
         << 7004000;
    QTest::newRow("mzoo:route:1:last")
        << QByteArray("\200\341\252\003\350\350\252\003\320\360\252\003"
                      "\270\370\252\003\240\200\253\003\210\210\253\003"
                      "\360\217\253\003\330\227\253\003\300\237\253\003"
                      "\250\247\253\003\220\257\253\003\370\266\253\003"
                      "\340\276\253\003")
        << list;

    // Satellite counts from actual FlowSync data.
    list.clear();
    list << 10 << 10 << 10 << 10 << 10 << 10 << 10 << 9 << 8 << 8 << 8 << 9 << 8 << 8 << 9;
    QTest::newRow("mzoo:route:5:rnd")
        << QByteArray("\n\n\n\n\n\n\n\t\010\010\010\t\010\010\t")
        << list;
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
