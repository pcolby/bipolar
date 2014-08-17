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

#include "converterthread.h"

#include "trainingsession.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

ConverterThread::ConverterThread(QObject * const parent)
    : QThread(parent), cancelled(false)
{

}

bool ConverterThread::isCancelled() const
{
    return cancelled;
}

const QStringList &ConverterThread::sessionBaseNames() const
{
    return baseNames;
}

// Public slots.

void ConverterThread::cancel()
{
    cancelled = true;
}

// Protected methods.

void ConverterThread::findSessionBaseNames()
{
    QSettings settings;

    QRegExp regex(QLatin1String("(v2-users-[^-]+-training-sessions-[^-]+)-.*"));
    foreach (const QString &folder,
             settings.value(QLatin1String("inputFolders")).toStringList()) {
        QDir dir(folder);
        foreach (const QFileInfo &info, dir.entryInfoList()) {
            if (regex.exactMatch(info.fileName())) {
                const QString baseName = dir.absoluteFilePath(regex.cap(1));
                if (!baseNames.contains(baseName)) {
                    baseNames.append(baseName);
                }
            }
        }
    }

    emit sessionBaseNamesChanged(baseNames.size());
}

void ConverterThread::proccessSession(const QString &baseName)
{
    if (cancelled) return;
    qDebug() << QDir::toNativeSeparators(baseName);

    // Parse the training session.
    polar::v2::TrainingSession session(baseName);
    if (!session.parse()) {
        return;
    }

    /// @todo
}

void ConverterThread::run()
{
    findSessionBaseNames();
    for (int index = 0; (index < baseNames.size()) && (!cancelled); ++index) {
        emit progress(index);
        proccessSession(baseNames.at(index));
    }
}
