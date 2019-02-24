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
