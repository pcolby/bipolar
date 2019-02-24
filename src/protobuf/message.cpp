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
            qWarning() << "Invalid tag:" << tagAndType.first;
            return QVariantMap();
        }

        // If this is a (deprecated) "end group", return the parsed group.
        if (tagAndType.second == Types::EndGroup) {
            return parsedFields;
        }

        // Get the (optional) field name and type hint for this field.
        const QString tagPath = QString::fromLatin1("%1%2").arg(tagPathPrefix).arg(tagAndType.first);
        FieldInfo fieldInfo = this->fieldInfo.value(tagPath); // Note intentional fallback to default-constructed.
        if (fieldInfo.fieldName.isEmpty()) {
            fieldInfo.fieldName = QString::fromLatin1("%1").arg(tagAndType.first);
        }

        // Parse the field value.
        const QVariant value = parseValue(data, tagAndType.second, fieldInfo.scalarType, tagPath);
        if (!value.isValid()) {
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

QVariant Message::parseValue(QIODevice &data, const quint8 wireType,
                             const Types::ScalarType scalarType,
                             const QString &tagPath) const
{
    // A small sanity check. In this case, the wireType will take precedence.
    if ((scalarType != Types::Unknown) &&
        (wireType != Types::LengthDelimeted) &&
        (wireType != Types::getWireType(scalarType))) {
        qWarning() << tagPath << "wire type" << wireType << "does not match "
            "expected wire type" << Types::getWireType(scalarType) << "for "
            "scalar type" << scalarType << '.';
    }

    switch (wireType) {
    case Types::Varint: // int32, int64, uint32, uint64, sint32, sint64, bool, enum.
        switch (scalarType) {
        case Types::Int32:      return parseStandardVarint(data);
        case Types::Int64:      return parseStandardVarint(data);
        case Types::Uint32:     return parseUnsignedVarint(data);
        case Types::Uint64:     return parseUnsignedVarint(data);
        case Types::Sint32:     return parseSignedVarint(data);
        case Types::Sint64:     return parseSignedVarint(data);
        case Types::Bool:       return parseStandardVarint(data);
        case Types::Enumerator: return parseStandardVarint(data);
        default:                return parseStandardVarint(data);
        }
        break;
    case Types::SixtyFourBit: // fixed64, sfixed64, double.
        switch (scalarType) {
        case Types::Fixed64:  return parseFixedNumber<quint64>(data);
        case Types::Sfixed64: return parseFixedNumber<qint64>(data);
        case Types::Double:   return parseFixedNumber<double>(data);
        default:              return data.read(8); // The raw 8-byte sequence.
        }
        break;
    case Types::LengthDelimeted: // string, bytes, embedded messages, packed repeated fields.
        return parseLengthDelimitedValue(data, scalarType, tagPath);
    case Types::StartGroup: // deprecated.
        return parse(data, tagPath + pathSeparator);
    case Types::EndGroup: // deprecated.
        return QVariant(); // Caller will need to end the group started previously.
    case Types::ThirtyTwoBit: // fixed32, sfixed32, float.
        switch (scalarType) {
        case Types::Fixed32:  return parseFixedNumber<quint32>(data);
        case Types::Sfixed32: return parseFixedNumber<qint32>(data);
        case Types::Float:    return parseFixedNumber<float>(data);
        default:              return data.read(4); // The raw 4-byte sequence.
        }
        break;
    }
    qWarning() << "Invalid wireType:" << wireType << "(tagPath:" << tagPath << ')';
    return QVariant();
}

QVariant Message::parseLengthDelimitedValue(QIODevice &data,
                                            const Types::ScalarType scalarType,
                                            const QString &tagPath) const
{
    const QVariant value = readLengthDelimitedValue(data);
    if (!value.isValid()) {
        qWarning() << "Failed to read prefix-delimited value.";
        return QVariant();
    }

    // Return bytes and unknowns as-is.
    if ((scalarType == Types::Bytes) || (scalarType == Types::Unknown)) {
        return value;
    }

    // Assume strings are UTF-8, which works fine for Polar data. If other
    // encodings are used, the called should use ScalerType Bytes (or Unknown)
    // and convert to QString upon return. This is also consistent with the
    // `protoc --decode_raw` output.
    if (scalarType == Types::String ) {
        return QString::fromUtf8(value.toByteArray());
    }

    // Parse embedded messages recursively.
    if (scalarType == Types::EmbeddedMessage) {
        QByteArray array = value.toByteArray();
        return parse(array, tagPath + pathSeparator);
    }

    // Parse packed repeated values into a list.
    QVariantList list;
    QByteArray array = value.toByteArray();
    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);
    for (QVariant item(0); item.isValid();) {
        item = parseValue(buffer, Types::getWireType(scalarType), scalarType,
                          tagPath + pathSeparator);
        if (item.isValid()) {
            list << item;
        }
    }
    return list;
}

QVariant Message::readLengthDelimitedValue(QIODevice &data) const
{
    // Note: We're assuming length-delimited values use unsigned varints for lengths.
    // I haven't found any Protocl Buffers documentation to support / dispute this.
    const QVariant length = parseUnsignedVarint(data);
    if (!length.isValid()) {
        qWarning() << "Failed to read prefix-delimited length.";
        return QVariant();
    }
    const QByteArray value = data.read(length.toULongLong());
    return (value.length() == length.toInt()) ? value : QVariant();
}

}
