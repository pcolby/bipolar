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

#include "varint.h"

#include <QBuffer>
#include <QDebug>

namespace ProtoBuf {

QVariant parseSignedVarint(QByteArray data)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseSignedVarint(buffer);
}

QVariant parseSignedVarint(QIODevice &data)
{
    const QVariant variant = parseUnsignedVarint(data);
    if (!variant.isValid()) {
        return QVariant();
    }
    const quint64 result = variant.toULongLong();
    return static_cast<qint64>(result >> 1) * ((result & 0x1) ? -1 : 1) + ((result & 0x1) ? -1 : 0);
}

QVariantList parseSignedVarints(QByteArray data, int maxItems)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseSignedVarints(buffer, maxItems);
}

QVariantList parseSignedVarints(QIODevice &data, int maxItems)
{
    QVariantList list;
    for (; (maxItems < 0) || (list.size() < maxItems);) {
        const QVariant item = parseSignedVarint(data);
        if (item.isValid()) {
            list << item;
        } else {
            maxItems = 0;
        }
    }
    return list;
}

QVariant parseStandardVarint(QByteArray data)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseStandardVarint(buffer);
}

QVariant parseStandardVarint(QIODevice &data)
{
    const QVariant variant = parseUnsignedVarint(data);
    if (!variant.isValid()) {
        return QVariant();
    }
    return static_cast<qint64>(variant.toULongLong());
}

QVariantList parseStandardVarints(QByteArray data, int maxItems)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseStandardVarints(buffer, maxItems);
}

QVariantList parseStandardVarints(QIODevice &data, int maxItems)
{
    QVariantList list;
    for (; (maxItems < 0) || (list.size() < maxItems);) {
        const QVariant item = parseStandardVarint(data);
        if (item.isValid()) {
            list << item;
        } else {
            maxItems = 0;
        }
    }
    return list;
}

QVariant parseUnsignedVarint(QByteArray data)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseUnsignedVarint(buffer);
}

QVariant parseUnsignedVarint(QIODevice &data)
{
    quint64 result = 0;
    for (uchar byte = 0xFF, index = 0; byte >= 0x80; ++index) {
        const QByteArray array = data.read(1);
        if (array.isEmpty()) return QVariant();
        byte = static_cast<uchar>(array.at(0));
        result += (byte & Q_UINT64_C(0x7F)) << (7 * index);
    }
    return result;
}

QVariantList parseUnsignedVarints(QByteArray data, int maxItems)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseUnsignedVarints(buffer, maxItems);
}

QVariantList parseUnsignedVarints(QIODevice &data, int maxItems)
{
    QVariantList list;
    for (; (maxItems < 0) || (list.size() < maxItems);) {
        const QVariant item = parseUnsignedVarint(data);
        if (item.isValid()) {
            list << item;
        } else {
            maxItems = 0;
        }
    }
    return list;
}

}
