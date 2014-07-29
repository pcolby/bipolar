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
#include "os/fileversioninfo.h"
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

// These constants match those used by Polar's V2 API.
#define CREATE  QLatin1String("create")
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

/// @see https://github.com/pcolby/bipolar/wiki/Polar-Sport-Types
QString TrainingSession::getTcxSport(const quint64 &polarSportValue)
{
    #define TCX_RUNNING QLatin1String("Running")
    #define TCX_BIKING  QLatin1String("Biking")
    #define TCX_OTHER   QLatin1String("Other")
    static QMap<quint64, QString> map;
    if (map.isEmpty()) {
        map.insert( 1, TCX_RUNNING); // Running
        map.insert( 2, TCX_BIKING);  // Cycling
        map.insert( 3, TCX_OTHER);   // Walking
        map.insert( 4, TCX_OTHER);   // Jogging
        map.insert( 5, TCX_BIKING);  // Mountain biking
        map.insert( 6, TCX_OTHER);   // Skiing
        map.insert( 7, TCX_OTHER);   // Downhill skiing
        map.insert( 8, TCX_OTHER);   // Rowing
        map.insert( 9, TCX_OTHER);   // Nordic walking
        map.insert(10, TCX_OTHER);   // Skating
        map.insert(11, TCX_OTHER);   // Hiking
        map.insert(12, TCX_OTHER);   // Tennis
        map.insert(13, TCX_OTHER);   // Squash
        map.insert(14, TCX_OTHER);   // Badminton
        map.insert(15, TCX_OTHER);   // Strength training
        map.insert(16, TCX_OTHER);   // Other outdoor
        map.insert(17, TCX_RUNNING); // Treadmill running
        map.insert(18, TCX_BIKING);  // Indoor cycling
        map.insert(19, TCX_RUNNING); // Road running
        map.insert(20, TCX_OTHER);   // Circuit training
        map.insert(22, TCX_OTHER);   // Snowboarding
        map.insert(23, TCX_OTHER);   // Swimming
        map.insert(24, TCX_OTHER);   // Freestyle XC skiing
        map.insert(25, TCX_OTHER);   // Classic XC skiing
        map.insert(27, TCX_RUNNING); // Trail running
        map.insert(28, TCX_OTHER);   // Ice skating
        map.insert(29, TCX_OTHER);   // Inline skating
        map.insert(30, TCX_OTHER);   // Roller skating
        map.insert(32, TCX_OTHER);   // Group exercise
        map.insert(33, TCX_OTHER);   // Yoga
        map.insert(34, TCX_OTHER);   // Crossfit
        map.insert(35, TCX_OTHER);   // Golf
        map.insert(36, TCX_RUNNING); // Track&field running
        map.insert(38, TCX_BIKING);  // Road biking
        map.insert(39, TCX_OTHER);   // Soccer
        map.insert(40, TCX_OTHER);   // Cricket
        map.insert(41, TCX_OTHER);   // Basketball
        map.insert(42, TCX_OTHER);   // Baseball
        map.insert(43, TCX_OTHER);   // Rugby
        map.insert(44, TCX_OTHER);   // Field hockey
        map.insert(45, TCX_OTHER);   // Volleyball
        map.insert(46, TCX_OTHER);   // Ice hockey
        map.insert(47, TCX_OTHER);   // Football
        map.insert(48, TCX_OTHER);   // Handball
        map.insert(49, TCX_OTHER);   // Beach volley
        map.insert(50, TCX_OTHER);   // Futsal
        map.insert(51, TCX_OTHER);   // Floorball
        map.insert(52, TCX_OTHER);   // Dancing
        map.insert(53, TCX_OTHER);   // Trotting
        map.insert(54, TCX_OTHER);   // Riding
        map.insert(55, TCX_OTHER);   // Cross-trainer
        map.insert(56, TCX_OTHER);   // Fitness martial arts
        map.insert(57, TCX_OTHER);   // Functional training
        map.insert(58, TCX_OTHER);   // Bootcamp
        map.insert(59, TCX_OTHER);   // Freestyle roller skiing
        map.insert(60, TCX_OTHER);   // Classic roller skiing
        map.insert(61, TCX_OTHER);   // Aerobics
        map.insert(62, TCX_OTHER);   // Aqua fitness
        map.insert(63, TCX_OTHER);   // Step workout
        map.insert(64, TCX_OTHER);   // Body&amp;Mind
        map.insert(65, TCX_OTHER);   // Pilates
        map.insert(66, TCX_OTHER);   // Stretching
        map.insert(67, TCX_OTHER);   // Fitness dancing
        map.insert(68, TCX_OTHER);   // Triathlon
        map.insert(69, TCX_OTHER);   // Duathlon
        map.insert(70, TCX_OTHER);   // Off-road triathlon
        map.insert(71, TCX_OTHER);   // Off-road duathlon
        map.insert(82, TCX_OTHER);   // Multisport
        map.insert(83, TCX_OTHER);   // Other indoor
    }
    QMap<quint64, QString>::ConstIterator iter = map.constFind(polarSportValue);
    if (iter == map.constEnd()) {
        qWarning() << "unknown polar sport value" << polarSportValue;
    }
    return (iter == map.constEnd()) ? TCX_OTHER : iter.value();
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
        Q_ASSERT(!baseName.isEmpty());
        qWarning() << "parse called with no baseName specified";
        return false;
    }

    parsedSession = parseCreateSession(this->baseName + QLatin1String("-create"));

    QMap<QString, QMap<QString, QString> > fileNames;
    const QFileInfo fileInfo(this->baseName);
    foreach (const QFileInfo &entryInfo, fileInfo.dir().entryInfoList(
             QStringList(fileInfo.fileName() + QLatin1String("-*"))))
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
    QVariantList sources;
    #define PARSE_IF_CONTAINS(str, Func) \
        if (fileNames.contains(str)) { \
            const QVariantMap map = parse##Func(fileNames.value(str)); \
            if (!map.empty()) { \
                exercise[str] = map; \
                sources << fileNames.value(str); \
            } \
        }
    PARSE_IF_CONTAINS(CREATE,  CreateExercise);
    PARSE_IF_CONTAINS(LAPS,    Laps);
  //PARSE_IF_CONTAINS(PHASES,  Phases);
    PARSE_IF_CONTAINS(ROUTE,   Route);
    PARSE_IF_CONTAINS(SAMPLES, Samples);
  //PARSE_IF_CONTAINS(SENSORS, Sensors);
  //PARSE_IF_CONTAINS(STATS,   Stats);
    PARSE_IF_CONTAINS(ZONES,   Zones);
    #undef PARSE_IF_CONTAINS

    if (!exercise.empty()) {
        exercise[QLatin1String("sources")] = sources;
        parsedExercises[exerciseId] = exercise;
        return true;
    }
    return false;
}

