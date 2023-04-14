// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __ABSTRACT_VERSION_INFO_H__
#define __ABSTRACT_VERSION_INFO_H__

#include <QList>
#include <QString>
#include <QStringList>

class AbstractVersionInfo {

public:
    virtual ~AbstractVersionInfo() { };

    virtual bool isValid() const = 0;

    virtual QString fileInfo(const QString &name) const = 0;

    virtual QList<quint16> fileVersion() const = 0;

    virtual QString fileVersionString(const QChar &sep = QLatin1Char('.')) const;
    virtual QString fileVersionString(const QString &sep) const;
    virtual QStringList fileVersionStrings() const;

};

#endif // __ABSTRACT_VERSION_INFO_H__
