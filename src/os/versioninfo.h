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
