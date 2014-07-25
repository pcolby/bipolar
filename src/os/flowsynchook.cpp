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

#include "flowsynchook.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <QProcess>
#endif

//#ifdef Q_OS_WIN

//#include <windows.h>

/// @brief Find out where Polar FlowSync is installed.
QDir FlowSyncHook::flowSyncDir()
{
#ifdef Q_OS_WIN
    // Try the %ProgramFiles% style locations first.
    foreach (const QString &env, QProcess::systemEnvironment()) {
        // eg %ProgramFiles%, %ProgramFiles(x86)%, %ProgramW6432%
        if (env.startsWith(QLatin1String("%ProgramFiles"),  Qt::CaseInsensitive) ||
            env.startsWith(QLatin1String("%ProgramW6432%"), Qt::CaseInsensitive)) {
            const QDir dir(env.split(QLatin1Char('=')).at(0) +
                           QLatin1String("/Polar/Polar FlowSync"));
            if ((dir.exists()) && (dir.exists(QLatin1String("Qt5Network.dll")))) {
                qDebug() << "found flowsync" << QDir::toNativeSeparators(dir.path());
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
            qDebug() << "found flowsync" << QDir::toNativeSeparators(dir.path());
            return dir;
        }
    }
#endif

    qWarning() << "failed to locate flowsync";
    return QDir();
}

bool FlowSyncHook::install(const QDir dir)
{
    qWarning() << __FUNCTION__ << "not implemented yet";
    return false;
}

bool FlowSyncHook::isInstalled(const QDir dir)
{
    qWarning() << __FUNCTION__ << "not implemented yet";
    return false;
}
