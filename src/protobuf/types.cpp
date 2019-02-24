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
