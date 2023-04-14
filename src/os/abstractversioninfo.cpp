// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractversioninfo.h"

QString AbstractVersionInfo::fileVersionString(const QChar &sep) const
{
    return fileVersionStrings().join(sep);
}

QString AbstractVersionInfo::fileVersionString(const QString &sep) const
{
    return fileVersionStrings().join(sep);
}

QStringList AbstractVersionInfo::fileVersionStrings() const
{
    QStringList list;
    foreach (const quint16 value, fileVersion()) {
        list.append(QString::fromLatin1("%1").arg(value));
    }
    return list;
}
