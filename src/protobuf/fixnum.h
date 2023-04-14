// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
