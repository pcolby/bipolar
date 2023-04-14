// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
