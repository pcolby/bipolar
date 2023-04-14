// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __PROTOBUF_TYPES_H__
#define __PROTOBUF_TYPES_H__

#include <QByteArray>
#include <QIODevice>
#include <QPair>
#include <QVariantList>

namespace ProtoBuf {

class Types {

public:

    /// @see https://developers.google.com/protocol-buffers/docs/proto#scalar
    enum ScalarType {
        Unknown,
        Double,
        Float,
        Int32,
        Int64,
        Uint32,
        Uint64,
        Sint32,
        Sint64,
        Fixed32,
        Fixed64,
        Sfixed32,
        Sfixed64,
        Bool,
        String,
        Bytes,
        EmbeddedMessage, // Not actually a scalar value.
        Enumerator,      // Same as Int32.
        Group,           // Deprecated.
    };

    /// @see https://developers.google.com/protocol-buffers/docs/encoding#structure
    enum WireType {
        Varint = 0,
        SixtyFourBit = 1,
        LengthDelimeted = 2,
        StartGroup = 3,
        EndGroup = 4,
        ThirtyTwoBit = 5
    };

    static WireType getWireType(const ScalarType scalarType);

};

}

#endif // __PROTOBUF_TYPES_H__
