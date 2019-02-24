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

#include "flowsynchook.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

#ifdef Q_OS_WIN
#include "fileversioninfo.h"
#include <QProcess>
#include <QSettings>
#endif

/// @brief Find out where Polar FlowSync is installed.
QDir FlowSyncHook::flowSyncDir(bool *found)
{
#ifdef Q_OS_WIN
    // Try the FlowSync installer's registry entries.
    #define REGISTRY_KEY_NAME "%1/Microsoft/Windows/CurrentVersion/Uninstall/{A1538F5C-7B65-4DB6-9FFB-FFC0DF2E85D8}_is1"
    QStringList keyPrefixes;
    keyPrefixes << QLatin1String("HKEY_CURRENT_USER/Software")
                << QLatin1String("HKEY_CURRENT_USER/Software/WOW6432Node")
                << QLatin1String("HKEY_LOCAL_MACHINE/Software")
                << QLatin1String("HKEY_LOCAL_MACHINE/Software/WOW6432Node");
    foreach (const QString &keyPrefix, keyPrefixes) {
        const QString keyName = QString::fromLatin1(REGISTRY_KEY_NAME).arg(keyPrefix);
        const QSettings settings(keyName, QSettings::NativeFormat);
        const QVariant installLocation = settings.value(QLatin1String("InstallLocation"));
        if (installLocation.isValid()) {
            qDebug() << keyName << installLocation;
            const QDir dir(installLocation.toString());
            if ((dir.exists()) && (dir.exists(QLatin1String("Qt5Network.dll")))) {
                if (found != NULL) *found = true;
                return dir;
            }
        }
    }
    #undef REGISTRY_KEY_NAME

    // Try the standard %ProgramFiles% style locations.
    foreach (const QString &env, QProcess::systemEnvironment()) {
        // eg %ProgramFiles%, %ProgramFiles(x86)%, %ProgramW6432%
        if (env.startsWith(QLatin1String("%ProgramFiles"),  Qt::CaseInsensitive) ||
            env.startsWith(QLatin1String("%ProgramW6432%"), Qt::CaseInsensitive)) {
            const QDir dir(env.split(QLatin1Char('=')).at(0) +
                           QLatin1String("/Polar/Polar FlowSync"));
            if ((dir.exists()) && (dir.exists(QLatin1String("Qt5Network.dll")))) {
                if (found != NULL) *found = true;
                return dir;
            }
        }
    }

    // Fall back to a list of known paths.
    QStringList knownPaths;
    knownPaths << QLatin1String("C:/Program Files")
               << QLatin1String("C:/Program Files (x86)");
    foreach (const QString &path, knownPaths) {
        const QDir dir(path + QLatin1String("/Polar/Polar FlowSync"));
        if ((dir.exists()) && (dir.exists(QLatin1String("Qt5Network.dll")))) {
            if (found != NULL) *found = true;
            return dir;
        }
    }
#endif

    qWarning() << "failed to locate flowsync";
    if (found != NULL) *found = false;
    return QDir();
}

QDir FlowSyncHook::installableHookDir()
{
#ifdef Q_OS_WIN
    return QDir(QCoreApplication::applicationDirPath() + QLatin1String("/hook"));
#endif
}

bool FlowSyncHook::install(const QDir &fromDir, QDir toDir)
{
    // Backup the existing DLL.
    const QString backupFileName = QString::fromLatin1("Qt5Network.dll.%1")
        .arg(QDateTime::currentDateTime().toString(QLatin1String("yyyyMMddHHmmss")));
    if (!toDir.rename(QLatin1String("Qt5Network.dll"), backupFileName)) {
        qWarning() << "Failed to backup "
            << QDir::toNativeSeparators(toDir.filePath(QLatin1String("Qt5Network.dll")))
            << "to" << QDir::toNativeSeparators(toDir.filePath(backupFileName));
        return false;
    }

    // Copy our hook DLL into place.
    if (!QFile::copy(fromDir.filePath(QLatin1String("Qt5Network.dll")),
                     toDir.filePath(QLatin1String("Qt5Network.dll")))) {
        qWarning() << "Failed to copy"
            << QDir::toNativeSeparators(fromDir.filePath(QLatin1String("Qt5Network.dll")))
            << "to" << QDir::toNativeSeparators(toDir.filePath(QLatin1String("Qt5Network.dll")));
        return false;
    }
    return true;
}

int FlowSyncHook::getVersion(const QDir &dir)
{
    const QString dll = dir.absoluteFilePath(QLatin1String("Qt5Network.dll"));

    const FileVersionInfo info(dll);
    if (!info.isValid()) {
        qWarning() << "Unabled to read version information for"
                   << QDir::toNativeSeparators(dll);
        return -1;
    }

    const QString internalName = info.fileInfo(QLatin1String("InternalName"));
    if (!internalName.startsWith(QLatin1String("Bipolar Hook"))) {
        return -2; // Qt5Network.dll is not our hooked version.
    }

    const QStringList parts = internalName.split(QLatin1Char(' '), QString::SkipEmptyParts);
    if (parts.length() < 3) {
        qWarning() << "Invalid internal hook version:" << parts;
        return -3;
    } else if (parts.length() > 3) {
        qWarning() << "Ignoring trailing hook version information:" << parts.mid(3);
    }
    return parts.at(2).toInt();
}
