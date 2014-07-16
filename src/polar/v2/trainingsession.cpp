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
#include "types.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

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
    return !parsedExercises.isEmpty();
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

    QMap<QString, QMap<QString, QString> > fileNames;
    const QFileInfo fileInfo(baseName);
    foreach (const QFileInfo &entryInfo, fileInfo.dir().entryInfoList(
             QStringList(fileInfo.fileName() + QLatin1Char('*'))))
    {
        const QStringList nameParts = entryInfo.fileName().split(QLatin1Char('-'));
        if ((nameParts.size() >= 3) && (nameParts.at(nameParts.size() - 3) == QLatin1String("exercises"))) {
            fileNames[nameParts.at(nameParts.size() - 2)][nameParts.at(nameParts.size() - 1)] = entryInfo.filePath();
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
  //PARSE_IF_CONTAINS(STATS,   Stats);
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
        QLatin1String(name), ProtoBuf::Types::type \
    )

QVariantMap TrainingSession::parseLaps(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",        "laps",             EmbeddedMessage);
    ADD_FIELD_INFO("1/1",      "header",           EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1",    "split-time",       EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1/1",  "hours",            Uint32);
    ADD_FIELD_INFO("1/1/1/2",  "mintues",          Uint32);
    ADD_FIELD_INFO("1/1/1/3",  "seconds",          Uint32);
    ADD_FIELD_INFO("1/1/1/4/", "milliseconds",     Uint32);
    ADD_FIELD_INFO("1/1/2",    "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("1/1/2/1",  "hours",            Uint32);
    ADD_FIELD_INFO("1/1/2/2",  "mintues",          Uint32);
    ADD_FIELD_INFO("1/1/2/3",  "seconds",          Uint32);
    ADD_FIELD_INFO("1/1/2/4/", "milliseconds",     Uint32);
    ADD_FIELD_INFO("1/1/3",    "distance",         Float);
    ADD_FIELD_INFO("1/1/4",    "ascent",           Float);
    ADD_FIELD_INFO("1/1/5",    "descent",          Float);
    ADD_FIELD_INFO("1/1/6",    "lap-type",         Enumerator);
    ADD_FIELD_INFO("1/2",      "stats",            EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1",    "heartrate",        EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/2",    "speed",            EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/3",    "cadence",          EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/4",    "power",            EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/5",    "pedaling",         EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/6",    "incline",          EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("1/2/7",    "stride",           EmbeddedMessage); ///< @todo
    ADD_FIELD_INFO("2",        "summary",          EmbeddedMessage);
    ADD_FIELD_INFO("2/1",      "best-duration",    EmbeddedMessage);
    ADD_FIELD_INFO("2/1/1",    "hours",            Uint32);
    ADD_FIELD_INFO("2/1/2",    "mintues",          Uint32);
    ADD_FIELD_INFO("2/1/3",    "seconds",          Uint32);
    ADD_FIELD_INFO("2/1.4",    "milliseconds",     Uint32);
    ADD_FIELD_INFO("2/2",      "average-duration", EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1",    "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2",    "mintues",          Uint32);
    ADD_FIELD_INFO("2/2/3",    "seconds",          Uint32);
    ADD_FIELD_INFO("2/2.4",    "milliseconds",     Uint32);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
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
    ADD_FIELD_INFO("1",     "duration",     Uint32);
    ADD_FIELD_INFO("2",     "latitude",     Double);
    ADD_FIELD_INFO("3",     "longitude",    Double);
    ADD_FIELD_INFO("4",     "altitude",     Sint32);
    ADD_FIELD_INFO("5",     "satellites",   Uint32);
    ADD_FIELD_INFO("9",     "timestamp",    EmbeddedMessage);
    ADD_FIELD_INFO("9/1",   "date",         EmbeddedMessage);
    ADD_FIELD_INFO("9/1/1", "year",         Uint32);
    ADD_FIELD_INFO("9/1/2", "month",        Uint32);
    ADD_FIELD_INFO("9/1/3", "day",          Uint32);
    ADD_FIELD_INFO("9/2",   "time",         EmbeddedMessage);
    ADD_FIELD_INFO("9/2/1", "hour",         Uint32);
    ADD_FIELD_INFO("9/2/2", "minute",       Uint32);
    ADD_FIELD_INFO("9/2/3", "seconds",      Uint32);
    ADD_FIELD_INFO("9/2/4", "milliseconds", Uint32);
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
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",     "record-interval",          EmbeddedMessage);
    ADD_FIELD_INFO("1/1",   "hours",                    Uint32);
    ADD_FIELD_INFO("1/2",   "mintues",                  Uint32);
    ADD_FIELD_INFO("1/3",   "seconds",                  Uint32);
    ADD_FIELD_INFO("1/4",   "milliseconds",             Uint32);
    ADD_FIELD_INFO("2",     "heartrate",                Uint32);
    ADD_FIELD_INFO("3",     "heartrate-offline",        EmbeddedMessage);
    ADD_FIELD_INFO("3/1",   "start-index",              Uint32);
    ADD_FIELD_INFO("3/2",   "stop-index",               Uint32);
    ADD_FIELD_INFO("4",     "cadence",                  Uint32);
    ADD_FIELD_INFO("5",     "cadence-offline",          EmbeddedMessage);
    ADD_FIELD_INFO("5/1",   "start-index",              Uint32);
    ADD_FIELD_INFO("5/2",   "stop-index",               Uint32);
    ADD_FIELD_INFO("6",     "altitude",                 Float);
    ADD_FIELD_INFO("7",     "altitude-calibration",     EmbeddedMessage);
    ADD_FIELD_INFO("7/1",   "start-index",              Uint32);
    ADD_FIELD_INFO("7/2",   "value",                    Float);
    ADD_FIELD_INFO("7/3",   "operation",                Enumerator);
    ADD_FIELD_INFO("7/4",   "cause",                    Enumerator);
    ADD_FIELD_INFO("8",     "temperature",              Float);
    ADD_FIELD_INFO("9",     "speed",                    Float);
    ADD_FIELD_INFO("10",    "speed-offline",            EmbeddedMessage);
    ADD_FIELD_INFO("10/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("10/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("11",    "distance",                 Float);
    ADD_FIELD_INFO("12",    "distance-offline",         EmbeddedMessage);
    ADD_FIELD_INFO("12/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("12/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("13",    "stride-length",            Uint32);
    ADD_FIELD_INFO("14",    "stride-offline",           EmbeddedMessage);
    ADD_FIELD_INFO("14/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("14/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("15",    "stride-calibration",       EmbeddedMessage);
    ADD_FIELD_INFO("15/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("15/2",  "value",                    Float);
    ADD_FIELD_INFO("15/3",  "operation",                Enumerator);
    ADD_FIELD_INFO("15/4",  "cause",                    Enumerator);
    ADD_FIELD_INFO("16",    "fwd-acceleration",         Float);
    ADD_FIELD_INFO("17",    "moving-type",              Enumerator);
    ADD_FIELD_INFO("18",    "altitude-offline",         EmbeddedMessage);
    ADD_FIELD_INFO("18/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("18/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("19",    "temperature-offline",      EmbeddedMessage);
    ADD_FIELD_INFO("19/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("19/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("20",    "fwd-acceleration-offline", EmbeddedMessage);
    ADD_FIELD_INFO("20/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("20/2",  "stop-index",               Uint32);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
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
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",     "heartrate",        EmbeddedMessage);
    ADD_FIELD_INFO("1/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1", "low",              Uint32);
    ADD_FIELD_INFO("1/1/2", "high",             Uint32);
    ADD_FIELD_INFO("1/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("1/2/2", "mintues",          Uint32);
    ADD_FIELD_INFO("1/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("1/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("2",     "power",            EmbeddedMessage);
    ADD_FIELD_INFO("2/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("2/1/1", "low",              Uint32);
    ADD_FIELD_INFO("2/1/2", "high",             Uint32);
    ADD_FIELD_INFO("2/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2", "mintues",          Uint32);
    ADD_FIELD_INFO("2/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("2/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3",     "fatfit",           EmbeddedMessage);
    ADD_FIELD_INFO("3/1",   "limit",            Uint32);
    ADD_FIELD_INFO("3/2",   "fit-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/2/2", "mintues",          Uint32);
    ADD_FIELD_INFO("3/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3/3",   "fat-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/3/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/3/2", "mintues",          Uint32);
    ADD_FIELD_INFO("3/3/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/3.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("4",     "speed",            EmbeddedMessage);
    ADD_FIELD_INFO("4/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("4/1/1", "low",              Float);
    ADD_FIELD_INFO("4/1/2", "high",             Float);
    ADD_FIELD_INFO("4/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("4/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("4/2/2", "mintues",          Uint32);
    ADD_FIELD_INFO("4/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("4/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("4/3",   "distance",         Float);
    ADD_FIELD_INFO("10",    "heartrate-source", Enumerator);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
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

QDomDocument TrainingSession::toGPX() const
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QLatin1String("xml"),
        QLatin1String("version='1.0' encoding='utf-8'")));

    QDomElement gpx = doc.createElement(QLatin1String("gpx"));
    gpx.setAttribute(QLatin1String("version"), QLatin1String("1.1"));
    gpx.setAttribute(QLatin1String("creator"), QString::fromLatin1("%1 %2")
                     .arg(QApplication::applicationName())
                     .arg(QApplication::applicationVersion()));
    gpx.setAttribute(QLatin1String("xmlns"),
                     QLatin1String("http://www.topografix.com/GPX/1/0"));
    gpx.setAttribute(QLatin1String("xmlns:xsi"),
                     QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));
    gpx.setAttribute(QLatin1String("xsi:schemaLocation"),
                     QLatin1String("http://www.topografix.com/GPX/1/1 "
                                   "http://www.topografix.com/GPX/1/1/gpx.xsd"));

    foreach (const QVariant &exercise, parsedExercises) {
        //qDebug() << exercise;
    }

    doc.appendChild(gpx);
    return doc;
}

QDomDocument TrainingSession::toTCX() const
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QLatin1String("xml"),
        QLatin1String("version='1.0' encoding='utf-8'")));

    QDomElement tcx = doc.createElement(QLatin1String("TrainingCenterDatabase"));
    tcx.setAttribute(QLatin1String("xmlns"),
                     QLatin1String("http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2"));
    tcx.setAttribute(QLatin1String("xmlns:xsi"),
                     QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));
    tcx.setAttribute(QLatin1String("xsi:schemaLocation"),
                     QLatin1String("http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 "
                                   "http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd"));

    doc.appendChild(tcx);
    return doc;
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
