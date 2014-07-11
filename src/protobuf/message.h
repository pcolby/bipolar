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

#ifndef __PROTOBUF_MESSAGE_H__
#define __PROTOBUF_MESSAGE_H__

#include <QByteArray>
#include <QIODevice>
#include <QPair>
#include <QVariantList>

namespace ProtoBuf {

class Message : public QObject {
    Q_OBJECT

public:
    enum FieldType {
        TypeUnknown = 0,
        TypeBoolean,
        TypeBytes,
        TypeEmbeddedMessage,
        TypeEnumerator,
        TypeFloatingPoint,
        TypeSignedInteger,
        TypeStandardInteger,
        TypeString,
        TypeUnsignedInteger,
    };

    struct FieldInfo {
        QString fieldName;
        FieldType typeHint;

        FieldInfo(const QString fieldName = QString(), FieldType typeHint = TypeUnknown)
            : fieldName(fieldName), typeHint(typeHint)
        {

        }
    };

    typedef QMap<QString, FieldInfo> FieldInfoMap;

    Message(const FieldInfoMap &fieldInfo, const QString pathSeparator = QLatin1String("/"));

    QVariantMap parse(QByteArray &data, const QString &tagPathPrefix = QString()) const;
    QVariantMap parse(QIODevice &data, const QString &tagPathPrefix = QString()) const;

protected:
    FieldInfoMap fieldInfo;
    QString pathSeparator;

    QPair<quint32, quint8> parseTagAndType(QIODevice &data) const;

    template<typename Type>
    QVariant parseLengthDelimitedValue(Type &data, const quint8 wireType,
                                       const FieldType typeHint,
                                       const QString &tagPath) const;

    template<typename Type>
    QVariant parseValue(Type &data, const quint8 wireType, const FieldType typeHint,
                        const QString &tagPath) const;

    template<typename Type>
    QByteArray readLengthDelimitedValue(Type &data) const;

};

}

#endif // __PROTOBUF_MESSAGE_H__
