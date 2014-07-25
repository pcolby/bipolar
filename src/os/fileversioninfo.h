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

#ifdef Q_OS_WIN // FileVersionInfo is a Windows-only class.

class FileVersionInfo {

public:

    enum Lang {
        US_ENGLISH = 0x0409, // == 1033
    };

    enum CodePage {
        UTF_16       = 0x04B0, // == 1200
        ANSI_LATIN_1 = 0x04E4, // == 1252
    };

    FileVersionInfo(const QString &fileName = QString());
    ~FileVersionInfo();

    bool isValid() const;

    QString fileInfo(const QString &name, const quint16 lang = US_ENGLISH,
                     const quint16 &codepage = UTF_16) const;

    QList<quint16> fileVersion() const;
    QStringList fileVersionStrings() const;

protected:
    void * versionInfo;

};

#endif // Q_OS_WIN

#endif // __FILE_VERSION_INFO_H__
