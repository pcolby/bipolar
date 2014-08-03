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

#include "bundleinfo.h"

#include <CFBundle.h>
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
    // Get this application's version string.
    const CFTypeRef ref = CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), kCFBundleVersionKey);
    if ((ref == NULL) || (CFGetTypeID(ref) != CFStringGetTypeID())) {
        return QString(); // Invalid version.
    }

    // Convert this CFStringRef to a QString (emulate Qt's private QCFString::toString function).
    const CFIndex length = CFStringGetLength((CFStringRef)ref);
    const UniChar * const chars = CFStringGetCharactersPtr(static_cast<CFStringRef>(ref));
    if (chars == NULL) {
      QVarLengthArray<UniChar> buffer(length);
      CFStringGetCharacters((CFStringRef)ref, CFRangeMake(0, length), buffer.data());
      return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
    }

    return QString(reinterpret_cast<const QChar *>(chars), length);
}

QString BundleInfo::fileInfo(const QString &name) const
{
    if (!isValid()) {
        return QString();
    }

    /// @todo  Convert name to stringref.

    return getBundleInfo(CFBundleGetMainBundle(), kCFBundleVersionKey);
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
