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
