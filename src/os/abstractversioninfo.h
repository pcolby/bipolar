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
