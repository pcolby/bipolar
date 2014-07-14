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

#include "message.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QtZlib/zlib.h>

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

bool TrainingSession::isGzipped(const QByteArray &data)
{
    return data.startsWith("\x1f\x8b");
}

bool TrainingSession::isGzipped(QIODevice &data)
{
    return isGzipped(data.peek(2));
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

#define ADD_FIELD_INFO(tag, name, type) \
    fieldInfo[QLatin1String(tag)] = ProtoBuf::Message::FieldInfo( \
        QLatin1String(name), ProtoBuf::Message::Type##type \
    )

QVariantMap TrainingSession::parseLaps(QIODevice &data) const
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseLaps(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open laps file"), fileName);
        return QVariantMap();
    }
    return parseLaps(file);
}

QVariantMap TrainingSession::parseRoute(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
/*    ADD_FIELD_INFO("1",     "duration",     UnsignedInteger);
    ADD_FIELD_INFO("2",     "latitude",     FloatingPoint);
    ADD_FIELD_INFO("3",     "longitude",    FloatingPoint);
    ADD_FIELD_INFO("4",     "altitude",     SignedInteger);
    ADD_FIELD_INFO("5",     "satellites",   UnsignedInteger);
    ADD_FIELD_INFO("9",     "timestamp",    EmbeddedMessage);
    ADD_FIELD_INFO("9/1",   "date",         EmbeddedMessage);
    ADD_FIELD_INFO("9/1/1", "year",         UnsignedInteger);
    ADD_FIELD_INFO("9/1/2", "month",        UnsignedInteger);
    ADD_FIELD_INFO("9/1/3", "day",          UnsignedInteger);
    ADD_FIELD_INFO("9/2",   "time",         EmbeddedMessage);
    ADD_FIELD_INFO("9/2/1", "hour",         UnsignedInteger);
    ADD_FIELD_INFO("9/2/2", "minute",       UnsignedInteger);
    ADD_FIELD_INFO("9/2/3", "seconds",      UnsignedInteger);
    ADD_FIELD_INFO("9/2/4", "milliseconds", UnsignedInteger);*/
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parseRoute(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open route file"), fileName);
        return QVariantMap();
    }
    return parseRoute(file);
}

QVariantMap TrainingSession::parseSamples(QIODevice &data) const
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseSamples(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open samples file"), fileName);
        return QVariantMap();
    }
    return parseSamples(file);
}

QVariantMap TrainingSession::parseZones(QIODevice &data) const
{
    Q_UNUSED(data);
    Q_ASSERT_X(false, __FUNCTION__, "not implemented yet");
    return QVariantMap();
}

QVariantMap TrainingSession::parseZones(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit parseError(tr("failed to open zones file"), fileName);
        return QVariantMap();
    }
    return parseZones(file);
}

QByteArray TrainingSession::unzip(const QByteArray &data,
                                  const int initialBufferSize) const
{
    Q_ASSERT(initialBufferSize > 0);
    QByteArray result;
    result.resize(initialBufferSize);

    // Prepare a zlib stream structure.
    z_stream stream = {};
    stream.next_in = (Bytef *) data.data();
    stream.avail_in = data.length();
    stream.next_out = (Bytef *) result.data();
    stream.avail_out = result.size();
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    // Decompress the data.
    int z_result;
    for (z_result = inflateInit2(&stream, 15 + 32); z_result == Z_OK;) {
        if ((z_result = inflate(&stream, Z_SYNC_FLUSH)) == Z_OK) {
            const int oldSize = result.size();
            result.resize(result.size() * 2);
            stream.next_out = (Bytef *)(result.data() + oldSize);
            stream.avail_out = oldSize;
        }
    }

    // Check for errors.
    if (z_result != Z_STREAM_END) {
        qWarning() << stream.msg;
        emit parseError(tr("zlib error %1: %2").arg(z_result).
                        arg(QLatin1String(stream.msg)));
        return QByteArray();
    }

    // Free any allocated resources.
    if ((z_result = inflateEnd(&stream)) != Z_OK) {
        qWarning() << "inflateEnd returned" << z_result << stream.msg;
    }

    // Return the decompressed data.
    result.chop(stream.avail_out);
    return result;
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
