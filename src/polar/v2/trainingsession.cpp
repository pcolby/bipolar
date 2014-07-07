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

#include "trainingsession.h"

#include <QDir>
#include <QFileInfo>

// These constants match those used by Polar's V2 API.
#define LAPS    QLatin1String("laps")
#define ROUTE   QLatin1String("route")
#define SAMPLES QLatin1String("samples")
#define ZONES   QLatin1String("zones")

namespace polar {
namespace v2 {

TrainingSession::TrainingSession(const QString &baseName)
    : baseName(baseName)
{

}

bool TrainingSession::isValid() const
{
    return parsedExercises.isEmpty();
}

bool TrainingSession::parse(const QString &baseName)
{
    parsedExercises.clear();

    if (!baseName.isEmpty()) {
        this->baseName = baseName;
    }

    if (this->baseName.isEmpty()) {
        emit parseError(QLatin1String("parse called with no baseName specified"));
        return false;
    }

    const QFileInfo fileInfo(baseName);
    const QDir dir(fileInfo.baseName(), fileInfo.fileName(), QDir::NoSort, QDir::Readable|QDir::Files);
    QMap<QString, QMap<QString, QString> > fileNames;
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        const QStringList nameParts = fileInfo.fileName().split(QLatin1Char('_'));
        if ((nameParts.size() >= 3) && (nameParts.at(nameParts.size() - 3) == QLatin1String("exercises"))) {
            fileNames[nameParts.at(nameParts.size() - 2)][nameParts.at(nameParts.size() - 1)] = fileInfo.path();
        }
    }

    for (QMap<QString, QMap<QString, QString> >::const_iterator iter = fileNames.constBegin();
         iter != fileNames.constEnd(); ++iter)
    {
        parse(iter.key(), iter.value());
    }

    return isValid();
}

bool TrainingSession::parse(const QString &exerciseId, const QMap<QString, QString> &fileNames)
{
    QVariantMap exercise;
    #define PARSE_IF_CONTAINS(str, Func) \
        if (fileNames.contains(str)) { \
            const QVariantMap map = parse##Func(fileNames[str]); \
            if (!map.empty()) { \
                exercise[str] = map; \
            } \
        }
    PARSE_IF_CONTAINS(LAPS,    Laps);
  //PARSE_IF_CONTAINS(PHASES,  Phases);
    PARSE_IF_CONTAINS(ROUTE,   Route);
    PARSE_IF_CONTAINS(SAMPLES, Samples);
  //PARSE_IF_CONTAINS(SENSORS, Sensors);
  //PARSE_IF_CONTAINS(STATS,   Zones);
    PARSE_IF_CONTAINS(ZONES,   Zones);
    #undef PARSE_IF_CONTAINS

    if (!exercise.empty()) {
        parsedExercises[exerciseId] = exercise;
        return true;
    }
    return false;
}

QVariantMap TrainingSession::parseLaps(QIODevice &data)
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseLaps(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open laps file"), fileName);
    }
    return parseLaps(file);
}

QVariantMap TrainingSession::parseRoute(QIODevice &data)
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseRoute(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open route file"), fileName);
    }
    return parseRoute(file);
}

QVariantMap TrainingSession::parseSamples(QIODevice &data)
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseSamples(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open samples file"), fileName);
    }
    return parseSamples(file);
}

QVariantMap TrainingSession::parseZones(QIODevice &data)
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseZones(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open zones file"), fileName);
    }
    return parseZones(file);
}

bool TrainingSession::writeGPX(const QString &fileName)
{    
    // GPX:
    //  * Multiple <trk> elements.
    //  * No HR, cadence, etc. So really no use without "route".
    Q_UNUSED(fileName);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeGPX(const QIODevice &device)
{
    Q_UNUSED(device);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeGPX(const bool separateFiles)
{
    Q_UNUSED(separateFiles);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QStringList &sport)
{
    // TCX:
    //  * Multiple <Activity> elements.
    //  * Each can track GPS, HR, cadence, alti, sensor states, etc. All optional.
    //  * So either of route, samples would suffice.
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QString &fileName, const QStringList &sport)
{
    Q_UNUSED(fileName);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const QIODevice &device, const QStringList &sport)
{
    Q_UNUSED(device);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

bool TrainingSession::writeTCX(const bool separateFiles, const QStringList &sport)
{
    Q_UNUSED(separateFiles);
    Q_UNUSED(sport);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return false;
}

}}