#define ADD_FIELD_INFO(tag, name, type) \
    fieldInfo[QLatin1String(tag)] = ProtoBuf::Message::FieldInfo( \
        QLatin1String(name), ProtoBuf::Types::type \
    )

QVariantMap TrainingSession::parseCreateExercise(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",     "start",         EmbeddedMessage);
    ADD_FIELD_INFO("1/1",   "date",          EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1", "year",          Uint32);
    ADD_FIELD_INFO("1/1/2", "month",         Uint32);
    ADD_FIELD_INFO("1/1/3", "day",           Uint32);
    ADD_FIELD_INFO("1/2",   "time",          EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1", "hour",          Uint32);
    ADD_FIELD_INFO("1/2/2", "minute",        Uint32);
    ADD_FIELD_INFO("1/2/3", "seconds",       Uint32);
    ADD_FIELD_INFO("1/2/4", "milliseconds",  Uint32);
    ADD_FIELD_INFO("1/4",   "offset",        Int32);
    ADD_FIELD_INFO("2",     "duration",      EmbeddedMessage);
    ADD_FIELD_INFO("2/1",   "hours",         Uint32);
    ADD_FIELD_INFO("2/2",   "minutes",       Uint32);
    ADD_FIELD_INFO("2/3",   "seconds",       Uint32);
    ADD_FIELD_INFO("2/4",   "milliseconds",  Uint32);
    ADD_FIELD_INFO("3",     "sport",         EmbeddedMessage);
    ADD_FIELD_INFO("3/1",   "value",         Uint64);
    ADD_FIELD_INFO("4",     "distance",      Float);
    ADD_FIELD_INFO("5",     "calories",      Uint32);
    ADD_FIELD_INFO("6",     "training-load", EmbeddedMessage);
    ADD_FIELD_INFO("6/1",   "load-value",    Uint32);
    ADD_FIELD_INFO("6/2",   "recovery-time", EmbeddedMessage);
    ADD_FIELD_INFO("6/2/1", "hours",         Uint32);
    ADD_FIELD_INFO("6/2/2", "minutes",       Uint32);
    ADD_FIELD_INFO("6/2/3", "seconds",       Uint32);
    ADD_FIELD_INFO("6/2/4", "milliseconds",  Uint32);
    ADD_FIELD_INFO("6/3",   "carbs",         Uint32);
    ADD_FIELD_INFO("6/4",   "protein",       Uint32);
    ADD_FIELD_INFO("6/5",   "fat",           Uint32);
    ADD_FIELD_INFO("7",     "sensors",       Enumerator);
    ADD_FIELD_INFO("9",     "running-index", EmbeddedMessage);
    ADD_FIELD_INFO("9/1",   "value",         Uint32);
    ADD_FIELD_INFO("9/2",   "duration",      EmbeddedMessage);
    ADD_FIELD_INFO("9/2/1", "hours",         Uint32);
    ADD_FIELD_INFO("9/2/2", "minutes",       Uint32);
    ADD_FIELD_INFO("9/2/3", "seconds",       Uint32);
    ADD_FIELD_INFO("9/2/4", "milliseconds",  Uint32);
    ADD_FIELD_INFO("10",    "ascent",        Float);
    ADD_FIELD_INFO("11",    "descent",       Float);
    ADD_FIELD_INFO("12",    "latitude",      Double);
    ADD_FIELD_INFO("13",    "longitude",     Double);
    ADD_FIELD_INFO("14",    "place",         String);
    ADD_FIELD_INFO("15",    "exercise-result",          EmbeddedMessage); /// @todo
    ADD_FIELD_INFO("16",    "exercise-counters",        EmbeddedMessage); /// @todo
    ADD_FIELD_INFO("17",    "speed-calibration-offset", Float);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parseCreateExercise(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open exercise-create file" << fileName;
        return QVariantMap();
    }
    return parseCreateExercise(file);
}

QVariantMap TrainingSession::parseCreateSession(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",      "start",              EmbeddedMessage);
    ADD_FIELD_INFO("1/1",    "date",               EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1",  "year",               Uint32);
    ADD_FIELD_INFO("1/1/2",  "month",              Uint32);
    ADD_FIELD_INFO("1/1/3",  "day",                Uint32);
    ADD_FIELD_INFO("1/2",    "time",               EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1",  "hour",               Uint32);
    ADD_FIELD_INFO("1/2/2",  "minute",             Uint32);
    ADD_FIELD_INFO("1/2/3",  "seconds",            Uint32);
    ADD_FIELD_INFO("1/2/4",  "milliseconds",       Uint32);
    ADD_FIELD_INFO("1/4",    "offset",             Int32);
    ADD_FIELD_INFO("2",      "exercise-count",     Uint32);
    ADD_FIELD_INFO("3",      "device",             String);
    ADD_FIELD_INFO("4",      "model",              String);
    ADD_FIELD_INFO("5",      "duration",           EmbeddedMessage);
    ADD_FIELD_INFO("5/1",    "hours",              Uint32);
    ADD_FIELD_INFO("5/2",    "minutes",            Uint32);
    ADD_FIELD_INFO("5/3",    "seconds",            Uint32);
    ADD_FIELD_INFO("5/4",    "milliseconds",       Uint32);
    ADD_FIELD_INFO("6",      "distance",           Float);
    ADD_FIELD_INFO("7",      "calories",           Uint32);
    ADD_FIELD_INFO("8",      "heartreat",          EmbeddedMessage);
    ADD_FIELD_INFO("8/1",    "average",            Uint32);
    ADD_FIELD_INFO("8/2",    "maximum",            Uint32);
    ADD_FIELD_INFO("9",      "heartrate-duration", EmbeddedMessage);
    ADD_FIELD_INFO("9/1",    "hours",              Uint32);
    ADD_FIELD_INFO("9/2",    "minutes",            Uint32);
    ADD_FIELD_INFO("9/3",    "seconds",            Uint32);
    ADD_FIELD_INFO("9/4",    "milliseconds",       Uint32);
    ADD_FIELD_INFO("10",     "training-load",      EmbeddedMessage);
    ADD_FIELD_INFO("10/1",   "load-value",         Uint32);
    ADD_FIELD_INFO("10/2",   "recovery-time",      EmbeddedMessage);
    ADD_FIELD_INFO("10/2/1", "hours",              Uint32);
    ADD_FIELD_INFO("10/2/2", "minutes",            Uint32);
    ADD_FIELD_INFO("10/2/3", "seconds",            Uint32);
    ADD_FIELD_INFO("10/2/4", "milliseconds",       Uint32);
    ADD_FIELD_INFO("10/3",   "carbs",              Uint32);
    ADD_FIELD_INFO("10/4",   "protein",            Uint32);
    ADD_FIELD_INFO("10/5",   "fat",                Uint32);
    ADD_FIELD_INFO("11",     "session-name",       EmbeddedMessage);
    ADD_FIELD_INFO("11/1",   "text",               String);
    ADD_FIELD_INFO("12",     "feeling",            Float);
    ADD_FIELD_INFO("13",     "note",               EmbeddedMessage);
    ADD_FIELD_INFO("13/1",   "text",               String);
    ADD_FIELD_INFO("14",     "place",              EmbeddedMessage);
    ADD_FIELD_INFO("14/1",   "text",               String);
    ADD_FIELD_INFO("15",     "latitude",           Double);
    ADD_FIELD_INFO("16",     "longitude",          Double);
    ADD_FIELD_INFO("17",     "benefit",            Enumerator);
    ADD_FIELD_INFO("18",     "sport",              EmbeddedMessage);
    ADD_FIELD_INFO("18/1",   "value",              Uint64);
    ADD_FIELD_INFO("19",     "training-target",    EmbeddedMessage);
    ADD_FIELD_INFO("19/1",   "value",              Uint64);
    ADD_FIELD_INFO("19/2",   "last-modified",      EmbeddedMessage);
    ADD_FIELD_INFO("19/2/1",   "date",             EmbeddedMessage);
    ADD_FIELD_INFO("19/2/1/1", "year",             Uint32);
    ADD_FIELD_INFO("19/2/1/2", "month",            Uint32);
    ADD_FIELD_INFO("19/2/1/3", "day",              Uint32);
    ADD_FIELD_INFO("19/2/2",   "time",             EmbeddedMessage);
    ADD_FIELD_INFO("19/2/2/1", "hour",             Uint32);
    ADD_FIELD_INFO("19/2/2/2", "minute",           Uint32);
    ADD_FIELD_INFO("19/2/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("19/2/2/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("20",     "end",                EmbeddedMessage);
    ADD_FIELD_INFO("20/1",   "date",               EmbeddedMessage);
    ADD_FIELD_INFO("20/1/1", "year",               Uint32);
    ADD_FIELD_INFO("20/1/2", "month",              Uint32);
    ADD_FIELD_INFO("20/1/3", "day",                Uint32);
    ADD_FIELD_INFO("20/2",   "time",               EmbeddedMessage);
    ADD_FIELD_INFO("20/2/1", "hour",               Uint32);
    ADD_FIELD_INFO("20/2/2", "minute",             Uint32);
    ADD_FIELD_INFO("20/2/3", "seconds",            Uint32);
    ADD_FIELD_INFO("20/2/4", "milliseconds",       Uint32);
    ADD_FIELD_INFO("20/4",   "offset",             Int32);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parseCreateSession(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open session-create file" << fileName;
        return QVariantMap();
    }
    return parseCreateSession(file);
}

QVariantMap TrainingSession::parseLaps(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",        "laps",             EmbeddedMessage);
    ADD_FIELD_INFO("1/1",      "header",           EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1",    "split-time",       EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1/1",  "hours",            Uint32);
    ADD_FIELD_INFO("1/1/1/2",  "minutes",          Uint32);
    ADD_FIELD_INFO("1/1/1/3",  "seconds",          Uint32);
    ADD_FIELD_INFO("1/1/1/4/", "milliseconds",     Uint32);
    ADD_FIELD_INFO("1/1/2",    "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("1/1/2/1",  "hours",            Uint32);
    ADD_FIELD_INFO("1/1/2/2",  "minutes",          Uint32);
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
    ADD_FIELD_INFO("2/1/2",    "minutes",          Uint32);
    ADD_FIELD_INFO("2/1/3",    "seconds",          Uint32);
    ADD_FIELD_INFO("2/1.4",    "milliseconds",     Uint32);
    ADD_FIELD_INFO("2/2",      "average-duration", EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1",    "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2",    "minutes",          Uint32);
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
        qWarning() << "failed to open laps file" << fileName;
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
        qWarning() << "failed to open route file" << fileName;
        return QVariantMap();
    }
    return parseRoute(file);
}

QVariantMap TrainingSession::parseSamples(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",     "record-interval",          EmbeddedMessage);
    ADD_FIELD_INFO("1/1",   "hours",                    Uint32);
    ADD_FIELD_INFO("1/2",   "minutes",                  Uint32);
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
        qWarning() << "failed to open samples file" << fileName;
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
    ADD_FIELD_INFO("1/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("1/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("1/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("2",     "power",            EmbeddedMessage);
    ADD_FIELD_INFO("2/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("2/1/1", "low",              Uint32);
    ADD_FIELD_INFO("2/1/2", "high",             Uint32);
    ADD_FIELD_INFO("2/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("2/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("2/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3",     "fatfit",           EmbeddedMessage);
    ADD_FIELD_INFO("3/1",   "limit",            Uint32);
    ADD_FIELD_INFO("3/2",   "fit-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("3/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/2.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3/3",   "fat-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/3/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/3/2", "minutes",          Uint32);
    ADD_FIELD_INFO("3/3/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/3.4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("4",     "speed",            EmbeddedMessage);
    ADD_FIELD_INFO("4/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("4/1/1", "low",              Float);
    ADD_FIELD_INFO("4/1/2", "high",             Float);
    ADD_FIELD_INFO("4/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("4/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("4/2/2", "minutes",          Uint32);
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
        qWarning() << "failed to open zones file" << fileName;
        return QVariantMap();
    }
    return parseZones(file);
}

/**
 * @brief Fetch the first item from a list contained within a QVariant.
 *
 * This is just a convenience function that prevents us from having to perform
 * the basic QList::isEmpty() check in many, many places.
 *
 * @param variant QVariant (probably) containing a list.
 *
 * @return The first item in the list, or an invalid variant if there is no
 *         such list, or the list is empty.
 */
QVariant first(const QVariant &variant) {
    const QVariantList list = variant.toList();
    return (list.isEmpty()) ? QVariant() : list.first();
}

QVariantMap firstMap(const QVariant &list) {
    return first(list).toMap();
}

QDateTime getDateTime(const QVariantMap &map)
{
    const QVariantMap date = firstMap(map.value(QLatin1String("date")));
    const QVariantMap time = firstMap(map.value(QLatin1String("time")));
    const QString string = QString::fromLatin1("%1-%2-%3 %4:%5:%6.%7")
        .arg(first(date.value(QLatin1String("year"))).toString())
        .arg(first(date.value(QLatin1String("month"))).toString())
        .arg(first(date.value(QLatin1String("day"))).toString())
        .arg(first(time.value(QLatin1String("hour"))).toString())
        .arg(first(time.value(QLatin1String("minute"))).toString())
        .arg(first(time.value(QLatin1String("seconds"))).toString())
        .arg(first(time.value(QLatin1String("milliseconds"))).toString());
    QDateTime dateTime = QDateTime::fromString(string, QLatin1String("yyyy-M-d H:m:s.z"));

    const QVariantMap::const_iterator offset = map.constFind(QLatin1String("offset"));
    if (offset == map.constEnd()) {
        dateTime.setTimeSpec(Qt::UTC);
    } else {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
        dateTime.setOffsetFromUtc(first(offset.value()).toInt() * 60);
        #else /// @todo Remove this when Qt 5.2+ is available on Travis CI.
        dateTime.setUtcOffset(first(offset.value()).toInt() * 60);
        #endif
    }
    return dateTime;
}

quint64 getDuration(const QVariantMap &map)
{
    const QVariantMap::const_iterator
        hours        = map.constFind(QLatin1String("hours")),
        minutes      = map.constFind(QLatin1String("minutes")),
        seconds      = map.constFind(QLatin1String("seconds")),
        milliseconds = map.constFind(QLatin1String("milliseconds"));
    return
       ((((  hours == map.constEnd()) ? 0 : first(hours.value()).toULongLong()) * 60
       + ((minutes == map.constEnd()) ? 0 : first(minutes.value()).toULongLong())) * 60
       + ((seconds == map.constEnd()) ? 0 : first(seconds.value()).toULongLong())) * 1000
       + ((milliseconds == map.constEnd()) ? 0 : first(milliseconds.value()).toULongLong());
}

QString getDurationString(const QVariantMap &map)
{
    const QVariantMap::const_iterator
        hours        = map.constFind(QLatin1String("hours")),
        minutes      = map.constFind(QLatin1String("minutes")),
        seconds      = map.constFind(QLatin1String("seconds")),
        milliseconds = map.constFind(QLatin1String("milliseconds"));
    return QString::fromLatin1("%1:%2:%3.%4")
            .arg(first(  hours.value()).toUInt(), 2, 10, QLatin1Char('0'))
            .arg(first(minutes.value()).toUInt(), 2, 10, QLatin1Char('0'))
            .arg(first(seconds.value()).toUInt(), 2, 10, QLatin1Char('0'))
            .arg(first(milliseconds.value()).toUInt(), 3, 10, QLatin1Char('0'));
}

QString getFileName(const QString &file)
{
    const QFileInfo info(file);
    return info.fileName();
}

bool sensorOffline(const QVariantList &list, const int index)
{
    foreach (const QVariant &entry, list) {
        const QVariantMap map = entry.toMap();
        const QVariant startIndex = first(map.value(QLatin1String("start-index")));
        const QVariant endIndex = first(map.value(QLatin1String("start-index")));
        if ((!startIndex.canConvert(QMetaType::Int)) ||
            (!endIndex.canConvert(QMetaType::Int))) {
            qWarning() << "ignoring invalid 'offline' entry" << entry;
            continue;
        }
        if ((startIndex.toInt() <= index) && (index <= endIndex.toInt())) {
            return true; // Sensor was offline.
        }
    }
    return false; // Sensor was not offline.
}

/// @see http://www.topografix.com/GPX/1/1/gpx.xsd
QDomDocument TrainingSession::toGPX(const QDateTime &creationTime) const
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QLatin1String("xml"),
        QLatin1String("version='1.0' encoding='utf-8'")));

    QDomElement gpx = doc.createElement(QLatin1String("gpx"));
    gpx.setAttribute(QLatin1String("version"), QLatin1String("1.1"));
    gpx.setAttribute(QLatin1String("creator"), QString::fromLatin1("%1 %2 - %3")
                     .arg(QApplication::applicationName())
                     .arg(QApplication::applicationVersion())
                     .arg(QLatin1String("https://github.com/pcolby/bipolar")));
    gpx.setAttribute(QLatin1String("xmlns"),
                     QLatin1String("http://www.topografix.com/GPX/1/1"));
    gpx.setAttribute(QLatin1String("xmlns:xsi"),
                     QLatin1String("http://www.w3.org/2001/XMLSchema-instance"));
    gpx.setAttribute(QLatin1String("xsi:schemaLocation"),
                     QLatin1String("http://www.topografix.com/GPX/1/1 "
                                   "http://www.topografix.com/GPX/1/1/gpx.xsd"));
    doc.appendChild(gpx);

    QDomElement metaData = doc.createElement(QLatin1String("metadata"));
    metaData.appendChild(doc.createElement(QLatin1String("name")))
        .appendChild(doc.createTextNode(getFileName(baseName)));
    metaData.appendChild(doc.createElement(QLatin1String("desc")))
        .appendChild(doc.createTextNode(tr("GPX encoding of %1")
                                        .arg(getFileName(baseName))));
    QDomElement link = doc.createElement(QLatin1String("link"));
    link.setAttribute(QLatin1String("href"), QLatin1String("https://github.com/pcolby/bipolar"));
    metaData.appendChild(doc.createElement(QLatin1String("author")))
        .appendChild(link).appendChild(doc.createElement(QLatin1String("text")))
            .appendChild(doc.createTextNode(QLatin1String("Bipolar")));
    metaData.appendChild(doc.createElement(QLatin1String("time")))
        .appendChild(doc.createTextNode(creationTime.toString(Qt::ISODate)));
    gpx.appendChild(metaData);

    foreach (const QVariant &exercise, parsedExercises) {
        const QVariantMap map = exercise.toMap();

        QDomElement trk = doc.createElement(QLatin1String("trk"));
        gpx.appendChild(trk);

        QStringList sources;
        foreach (const QVariant &source, map.value(QLatin1String("sources")).toList()) {
            sources << getFileName(source.toString());
        }
        trk.appendChild(doc.createElement(QLatin1String("src")))
            .appendChild(doc.createTextNode(sources.join(QLatin1Char(' '))));

        if (map.contains(ROUTE)) {
            const QVariantMap route = map.value(ROUTE).toMap();
            QDomElement trkseg = doc.createElement(QLatin1String("trkseg"));
            trk.appendChild(trkseg);

            // Get the starting time.
            const QDateTime startTime = getDateTime(firstMap(
                route.value(QLatin1String("timestamp"))));

            // Get the number of samples.
            const QVariantList altitude   = route.value(QLatin1String("altitude")).toList();
            const QVariantList duration   = route.value(QLatin1String("duration")).toList();
            const QVariantList latitude   = route.value(QLatin1String("latitude")).toList();
            const QVariantList longitude  = route.value(QLatin1String("longitude")).toList();
            const QVariantList satellites = route.value(QLatin1String("satellites")).toList();
            if ((duration.size() != altitude.size())  ||
                (duration.size() != latitude.size())  ||
                (duration.size() != longitude.size()) ||
                (duration.size() != satellites.size())) {
                qWarning() << "lists not all equal sizes:" << duration.size()
                           << altitude.size() << latitude.size()
                           << longitude.size() << satellites.size();
            }

            /// @todo Use lap data to split the <trk> into multiple <trkseg>?

            for (int index = 0; index < duration.size(); ++index) {
                QDomElement trkpt = doc.createElement(QLatin1String("trkpt"));
                trkpt.setAttribute(QLatin1String("lat"), latitude.at(index).toDouble());
                trkpt.setAttribute(QLatin1String("lon"), longitude.at(index).toDouble());
                /// @todo Use the barometric altitude instead, if present?
                trkpt.appendChild(doc.createElement(QLatin1String("ele")))
                    .appendChild(doc.createTextNode(altitude.at(index).toString()));
                trkpt.appendChild(doc.createElement(QLatin1String("time")))
                    .appendChild(doc.createTextNode(startTime.addMSecs(
                        duration.at(index).toLongLong()).toString(Qt::ISODate)));
                trkpt.appendChild(doc.createElement(QLatin1String("sat")))
                    .appendChild(doc.createTextNode(satellites.at(index).toString()));
                trkseg.appendChild(trkpt);
            }
        }
    }
    return doc;
}

/// @see http://www.polar.com/files/Polar_HRM_file%20format.pdf
QStringList TrainingSession::toHRM()
{
    QStringList hrmList;

    foreach (const QVariant &exercise, parsedExercises) {
        const QVariantMap map = exercise.toMap();
        const QVariantMap create = map.value(CREATE).toMap();
        const QVariantMap samples = map.value(SAMPLES).toMap();
        const QVariantMap zones = map.value(ZONES).toMap();

        QString hrmData;
        QTextStream stream(&hrmData);

        // [Params]
        stream <<
            "[Params]\r\n"
            "Version=107\r\n"
            "Monitor=0\r\n"
            "SMode="
                "?" // a) Speed
                "?" // b) Cadence
                "?" // c) Altitude
                "0" // d) Power (not supported by V800 yet).
                "0" // e) Power Left Right Ballance (not supported by V800 yet).
                "0" // f) Power Pedalling Index (not supported by V800 yet).
                "?" // g) HR/CC data.
                "0" // h) US / Euro unit (always metric).
                "0" // i) Air pressure (not available).
                "\r\n";

        const QDateTime startTime = getDateTime(firstMap(create.value(QLatin1String("start"))));
        stream << "Date="      << startTime.toString(QLatin1String("yyyyMMdd")) << "\r\n";
        stream << "StartTime=" << startTime.toString(QLatin1String("HH:mm:ss.zzz")) << "\r\n";
        stream << "Length="    << getDurationString(firstMap(create.value(QLatin1String("duration")))) << "\r\n";
        stream << "Interval="  << (getDuration(firstMap(samples.value(QLatin1String("record-interval"))))/1000) << "\r\n";

        QVariantList hrZones = zones.value(QLatin1String("heartrate")).toList();
        // Since HRM v1.4 only supports 3 target zones, try to reduce the list down to three by removing zones with 0 duration.
        for (int index = 0; (hrZones.length() > 3) && (index < hrZones.length()); ++index) {
            const quint64 duration = getDuration(firstMap(hrZones.at(index).toMap().value(QLatin1String("duration"))));
            if (duration == 0) {
                hrZones.removeAt(index--);
            }
        }
        for (int index = 0; (index < 3) && (index < hrZones.length()); ++index) {
            const QVariantMap limits = firstMap(hrZones.at(hrZones.length() - 3 + index).toMap().value(QLatin1String("limits")));
            stream << "Upper" << (index+1) << "=" << first(limits.value(QLatin1String("high"))).toUInt() << "\r\n";
            stream << "Lower" << (index+1) << "=" << first(limits.value(QLatin1String("low"))).toUInt() << "\r\n";
        }
        for (int index = 0; (index < 3) && (index < hrZones.length()); ++index) {
            const quint64 duration = getDuration(firstMap(hrZones.at(hrZones.length() - 3 + index).toMap().value(QLatin1String("duration"))));
            const QString hhmm = QString::fromLatin1("%1:%2")
                .arg(duration/1000/60, 2, 10, QLatin1Char('0'))
                .arg(duration/1000%60, 2, 10, QLatin1Char('0'));
            stream << "Timer" << (index+1) << "=" << hhmm << "\r\n";
        }

        /// @todo ActiveLimit

        /// @todo MaxHR - not included in training session data.
        /// @todo RestHR - not included in training session data.
        /// @todo StartDelay - RR data not yet supported (by this app).
        /// @todo VO2max - not included in training session data.
        /// @todo Weight - not included in training session data.

        // [Coach] "Coach parameters are only from Polar Coach HR monitor."

        // [Note]
        stream << "\r\n[Note]\r\n";
        if (parsedSession.contains(QLatin1String("note"))) {
            stream << first(firstMap(parsedSession.value(
                QLatin1String("note"))).value(QLatin1String("text"))).toString();
        } else if (parsedSession.contains(QLatin1String("session-name"))) {
            stream << first(firstMap(parsedSession.value(
                QLatin1String("session-name"))).value(QLatin1String("text"))).toString();
        } else {
            stream << "Exported by " << QApplication::applicationName()
                   << " " << QApplication::applicationVersion();
        }

        /// @todo [HRZones]

        /// @todo [SwapTimes]

        /// @todo [HRCCModeCh]

        /// @todo [IntTimes]

        /// @todo [IntNotes]

        /// @todo [ExtraData]
        /// @todo Add Power when supported by V800.

        /// @todo [Summary-123]

        /// @todo [Summary-TH]

        /// @todo [Trip]

        /// @todo [HRData]

        hrmList.append(hrmData);
    }
    return hrmList;
}

/**
 * @brief TrainingSession::toTCX
 *
 * @param buildTime If set, will override the internally detected build time.
 *                  Note, this is really only here to allow for deterministic
 *                  testing - not to be used by the final application.
 *
 * @return A TCX document representing the parsed Polar data.
 *
 * @see http://developer.garmin.com/schemas/tcx/v2/
 * @see http://www8.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd
 */
QDomDocument TrainingSession::toTCX(const QString &buildTime) const
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

    QDomElement activities = doc.createElement(QLatin1String("Activities"));

    QDomElement multiSportSession;
    if ((parsedExercises.size() > 1) && (!parsedSession.isEmpty())) {
        multiSportSession = doc.createElement(QLatin1String("MultiSportSession"));
        multiSportSession.appendChild(doc.createElement(QLatin1String("Id")))
            .appendChild(doc.createTextNode(getDateTime(firstMap(parsedSession
                .value(QLatin1String("start")))).toString(Qt::ISODate)));
        activities.appendChild(multiSportSession);
    }

    foreach (const QVariant &exercise, parsedExercises) {
        const QVariantMap map = exercise.toMap();
        if (!map.contains(QLatin1String("create"))) {
            qWarning() << "skipping exercise with no 'create' request data";
            continue;
        }

        QDomElement activity = doc.createElement(QLatin1String("Activity"));
        if (multiSportSession.isNull()) {
            activities.appendChild(activity);
        } else if (multiSportSession.childNodes().length() <= 1) {
            multiSportSession
                .appendChild(doc.createElement(QLatin1String("FirstSport")))
                .appendChild(activity);
        } else {
            multiSportSession
                .appendChild(doc.createElement(QLatin1String("NextSport")))
                .appendChild(activity);
        }
        Q_ASSERT(!activity.parentNode().isNull());

        const QVariantMap create = map.value(QLatin1String("create")).toMap();

        // Get the sport type.
        activity.setAttribute(QLatin1String("Sport"),
            getTcxSport(first(firstMap(create.value(QLatin1String("sport")))
                .value(QLatin1String("value"))).toULongLong()));

        // Get the starting time.
        const QDateTime startTime = getDateTime(firstMap(create.value(QLatin1String("start"))));
        activity.appendChild(doc.createElement(QLatin1String("Id")))
            .appendChild(doc.createTextNode(startTime.toString(Qt::ISODate)));

        {
            QDomElement lap = doc.createElement(QLatin1String("Lap"));
            lap.setAttribute(QLatin1String("StartTime"), startTime.toString(Qt::ISODate));
            activity.appendChild(lap);

            lap.appendChild(doc.createElement(QLatin1String("TotalTimeSeconds")))
                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                    .arg(getDuration(firstMap(create.value(QLatin1String("duration"))))/1000.0)));
            lap.appendChild(doc.createElement(QLatin1String("DistanceMeters")))
                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                    .arg(first(create.value(QLatin1String("distance"))).toDouble())));
            /// @todo [Optional] MaximumSpeed (double)
            lap.appendChild(doc.createElement(QLatin1String("Calories")))
                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                    .arg(first(create.value(QLatin1String("calories"))).toUInt())));
            /// @todo [Optional] AverageHeartRateBpm/Value (ubyte)
            /// @todo [Optional] MaximumHeartRateBpm/Value (ubyte)
            /// @todo Intensity must be one of: Active, Resting.
            lap.appendChild(doc.createElement(QLatin1String("Intensity")))
                .appendChild(doc.createTextNode(QString::fromLatin1("Active")));
            /// @todo [Optional] Cadence (ubyte, <=254).
            /// @todo TriggerMethod must be one of: Manual, Distance, Location, Time, HeartRate.
            lap.appendChild(doc.createElement(QLatin1String("TriggerMethod")))
                .appendChild(doc.createTextNode(QString::fromLatin1("Manual")));

            QDomElement track = doc.createElement(QLatin1String("Track"));
            lap.appendChild(track);

            // Get the number of "samples" samples.
            const QVariantMap samples = map.value(SAMPLES).toMap();
            const quint64 recordInterval = getDuration(
                firstMap(samples.value(QLatin1String("record-interval"))));
            const QVariantList altitude    = samples.value(QLatin1String("altitude")).toList();
            const QVariantList cadence     = samples.value(QLatin1String("cadence")).toList();
            const QVariantList distance    = samples.value(QLatin1String("distance")).toList();
            const QVariantList heartrate   = samples.value(QLatin1String("heartrate")).toList();
            const QVariantList speed       = samples.value(QLatin1String("speed")).toList();
            const QVariantList temperature = samples.value(QLatin1String("temperature")).toList();
            qDebug() << "samples sizes:"
                << altitude.size() << cadence.size() << distance.size()
                << heartrate.size() << speed.size() << temperature.size();

            // Get the number of "route" samples.
            const QVariantMap route = map.value(ROUTE).toMap();
            const QVariantList duration    = route.value(QLatin1String("duration")).toList();
            const QVariantList gpsAltitude = route.value(QLatin1String("altitude")).toList();
            const QVariantList latitude    = route.value(QLatin1String("latitude")).toList();
            const QVariantList longitude   = route.value(QLatin1String("longitude")).toList();
            const QVariantList satellites  = route.value(QLatin1String("satellites")).toList();
            qDebug() << "route sizes:" << duration.size() << gpsAltitude.size()
                     << latitude.size() << longitude.size() << satellites.size();

            for (int index = 0; index >= 0; ++index) {
                QDomElement trackPoint = doc.createElement(QLatin1String("Trackpoint"));

                if ((index < latitude.length()) && (index < longitude.length())) {
                    QDomElement position = doc.createElement(QLatin1String("Position"));
                    position.appendChild(doc.createElement(QLatin1String("LatitudeDegrees")))
                        .appendChild(doc.createTextNode(latitude.at(index).toString()));
                    position.appendChild(doc.createElement(QLatin1String("LongitudeDegrees")))
                        .appendChild(doc.createTextNode(longitude.at(index).toString()));
                    trackPoint.appendChild(position);
                }

                if ((index < altitude.length()) &&
                    (!sensorOffline(samples.value(QLatin1String("altitude-offline")).toList(), index))) {
                    trackPoint.appendChild(doc.createElement(QLatin1String("AltitudeMeters")))
                        .appendChild(doc.createTextNode(altitude.at(index).toString()));
                }
                if ((index < distance.length()) &&
                    (!sensorOffline(samples.value(QLatin1String("distance-offline")).toList(), index))) {
                    trackPoint.appendChild(doc.createElement(QLatin1String("DistanceMeters")))
                        .appendChild(doc.createTextNode(distance.at(index).toString()));
                }
                if ((index < heartrate.length()) && (heartrate.at(index).toInt() > 0) &&
                    (!sensorOffline(samples.value(QLatin1String("heartrate-offline")).toList(), index))) {
                    trackPoint.appendChild(doc.createElement(QLatin1String("HeartRateBpm")))
                        .appendChild(doc.createElement(QLatin1String("Value")))
                        .appendChild(doc.createTextNode(heartrate.at(index).toString()));
                }
                if ((index < cadence.length()) && (cadence.at(index).toInt() >= 0) &&
                    (!sensorOffline(samples.value(QLatin1String("cadence-offline")).toList(), index))) {
                    trackPoint.appendChild(doc.createElement(QLatin1String("Cadence")))
                        .appendChild(doc.createTextNode(cadence.at(index).toString()));
                }

                if (trackPoint.hasChildNodes()) {
                    #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                    QDateTime trackPointTime = startTime.addMSecs(index * recordInterval);
                    #else /// @todo Remove this hack when Qt 5.2+ is available on Travis CI.
                    QDateTime trackPointTime = startTime.toUTC()
                        .addMSecs(index * recordInterval).addSecs(startTime.utcOffset());
                    trackPointTime.setUtcOffset(startTime.utcOffset());
                    #endif
                    trackPoint.insertBefore(doc.createElement(QLatin1String("Time")), QDomNode())
                        .appendChild(doc.createTextNode(trackPointTime.toString(Qt::ISODate)));
                    track.appendChild(trackPoint);
                } else {
                    break;
                }
            }
        }
    }

    if ((multiSportSession.hasChildNodes()) ||
        (multiSportSession.isNull() && activities.hasChildNodes())) {
        tcx.appendChild(activities);
    }

    {
        QDomElement author = doc.createElement(QLatin1String("Author"));
        author.setAttribute(QLatin1String("xsi:type"), QLatin1String("Application_t"));
        author.appendChild(doc.createElement(QLatin1String("Name")))
            .appendChild(doc.createTextNode(QLatin1String("Bipolar")));
        tcx.appendChild(author);

        {
            QDomElement build = doc.createElement(QLatin1String("Build"));
            author.appendChild(build);
            QDomElement version = doc.createElement(QLatin1String("Version"));
            build.appendChild(version);
            QStringList versionParts = QApplication::applicationVersion().split(QLatin1Char('.'));
            version.appendChild(doc.createElement(QLatin1String("VersionMajor")))
                .appendChild(doc.createTextNode(versionParts.at(0)));
            version.appendChild(doc.createElement(QLatin1String("VersionMinor")))
                .appendChild(doc.createTextNode(versionParts.at(1)));
            version.appendChild(doc.createElement(QLatin1String("BuildMajor")))
                .appendChild(doc.createTextNode(versionParts.at(2)));
            version.appendChild(doc.createElement(QLatin1String("BuildMinor")))
                .appendChild(doc.createTextNode(versionParts.at(3)));
            QString buildType = QLatin1String("Release");
            #ifdef Q_OS_WIN
            {
                FileVersionInfo versionInfo;
                const QString specialBuild = versionInfo.fileInfo(QLatin1String("SpecialBuild"),
                    FileVersionInfo::US_ENGLISH, FileVersionInfo::ANSI_LATIN_1);
                if (!specialBuild.isEmpty()) {
                    buildType = specialBuild;
                }
            }
            #endif
            build.appendChild(doc.createElement(QLatin1String("Type")))
                .appendChild(doc.createTextNode(buildType));
            build.appendChild(doc.createElement(QLatin1String("Time")))
                .appendChild(doc.createTextNode(
                    buildTime.isEmpty() ? QString::fromLatin1(__DATE__" "__TIME__) : buildTime));
            /// @todo Fetch the login name at build time?
            build.appendChild(doc.createElement(QLatin1String("Builder")))
                .appendChild(doc.createTextNode(QLatin1String("Paul Colby")));
        }

        /// @todo  Make this dynamic if/when app is localized.
        author.appendChild(doc.createElement(QLatin1String("LangID")))
            .appendChild(doc.createTextNode(QLatin1String("EN")));
        author.appendChild(doc.createElement(QLatin1String("PartNumber")))
            .appendChild(doc.createTextNode(QLatin1String("434-F4C42-59")));
    }
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
        qWarning() << "zlib error" << z_result << stream.msg;
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
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qWarning() << "failed to open" << QDir::toNativeSeparators(fileName);
        return false;
    }
    return writeGPX(file);
}

bool TrainingSession::writeGPX(QIODevice &device)
{
    QDomDocument gpx = toGPX();
    if (gpx.isNull()) {
        qWarning() << "failed to conver to GPX" << baseName;
        return false;
    }
    device.write(gpx.toByteArray());
    return true;
}

bool TrainingSession::writeTCX(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qWarning() << "failed to open" << QDir::toNativeSeparators(fileName);
        return false;
    }
    return writeTCX(file);
}

bool TrainingSession::writeTCX(QIODevice &device)
{
    QDomDocument tcx = toTCX();
    if (tcx.isNull()) {
        qWarning() << "failed to conver to TCX" << baseName;
        return false;
    }
    device.write(tcx.toByteArray());
    return true;
}

}}
