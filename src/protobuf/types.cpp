// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "types.h"

namespace ProtoBuf {

Types::WireType Types::getWireType(const ScalarType scalarType)
{
    switch (scalarType) {
    case Unknown:         break;
    case Double:          return SixtyFourBit;
    case Float:           return ThirtyTwoBit;
    case Int32:           return Varint;
    case Int64:           return Varint;
    case Uint32:          return Varint;
    case Uint64:          return Varint;
    case Sint32:          return Varint;
    case Sint64:          return Varint;
    case Fixed32:         return ThirtyTwoBit;
    case Fixed64:         return SixtyFourBit;
    case Sfixed32:        return ThirtyTwoBit;
    case Sfixed64:        return SixtyFourBit;
    case Bool:            return Varint;
    case String:          return LengthDelimeted;
    case Bytes:           return LengthDelimeted;
    case EmbeddedMessage: return LengthDelimeted;
    case Enumerator:      return Varint;
    case Group:           return StartGroup;
    }
    Q_ASSERT_X(false, "getWireType", "invalid scalar type");
    return Varint; // There's really nothing sensible to return here :|
}

}
