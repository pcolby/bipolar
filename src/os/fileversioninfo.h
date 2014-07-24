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

#ifndef __FILE_VERSION_INFO_H__
#define __FILE_VERSION_INFO_H__

#include <QList>
#include <QString>
#include <QStringList>

/// @todo  This class should be refactored such that the file info block is
///        loaded once, and can then be queried repeatedly.  Additionally, the
///        constructor should accept the name of the file to read info from,
///        defaulting to "self".

#ifdef Q_OS_WIN // FileVersionInfo is a Windows-only class.

class FileVersionInfo {

public:

    FileVersionInfo(const QString &fileName = QString());
    ~FileVersionInfo();

    bool isValid() const;

    QString fileInfo(const QString &name, const quint16 lang = 0x0409,
                     const quint16 &codepage = 0x04e4) const;

    QList<quint16> fileVersion() const;
    QStringList fileVersionStrings() const;

protected:
    void * versionInfo;

};

#endif // Q_OS_WIN

#endif // __FILE_VERSION_INFO_H__
