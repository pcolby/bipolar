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
