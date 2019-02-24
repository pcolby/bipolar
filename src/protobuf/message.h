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

#ifndef __PROTOBUF_MESSAGE_H__
#define __PROTOBUF_MESSAGE_H__

#include "types.h"

#include <QByteArray>
#include <QIODevice>
#include <QPair>
#include <QVariantList>

namespace ProtoBuf {

class Message : public QObject {
    Q_OBJECT

public:

    struct FieldInfo {
        QString fieldName;
        Types::ScalarType scalarType;

        FieldInfo(const QString fieldName = QString(),
                  Types::ScalarType scalarType = Types::Unknown)
            : fieldName(fieldName), scalarType(scalarType)
        {

        }

        FieldInfo(Types::ScalarType scalarType, const QString fieldName = QString())
            : fieldName(fieldName), scalarType(scalarType)
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

    QVariant parseLengthDelimitedValue(QIODevice &data,
                                       const Types::ScalarType scalarType,
                                       const QString &tagPath) const;

    QVariant parseValue(QIODevice &data, const quint8 wireType,
                        const Types::ScalarType scalarType,
                        const QString &tagPath) const;

    QVariant readLengthDelimitedValue(QIODevice &data) const;

};

}

#endif // __PROTOBUF_MESSAGE_H__
