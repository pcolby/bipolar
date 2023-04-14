// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "variant.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QTest>

namespace tools {
namespace variant {

// Replace raw QByteArray data with hex strings. This is needed because the
// JSON export to UTF-8 is inconsitent accross platforms, and accross Qt
// versions unfortunately.  Note, this is done for diagnostic outputs only,
// and does not affect the QVariant comparisons that make up the actual tests.
void sanitize(QVariant &variant) {
    switch (static_cast<QMetaType::Type>(variant.type())) {
    case QMetaType::QByteArray:
        variant = variant.toByteArray().toHex();
        break;
    case QMetaType::QVariantList: {
            QVariantList list;
            foreach (QVariant item, variant.toList()) {
                sanitize(item);
                list << item;
            }
            variant = list;
        }
        break;
    case QMetaType::QVariantMap: {
            QVariantMap map(variant.toMap());
            for (QVariantMap::iterator iter = map.begin(); iter != map.end(); ++iter) {
                sanitize(iter.value());
            }
            variant = map;
        }
        break;
    default:
        ; // Leave variant unmodified.
    }
}

bool writeAll(const QVariantMap &variant, const QString &fileName)
{
    bool result = true;
    if (!writeData(variant, fileName + QLatin1String(".var"))) {
        result = false;
    }
    if (!writeJson(variant, fileName + QLatin1String(".json"))) {
        result = false;
    }
    return result;
}

bool writeData(const QVariantMap &variant, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        return false;
    }
    QDataStream stream(&file);
    stream << variant;
    return true;
}

bool writeJson(const QVariantMap &variant, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        return false;
    }
    QVariant saneVariant(variant);
    sanitize(saneVariant);
    file.write(QJsonDocument::fromVariant(saneVariant).toJson());
    return true;
}

}}
