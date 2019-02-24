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

#ifndef __PROTOBUF_FIXNUM_H__
#define __PROTOBUF_FIXNUM_H__

#include <QByteArray>
#include <QIODevice>
#include <QVariant>

namespace ProtoBuf {

template<typename Type>
QVariant parseFixedNumber(QByteArray &data);

template<typename Type>
QVariant parseFixedNumber(QIODevice &data);

template<typename Type>
QVariantList parseFixedNumbers(QByteArray &data, int maxItems = -1);

template<typename Type>
QVariantList parseFixedNumbers(QIODevice &data, int maxItems = -1);

}

#endif // __PROTOBUF_VARINT_H__
