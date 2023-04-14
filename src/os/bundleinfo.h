// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __BUNDLE_VERSION_INFO_H__
#define __BUNDLE_VERSION_INFO_H__

#include "abstractversioninfo.h"

#ifdef Q_OS_MAC // BundleInfo is a Mac-only class.

class BundleInfo : public AbstractVersionInfo {

public:
    BundleInfo(const QString &fileName = QString());
    virtual ~BundleInfo();

    virtual bool isValid() const;

    virtual QString fileInfo(const QString &name) const;

    virtual QList<quint16> fileVersion() const;

protected:
    bool valid;

};

#endif // Q_OS_MAC

#endif // __BUNDLE_VERSION_INFO_H__
