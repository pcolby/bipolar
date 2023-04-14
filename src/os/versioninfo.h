// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __VERSION_INFO_H__
#define __VERSION_INFO_H__

#include <QtGlobal>

#ifdef Q_OS_MAC
#include "bundleinfo.h"
typedef BundleInfo VersionInfo;
#endif

#ifdef Q_OS_WIN
#include "fileversioninfo.h"
typedef FileVersionInfo VersionInfo;
#endif

#if !defined Q_OS_MAC && !defined Q_OS_WIN
// Temporary fallback for Linux.
#include "abstractversioninfo.h"
class VersionInfo : public AbstractVersionInfo {

public:
    explicit VersionInfo(const QString &fileName = QString()) { Q_UNUSED(fileName) }
    virtual ~VersionInfo() { }

    virtual bool isValid() const { return false; }

    virtual QString fileInfo(const QString &name) const { Q_UNUSED(name); return QString(); }

    virtual QList<quint16> fileVersion() const { return QList<quint16>(); }
};
#endif

#endif // __VERSION_INFO_H__
