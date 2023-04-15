// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bundleinfo.h"

#include <CoreFoundation/CFBundle.h>
#include <QVarLengthArray>

BundleInfo::BundleInfo(const QString &fileName)
{
    // We could implement support for named bundles, but not needed currently.
    Q_ASSERT_X(fileName.isEmpty(), "BundleInfo", "named bundles not supported");
    valid = fileName.isEmpty();
}

BundleInfo::~BundleInfo()
{

}

bool BundleInfo::isValid() const
{
    return valid;
}

QString getBundleInfo(const CFBundleRef bundle, const CFStringRef &key)
{
    const CFTypeRef ref = CFBundleGetValueForInfoDictionaryKey(bundle, key);
    if ((ref == NULL) || (CFGetTypeID(ref) != CFStringGetTypeID())) {
        return QString(); // Not a string.
    }
    return QString::fromCFString(static_cast<CFStringRef>(ref));
}

QString BundleInfo::fileInfo(const QString &name) const
{
    return (isValid()) ? getBundleInfo(CFBundleGetMainBundle(), name.toCFString()) : QString();
}

QList<quint16> BundleInfo::fileVersion() const
{
    QList<quint16> list;
    if (!isValid()) {
        return list;
    }

    const QString version = getBundleInfo(CFBundleGetMainBundle(), kCFBundleVersionKey);
    foreach (const QString &part, version.split(QLatin1Char('.'))) {
        list.append(part.toUInt());
    }
    return list;
}
