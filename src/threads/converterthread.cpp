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

#include "converterthread.h"

#include "gpx/gpxextensionstab.h"
#include "hrm/hrmextensionstab.h"
#include "hrm/generalhrmoptionstab.h"
#include "tcx/generaltcxoptionstab.h"
#include "tcx/tcxextensionstab.h"
#include "trainingsession.h"

#include <QDebug>
#include <QDir>
#include <QSettings>

ConverterThread::ConverterThread(QObject * const parent)
    : QThread(parent), cancelled(false)
{
    memset(&files,    0, sizeof(files));
    memset(&sessions, 0, sizeof(sessions));
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

    // Build the set of file formats to be exported.
    QSettings settings;
    polar::v2::TrainingSession::OutputFormats outputDataFormats;
    if (settings.value(QLatin1String("gpxEnabled")).toBool()) {
        outputDataFormats |= polar::v2::TrainingSession::GpxOutput;
    }
    if (settings.value(QLatin1String("hrmEnabled")).toBool()) {
        outputDataFormats |= polar::v2::TrainingSession::HrmOutput;
    }
    if (settings.value(QLatin1String("tcxEnabled")).toBool()) {
        outputDataFormats |= polar::v2::TrainingSession::TcxOutput;
    }

    // Load the output directory setting (empty == auto).
    const QString outputDir =
        (settings.value(QLatin1String("outputFolderIndex")).toInt() == 0) ?
            QString() : settings.value(QLatin1String("outputFolder")).toString();

    // Check for pre-existing output files.
    const QString outputFileNameFormat =
        settings.value(QLatin1String("outputFileNameFormat")).toString();
    polar::v2::TrainingSession session(baseName);
    setTrainingSessionOptions(&session);
    {
        QStringList outputFileNames = session.getOutputFileNames(
            outputFileNameFormat, outputDataFormats, outputDir);
        bool foundNonExistentOutputFileName = false;
        for (int index = 0;
             (index < outputFileNames.count()) && (!foundNonExistentOutputFileName);
             ++index)
        {
            if (!QFile::exists(outputFileNames.at(index))) {
                foundNonExistentOutputFileName = true;
            }
        }
        if ((!outputFileNames.isEmpty()) && (!foundNonExistentOutputFileName)) {
            sessions.skipped++;
            return; // No need to process this training session.
        }
    }

    // Parse the training session.
    if (!session.parse()) {
        sessions.failed++;
        return;
    }

    // Write the relevant output files.
    bool anyFailed = false;
    if (settings.value(QLatin1String("gpxEnabled")).toBool()) {
        const QString fileName = session.writeGPX(outputFileNameFormat, outputDir);
        if (!fileName.isEmpty()) {
            qDebug() << "Wrote" << QDir::toNativeSeparators(fileName);
            files.written++;
        } else {
            anyFailed = true;
            files.failed++;
        }
    }
    if (settings.value(QLatin1String("hrmEnabled")).toBool()) {
        const QStringList fileNames = session.writeHRM(outputFileNameFormat, outputDir);
        foreach (const QString &fileName, fileNames) {
            qDebug() << "Wrote" << QDir::toNativeSeparators(fileName);
            files.written++;
        }
        const int failedFilesCount = (fileNames.size() - (2 * session.exerciseCount()));
        if (failedFilesCount > 0) {
            anyFailed = true;
            files.failed += failedFilesCount;
        }
    }
    if (settings.value(QLatin1String("tcxEnabled")).toBool()) {
        const QString fileName = session.writeTCX(outputFileNameFormat, outputDir);
        if (!fileName.isEmpty()) {
            qDebug() << "Wrote" << QDir::toNativeSeparators(fileName);
            files.written++;
        } else {
            anyFailed = true;
            files.failed++;
        }
    }
    if (anyFailed) {
        sessions.failed++;
    } else {
        sessions.processed++;
    }
}

void ConverterThread::run()
{
    // Reset counters.
    memset(&files,    0, sizeof(files));
    memset(&sessions, 0, sizeof(sessions));

    // Find the base name of training sessions to consider for processing.
    findSessionBaseNames();

    // Process all found training sessions.
    for (int index = 0; (index < baseNames.size()) && (!cancelled); ++index) {
        emit progress(index);
        proccessSession(baseNames.at(index));
    }
}

void ConverterThread::setTrainingSessionOptions(polar::v2::TrainingSession * const session)
{
    Q_CHECK_PTR(session);

    // The src/widgets/*/*Tabs widgets load/save options from/to QSettings.
    // Here we load from QSettings, and apply to our TrainingSession instance.
    QSettings settings;

    settings.beginGroup(QLatin1String("gpx"));
    session->setGpxOption(polar::v2::TrainingSession::CluetrustGpxDataExtension,
        settings.value(GpxExtensionsTab::CluetrustGpxExtSettingsKey,
                       GpxExtensionsTab::CluetrustGpxExtDefaultSetting).toBool());
    session->setGpxOption(polar::v2::TrainingSession::GarminAccelerationExtension,
        settings.value(GpxExtensionsTab::GarminAccelerationExtSettingsKey,
                       GpxExtensionsTab::GarminAccelerationExtDefaultSetting).toBool());
    session->setGpxOption(polar::v2::TrainingSession::GarminTrackPointExtension,
        settings.value(GpxExtensionsTab::GarminTrackPointExtSettingsKey,
                       GpxExtensionsTab::GarminTrackPointExtDefaultSetting).toBool());
    settings.endGroup();

    settings.beginGroup(QLatin1String("hrm"));
    session->setHrmOption(polar::v2::TrainingSession::RrFiles,
        settings.value(GeneralHrmOptions::ExportRrFilesSettingsKey,
                       GeneralHrmOptions::ExportRrFilesDefaultSetting).toBool());
    session->setHrmOption(polar::v2::TrainingSession::LapNames,
        settings.value(HrmExtensionsTab::LapNamesExtSettingsKey,
                       HrmExtensionsTab::LapNamesExtDefaultSetting).toBool());
    settings.endGroup();

    settings.beginGroup(QLatin1String("tcx"));
    session->setTcxOption(polar::v2::TrainingSession::ForceTcxUTC,
        settings.value(GeneralTcxOptions::UtcOnlySettingsKey,
                       GeneralTcxOptions::UtcOnlyDefaultSetting).toBool());
    session->setTcxOption(polar::v2::TrainingSession::GarminActivityExtension,
        settings.value(TcxExtensionsTab::GarminActivityExtSettingsKey,
                       TcxExtensionsTab::GarminActivityExtDefaultSetting).toBool());
    session->setTcxOption(polar::v2::TrainingSession::GarminCourseExtension,
        settings.value(TcxExtensionsTab::GarminCourseExtSettingsKey,
                       TcxExtensionsTab::GarminCourseExtDefaultSetting).toBool());
    settings.endGroup();
}
