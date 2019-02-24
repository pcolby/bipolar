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

#include "fixnum.h"

#include <QBuffer>
#include <QDebug>
#include <QtEndian>

// See also  google/protobuf::src/google/protobuf/stubs/casts.h::bit_cast
template<typename Src, typename Dst> inline Dst copyAligned(const Src src)
{
    Q_STATIC_ASSERT(sizeof(Src) == sizeof(Dst)); // Guarantees alignment.
    Dst dst;
    memcpy(&dst, &src, sizeof(dst));
    return dst;
}

// In Qt 5.7, these the endian-swapping functions were changed to use void
// instead of uchar. See Qt commit 5c2ff22ba117f295718c529198ab42ee4646d90c.
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
#define ENDIAN_SWAPPING_PARAM_TYPE const uchar *
#else
#define ENDIAN_SWAPPING_PARAM_TYPE const void *
#endif

// Template specialisation for double (not included in Qt).
template<> double qFromLittleEndian<double>(ENDIAN_SWAPPING_PARAM_TYPE src)
{
    return copyAligned<quint64, double>(qFromLittleEndian<quint64>(src));
}

// Template specialisation for float (not included in Qt).
template<> float qFromLittleEndian<float>(ENDIAN_SWAPPING_PARAM_TYPE src)
{
    return copyAligned<quint32, float>(qFromLittleEndian<quint32>(src));
}

#undef ENDIAN_SWAPPING_PARAM_TYPE

namespace ProtoBuf {

/// @tparam  Type  May be one of: qint32, quint32, qint64, quint64, float or double.

template<typename Type>
QVariant parseFixedNumber(QByteArray &data) {
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseFixedNumber<Type>(buffer);
}

template<typename Type>
QVariant parseFixedNumber(QIODevice &data)
{
    Q_ASSERT((sizeof(Type) == 4) || (sizeof(Type) == 8));
    const QByteArray array = data.read(sizeof(Type));
    if (array.size() != sizeof(Type)) return QVariant();
    return qFromLittleEndian<Type>(reinterpret_cast<const uchar *>(array.constData()));
}

template<typename Type>
QVariantList parseFixedNumbers(QByteArray &data, int maxItems)
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parseFixedNumbers<Type>(buffer, maxItems);
}

template<typename Type>
QVariantList parseFixedNumbers(QIODevice &data, int maxItems)
{
    QVariantList list;
    for (; (maxItems < 0) || (list.size() < maxItems);) {
        const QVariant item = parseFixedNumber<Type>(data);
        if (item.isValid()) {
            list << item;
        } else {
            maxItems = 0;
        }
    }
    return list;
}

template QVariant parseFixedNumber<double> (QByteArray &);
template QVariant parseFixedNumber<float>  (QByteArray &);
template QVariant parseFixedNumber<qint32> (QByteArray &);
template QVariant parseFixedNumber<qint64> (QByteArray &);
template QVariant parseFixedNumber<quint32>(QByteArray &);
template QVariant parseFixedNumber<quint64>(QByteArray &);

template QVariantList parseFixedNumbers<double> (QByteArray &, int);
template QVariantList parseFixedNumbers<float>  (QByteArray &, int);
template QVariantList parseFixedNumbers<qint32> (QByteArray &, int);
template QVariantList parseFixedNumbers<qint64> (QByteArray &, int);
template QVariantList parseFixedNumbers<quint32>(QByteArray &, int);
template QVariantList parseFixedNumbers<quint64>(QByteArray &, int);

}
