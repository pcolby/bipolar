// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __FILE_VERSION_INFO_H__
#define __FILE_VERSION_INFO_H__

#include "abstractversioninfo.h"

#ifdef Q_OS_WIN // FileVersionInfo is a Windows-only class.

class FileVersionInfo : public AbstractVersionInfo {

public:

    enum Lang {
        US_ENGLISH = 0x0409, // == 1033
    };

    enum CodePage {
        UTF_16       = 0x04B0, // == 1200
        ANSI_LATIN_1 = 0x04E4, // == 1252
    };

    FileVersionInfo(const QString &fileName = QString());
    virtual ~FileVersionInfo();

    virtual bool isValid() const;

    virtual QString fileInfo(const QString &name) const;
    virtual QString fileInfo(const QString &name, const quint16 lang,
                             const quint16 &codepage) const;

    virtual QList<quint16> fileVersion() const;

protected:
    void * versionInfo;

};

#endif // Q_OS_WIN

#endif // __FILE_VERSION_INFO_H__
