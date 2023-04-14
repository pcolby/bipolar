// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __PROTOBUF_VARINT_H__
#define __PROTOBUF_VARINT_H__

#include <QByteArray>
#include <QIODevice>
#include <QVariant>

namespace ProtoBuf {

QVariant parseSignedVarint(QByteArray data);
QVariant parseSignedVarint(QIODevice &data);
QVariantList parseSignedVarints(QByteArray data, int maxItems = -1);
QVariantList parseSignedVarints(QIODevice &data, int maxItems = -1);

QVariant parseStandardVarint(QByteArray data);
QVariant parseStandardVarint(QIODevice &data);
QVariantList parseStandardVarints(QByteArray data, int maxItems = -1);
QVariantList parseStandardVarints(QIODevice &data, int maxItems = -1);

QVariant parseUnsignedVarint(QByteArray data);
QVariant parseUnsignedVarint(QIODevice &data);
QVariantList parseUnsignedVarints(QByteArray data, int maxItems = -1);
QVariantList parseUnsignedVarints(QIODevice &data, int maxItems = -1);

}

#endif // __PROTOBUF_VARINT_H__
