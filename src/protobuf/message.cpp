/*
    Copyright 2014 Paul Colby

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

#include "message.h"

#include "fixnum.h"
#include "varint.h"

#include <QBuffer>
#include <QDebug>

namespace ProtoBuf {

Message::Message(const FieldInfoMap &fieldInfo, const QString pathSeparator)
    : fieldInfo(fieldInfo), pathSeparator(pathSeparator)
{
    Q_ASSERT_X(!pathSeparator.isEmpty(), "Message::Message", "pathSeparator should not be empty");
}

QVariantMap Message::parse(QByteArray &data, const QString &tagPathPrefix) const
{
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    return parse(buffer, tagPathPrefix);
}

QVariantMap Message::parse(QIODevice &data, const QString &tagPathPrefix) const
{
    QVariantMap parsedFields;
    while (!data.atEnd()) {
        // Fetch the next field's tag index and wire type.
        QPair<quint32, quint8> tagAndType = parseTagAndType(data);
        if (tagAndType.first == 0) {
            return QVariantMap();
        }

        // If this is a (deprecated) "end group", return the parsed group.
        if (tagAndType.second == 4) {
            return parsedFields;
        }

        // Get the (optional) field name and type hint for this field.
        const QString tagPath = QString::fromLatin1("%1%2").arg(tagPathPrefix).arg(tagAndType.first);
        FieldInfo fieldInfo = this->fieldInfo.value(tagPath); // Note intentional fallback to default-constructed.
        if (fieldInfo.fieldName.isEmpty()) {
            fieldInfo.fieldName = tagPath;
        }

        // Parse the field value.
        const QVariant value = parseValue(data, tagAndType.second, fieldInfo.typeHint, tagPath);
        if (value.isNull()) {
            return QVariantMap();
        }

        // Add the parsed value(s) to the parsed fields map.
        QVariantList list = parsedFields[fieldInfo.fieldName].toList();
        if (static_cast<QMetaType::Type>(value.type()) == QMetaType::QVariantList) {
            list << value.toList();
        } else {
            list << value;
        }
        parsedFields[fieldInfo.fieldName] = list;
    }
    return parsedFields;
}


QPair<quint32, quint8> Message::parseTagAndType(QIODevice &data) const
{
    QVariant tagAndType = parseUnsignedVarint(data);
    return tagAndType.isValid()
        ? QPair<quint32, quint8>(tagAndType.toULongLong() >> 3, tagAndType.toULongLong() & 0x07)
        : QPair<quint32, quint8>(0, 0);
}

template <typename Type>
QVariant Message::parseValue(Type &data, const quint8 wireType, const FieldType typeHint,
                             const QString &tagPath) const
{
    switch (wireType) {
    case 0: // Varint (int32, int64, uint32, uint64, sint32, sint64, bool, enum)
        switch (typeHint) {
        case TypeBoolean:         return parseUnsignedVarint(data);
        case TypeEnumerator:      return parseStandardVarint(data);
        case TypeSignedInteger:   return parseSignedVarint(data);
        case TypeStandardInteger: return parseStandardVarint(data);
        case TypeUnsignedInteger: return parseUnsignedVarint(data);
        default:                  return parseUnsignedVarint(data);
        }
        break;
    case 1: // 64-bit (fixed64, sfixed64, double)
        switch (typeHint) {
        case TypeFloatingPoint:   return parseFixedNumber<double>(data);
        case TypeSignedInteger:   return parseFixedNumber<qint64>(data);
        case TypeUnsignedInteger: return parseFixedNumber<quint64>(data);
        default:                  return data.read(8); // The raw 8-byte sequence.
        }
        break;
    case 2: // Length-delimited (string, bytes, embedded messages, packed repeated fields)
        return parseLengthDelimitedValue(data, wireType, typeHint, tagPath);
    case 3: // Start group (groups (deprecated)
        return parse(data, tagPath + pathSeparator);
    case 4: // End group (groups (deprecated)
        return QVariant(); // Caller will need to end the group started previously.
    case 5: // 32-bit (fixed32, sfixed32, float)
        switch (typeHint) {
        case TypeFloatingPoint:   return parseFixedNumber<float>(data);
        case TypeSignedInteger:   return parseFixedNumber<qint32>(data);
        case TypeUnsignedInteger: return parseFixedNumber<quint32>(data);
        default:                  return data.read(4); // The raw 4-byte sequence.
        }
        break;
    }
    return QVariant();
}

template<typename Type>
QVariant Message::parseLengthDelimitedValue(Type &data, const quint8 wireType,
                                            const FieldType typeHint,
                                            const QString &tagPath) const
{
    const QByteArray value = readLengthDelimitedValue(data);
    if (value.isNull()) {
        return QVariant();
    }

    // Return bytes and strings as-is. For strings, the caller will need to
    // determine / assume the character encoding used in the field.
    if ((typeHint == TypeBytes) || (typeHint == TypeString) || (typeHint == TypeUnknown)) {
        return value;
    }

    // Parse embedded messages recursively.
    if (typeHint == TypeEmbeddedMessage) {
        return parse(data, tagPath + pathSeparator);
    }

    // Parse packed repeated values into a list.
    QVariantList list;
    for (QVariant item(0); item.isValid();) {
        item = parseValue(data, wireType, typeHint, tagPath + pathSeparator);
        if (item.isValid()) {
            list << item;
        }
    }
    return list;
}

template<typename Type>
QByteArray Message::readLengthDelimitedValue(Type &data) const
{
    // Note: We're assuming length-delimited values use unsigned varints for lengths.
    // I haven't found any Protocl Buffers documentation to support / dispute this.
    const QVariant length = parseUnsignedVarint(data);
    if (length.isNull()) {
        return QByteArray();
    }
    const QByteArray value = data.read(length.toULongLong());
    return (value.length() == length.toInt()) ? value : QByteArray();
}

}
