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

#include "trainingsession.h"

#include "message.h"
#include "types.h"

#include "os/versioninfo.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDomElement>
#include <QFileInfo>
#include <QProcessEnvironment>

#ifdef Q_CC_MSVC
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

// Qt 5.5 increased the accuracy of QVariant::toString output for floats and
// doubles (see qtproject/qtbase@8153386), resulting in slightly different
// output, and QCOMPARE unit test failures.
// https://github.com/qt/qtbase/commit/8153386397087ce4f5c8997992edf5c1fd38b8db
//
// Qt 5.7 added QLocale::FloatingPointShortest (see qt/qtbase@726fed0), and
// updated QVariant to use that (instead of the Qt 5.5 change above) when
// converting floats and doubles to string, again resulting in slightly
// different output, and QCOMPARE unit test failures.
// https://github.com/qt/qtbase/commit/726fed0d67013cbfac7921d3d4613ca83406fb0f
//
// So, QVariant floats and doubles convert (and compare) differently between
// Qt 5.[0-4], 5.[5,6], and 5.7+.  Here we use the Qt 5.5 / 5.6 implementation
// because its at least as accurate as 5.7+, and implementing a 5.7-compatible
// fallback would be a major undertaking (needing to duplicate the third-party
// double-conversion code Qt borrows from the V8 project).
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)) && (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
    #define VARIANT_TO_STRING(v) v.toString()
#else // Fallback implementation based closely on Qt 5.5's qvariant.cpp
    #ifndef DBL_MANT_DIG
    #define DBL_MANT_DIG  53
    #endif
    #ifndef FLT_MANT_DIG
    #define FLT_MANT_DIG  24
    #endif
    #define DBL_MAX_DIGITS10 (DBL_MANT_DIG * 30103) / 100000 + 2
    #define FLT_MAX_DIGITS10 (FLT_MANT_DIG * 30103) / 100000 + 2
    #define VARIANT_TO_STRING(v) \
        (static_cast<QMetaType::Type>(v.type()) == QMetaType::Double)      \
            ? QString::number(v.toDouble(), 'g', DBL_MAX_DIGITS10)         \
            : (static_cast<QMetaType::Type>(v.type()) == QMetaType::Float) \
                ? QString::number(v.toFloat(), 'g', FLT_MAX_DIGITS10)      \
                : v.toString()
#endif

// These constants match those used by Polar's V2 API.
#define AUTOLAPS   QLatin1String("autolaps")
#define CREATE     QLatin1String("create")
#define LAPS       QLatin1String("laps")
#define ROUTE      QLatin1String("route")
#define RRSAMPLES  QLatin1String("rrsamples")
#define SAMPLES    QLatin1String("samples")
#define STATISTICS QLatin1String("statistics")
#define ZONES      QLatin1String("zones")

namespace polar {
namespace v2 {

TrainingSession::TrainingSession(const QString &baseName)
    : baseName(baseName), hrmOptions(LapNames)
{

}

int TrainingSession::exerciseCount() const
{
    return (isValid()) ? parsedExercises.count() : -1;
}

QString TrainingSession::getPolarSportName(const quint64 &polarSportValue)
{
    // Tip: sed -nEe 's/.*sports\[([0-9]+)\].*(".*").*/        {\1, QStringLiteral(\2)},/p' file |
    //          sed -Ee 's/\{([0-9],)/{  \1/' -e 's/\{([0-9]{2},)/{ \1/' */
    static QMap<quint64, QString> map{
        {  1, QStringLiteral("Running")},
        {  2, QStringLiteral("Cycling")},
        {  3, QStringLiteral("Walking")},
        {  4, QStringLiteral("Jogging")},
        {  5, QStringLiteral("Mountain biking")},
        {  6, QStringLiteral("Skiing")},
        {  7, QStringLiteral("Downhill skiing")},
        {  8, QStringLiteral("Rowing")},
        {  9, QStringLiteral("Nordic walking")},
        { 10, QStringLiteral("Skating")},
        { 11, QStringLiteral("Hiking")},
        { 12, QStringLiteral("Tennis")},
        { 13, QStringLiteral("Squash")},
        { 14, QStringLiteral("Badminton")},
        { 15, QStringLiteral("Strength training")},
        { 16, QStringLiteral("Other outdoor")},
        { 17, QStringLiteral("Treadmill running")},
        { 18, QStringLiteral("Indoor cycling")},
        { 19, QStringLiteral("Road running")},
        { 20, QStringLiteral("Circuit training")},
        { 22, QStringLiteral("Snowboarding")},
        { 23, QStringLiteral("Swimming")},
        { 24, QStringLiteral("Freestyle XC skiing")},
        { 25, QStringLiteral("Classic XC skiing")},
        { 27, QStringLiteral("Trail running")},
        { 28, QStringLiteral("Ice skating")},
        { 29, QStringLiteral("Inline skating")},
        { 30, QStringLiteral("Roller skating")},
        { 32, QStringLiteral("Group exercise")},
        { 33, QStringLiteral("Yoga")},
        { 34, QStringLiteral("Crossfit")},
        { 35, QStringLiteral("Golf")},
        { 36, QStringLiteral("Track&amp;field running")},
        { 38, QStringLiteral("Road cycling")},
        { 39, QStringLiteral("Soccer")},
        { 40, QStringLiteral("Cricket")},
        { 41, QStringLiteral("Basketball")},
        { 42, QStringLiteral("Baseball")},
        { 43, QStringLiteral("Rugby")},
        { 44, QStringLiteral("Field hockey")},
        { 45, QStringLiteral("Volleyball")},
        { 46, QStringLiteral("Ice hockey")},
        { 47, QStringLiteral("Football")},
        { 48, QStringLiteral("Handball")},
        { 49, QStringLiteral("Beach volley")},
        { 50, QStringLiteral("Futsal")},
        { 51, QStringLiteral("Floorball")},
        { 52, QStringLiteral("Dancing")},
        { 53, QStringLiteral("Trotting")},
        { 54, QStringLiteral("Riding")},
        { 55, QStringLiteral("Cross-trainer")},
        { 56, QStringLiteral("Fitness martial arts")},
        { 57, QStringLiteral("Functional training")},
        { 58, QStringLiteral("Bootcamp")},
        { 59, QStringLiteral("Freestyle roller skiing")},
        { 60, QStringLiteral("Classic roller skiing")},
        { 61, QStringLiteral("Aerobics")},
        { 62, QStringLiteral("Aqua fitness")},
        { 63, QStringLiteral("Step workout")},
        { 64, QStringLiteral("Body&amp;Mind")},
        { 65, QStringLiteral("Pilates")},
        { 66, QStringLiteral("Stretching")},
        { 67, QStringLiteral("Fitness dancing")},
        { 68, QStringLiteral("Triathlon")},
        { 69, QStringLiteral("Duathlon")},
        { 70, QStringLiteral("Off-road triathlon")},
        { 71, QStringLiteral("Off-road duathlon")},
        { 83, QStringLiteral("Other indoor")},
        { 84, QStringLiteral("Orienteering")},
        { 85, QStringLiteral("Ski orienteering")},
        { 86, QStringLiteral("Mountain bike orienteering")},
        { 87, QStringLiteral("Biathlon")},
        { 88, QStringLiteral("Sailing")},
        { 89, QStringLiteral("Wheelchair racing")},
        { 90, QStringLiteral("Disc golf")},
        { 91, QStringLiteral("Table tennis")},
        { 92, QStringLiteral("Ultra running")},
        { 94, QStringLiteral("Climbing (indoor)")},
        { 95, QStringLiteral("Kayaking")},
        { 96, QStringLiteral("Canoeing")},
        {100, QStringLiteral("Kitesurfing")},
        {101, QStringLiteral("Windsurfing")},
        {102, QStringLiteral("Surfing")},
        {103, QStringLiteral("Pool swimming")},
        {104, QStringLiteral("Finnish baseball")},
        {105, QStringLiteral("Open water swimming")},
        {107, QStringLiteral("Wakeboarding")},
        {108, QStringLiteral("Water skiing")},
        {109, QStringLiteral("Boxing")},
        {110, QStringLiteral("Kickboxing")},
        {111, QStringLiteral("Mobility (dynamic)")},
        {112, QStringLiteral("Telemark skiing")},
        {113, QStringLiteral("Backcountry skiing")},
        {114, QStringLiteral("Gymnastics")},
        {115, QStringLiteral("Judo")},
        {116, QStringLiteral("Snowshoe trekking")},
        {117, QStringLiteral("Indoor rowing")},
        {118, QStringLiteral("Spinning")},
        {119, QStringLiteral("Street")},
        {120, QStringLiteral("Latin")},
        {121, QStringLiteral("Show")},
        {122, QStringLiteral("Ballet")},
        {123, QStringLiteral("Jazz")},
        {124, QStringLiteral("Modern")},
        {125, QStringLiteral("Ballroom")},
        {126, QStringLiteral("Core")},
        {127, QStringLiteral("Mobility (static)")},
        {128, QStringLiteral("LES MILLS BODYPUMP")},
        {129, QStringLiteral("LES MILLS BODYATTACK")},
        {130, QStringLiteral("LES MILLS BODYCOMBAT")},
        {131, QStringLiteral("LES MILLS GRIT Cardio")},
        {132, QStringLiteral("LES MILLS GRIT Strength")},
        {133, QStringLiteral("LES MILLS GRIT Plyo")},
        {134, QStringLiteral("LES MILLS SH&#x27;BAM")},
        {135, QStringLiteral("LES MILLS RPM")},
        {136, QStringLiteral("LES MILLS BODYJAM")},
        {137, QStringLiteral("LES MILLS BODYSTEP")},
        {138, QStringLiteral("LES MILLS SPRINT")},
        {139, QStringLiteral("LES MILLS TONE")},
        {140, QStringLiteral("LES MILLS BODYBALANCE")},
        {141, QStringLiteral("LES MILLS THE TRIP")},
        {142, QStringLiteral("LES MILLS CXWORX")},
        {143, QStringLiteral("LES MILLS BARRE")},
    };
    QMap<quint64, QString>::ConstIterator iter = map.constFind(polarSportValue);
    if (iter == map.constEnd()) {
        qWarning() << "Unknown polar sport value" << polarSportValue;
    }
    return (iter == map.constEnd()) ? QString() : iter.value();
}

#define TCX_RUNNING QLatin1String("Running")
#define TCX_BIKING  QLatin1String("Biking")
#define TCX_OTHER   QLatin1String("Other")

QString TrainingSession::getTcxCadenceSensor(const quint64 &polarSportValue)
{
    const QString tcxSport = getTcxSport(polarSportValue);
    if (tcxSport == TCX_BIKING) {
        return QLatin1String("Bike");
    } else if (tcxSport == TCX_RUNNING) {
        return QLatin1String("Footpod");
    }
    return QString();
}

/// @see https://github.com/pcolby/bipolar/wiki/Polar-Sport-Types
QString TrainingSession::getTcxSport(const quint64 &polarSportValue)
{
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
      //map.insert(21, TCX_
        map.insert(22, TCX_OTHER);   // Snowboarding
        map.insert(23, TCX_OTHER);   // Swimming
        map.insert(24, TCX_OTHER);   // Freestyle XC skiing
        map.insert(25, TCX_OTHER);   // Classic XC skiing
      //map.insert(26, TCX_
        map.insert(27, TCX_RUNNING); // Trail running
        map.insert(28, TCX_OTHER);   // Ice skating
        map.insert(29, TCX_OTHER);   // Inline skating
        map.insert(30, TCX_OTHER);   // Roller skating
      //map.insert(31, TCX_
        map.insert(32, TCX_OTHER);   // Group exercise
        map.insert(33, TCX_OTHER);   // Yoga
        map.insert(34, TCX_OTHER);   // Crossfit
        map.insert(35, TCX_OTHER);   // Golf
        map.insert(36, TCX_RUNNING); // Track&field running
      //map.insert(37, TCX_
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
      //map.insert(72, TCX_
      //map.insert(73, TCX_
      //map.insert(74, TCX_
      //map.insert(75, TCX_
      //map.insert(76, TCX_
      //map.insert(77, TCX_
      //map.insert(78, TCX_
      //map.insert(79, TCX_
      //map.insert(80, TCX_
      //map.insert(81, TCX_
        map.insert(82, TCX_OTHER);   // Multisport
        map.insert(83, TCX_OTHER);   // Other indoor
        map.insert(84, TCX_OTHER);   // Orienteering
        map.insert(85, TCX_OTHER);   // Ski orienteering
        map.insert(86, TCX_BIKING);  // Mountain bike orienteering
        map.insert(87, TCX_OTHER);   // Biathlon
        map.insert(88, TCX_OTHER);   // Sailing
        map.insert(89, TCX_OTHER);   // Wheelchair racing
        map.insert(90, TCX_OTHER);   // Disc golf
        map.insert(91, TCX_OTHER);   // Table tennis
        map.insert(92, TCX_RUNNING); // Ultra running
        map.insert(94, TCX_OTHER);   // Climbing (indoor)
      //map.insert(93, TCX_
        map.insert(95, TCX_OTHER);   // Kayaking
        map.insert(96, TCX_OTHER);   // Canoeing
      //map.insert(97, TCX_
      //map.insert(98, TCX_
      //map.insert(99, TCX_
        map.insert(100, TCX_OTHER);  // Kitesurfing
        map.insert(101, TCX_OTHER);  // Windsurfing
        map.insert(102, TCX_OTHER);  // Surfing
        map.insert(103, TCX_OTHER);  // Pool swimming
        map.insert(104, TCX_OTHER);  // Finnish baseball
        map.insert(105, TCX_OTHER);  // Open water swimming
      //map.insert(106, TCX
        map.insert(107, TCX_OTHER);  // Wakeboarding
        map.insert(108, TCX_OTHER);  // Water skiing
        map.insert(109, TCX_OTHER);  // Boxing
        map.insert(110, TCX_OTHER);  // Kickboxing
        map.insert(111, TCX_OTHER);  // Mobility (dynamic)
        map.insert(112, TCX_OTHER);  // Telemark skiing
        map.insert(113, TCX_OTHER);  // Backcountry skiing
        map.insert(114, TCX_OTHER);  // Gymnastics
        map.insert(115, TCX_OTHER);  // Judo
        map.insert(116, TCX_OTHER);  // Snowshoe trekking
        map.insert(117, TCX_OTHER);  // Indoor rowing
        map.insert(118, TCX_BIKING); // Spinning
        map.insert(119, TCX_OTHER);  // Street
        map.insert(120, TCX_OTHER);  // Latin
        map.insert(121, TCX_OTHER);  // Show
        map.insert(122, TCX_OTHER);  // Ballet
        map.insert(123, TCX_OTHER);  // Jazz
        map.insert(124, TCX_OTHER);  // Modern
        map.insert(125, TCX_OTHER);  // Ballroom
        map.insert(126, TCX_OTHER);  // Core
        map.insert(127, TCX_OTHER);  // Mobility (static)
        map.insert(128, TCX_OTHER);  // LES MILLS BODYPUMP
        map.insert(129, TCX_OTHER);  // LES MILLS BODYATTACK
        map.insert(130, TCX_OTHER);  // LES MILLS BODYCOMBAT
        map.insert(131, TCX_OTHER);  // LES MILLS GRIT Cardio
        map.insert(132, TCX_OTHER);  // LES MILLS GRIT Strength
        map.insert(133, TCX_OTHER);  // LES MILLS GRIT Plyo
        map.insert(134, TCX_OTHER);  // LES MILLS SH'BAM
        map.insert(135, TCX_BIKING); // LES MILLS RPM
        map.insert(136, TCX_OTHER);  // LES MILLS BODYJAM
        map.insert(137, TCX_OTHER);  // LES MILLS BODYSTEP
        map.insert(138, TCX_BIKING); // LES MILLS SPRINT
        map.insert(139, TCX_OTHER);  // LES MILLS BODYVIVE
        map.insert(140, TCX_OTHER);  // LES MILLS BODYBALANCE
        map.insert(141, TCX_BIKING); // LES MILLS THE TRIP
        map.insert(142, TCX_OTHER);  // LES MILLS CXWORX
        map.insert(143, TCX_OTHER);  // LES MILLS BARRE
    }
    QMap<quint64, QString>::ConstIterator iter = map.constFind(polarSportValue);
    if (iter == map.constEnd()) {
        qWarning() << "Unknown polar sport value" << polarSportValue;
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

bool TrainingSession::parse()
{
    parsedExercises.clear();

    parsedPhysicalInformation = parsePhysicalInformation(
        baseName + QLatin1String("-physical-information"));

    parsedSession = parseCreateSession(baseName + QLatin1String("-create"));

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
    PARSE_IF_CONTAINS(AUTOLAPS,   Laps);
    PARSE_IF_CONTAINS(CREATE,     CreateExercise);
    PARSE_IF_CONTAINS(LAPS,       Laps);
  //PARSE_IF_CONTAINS(PHASES,     Phases);
    PARSE_IF_CONTAINS(ROUTE,      Route);
    PARSE_IF_CONTAINS(RRSAMPLES,  RRSamples);
    PARSE_IF_CONTAINS(SAMPLES,    Samples);
  //PARSE_IF_CONTAINS(SENSORS,    Sensors);
    PARSE_IF_CONTAINS(STATISTICS, Statistics);
    PARSE_IF_CONTAINS(ZONES,      Zones);
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
    ADD_FIELD_INFO("1/3",   "trusted",       Bool);
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
    ADD_FIELD_INFO("15",       "target-result",     EmbeddedMessage);
    ADD_FIELD_INFO("15/1",     "index",             Uint32);
    ADD_FIELD_INFO("15/2",     "reached",           Bool);
    ADD_FIELD_INFO("15/3",     "end-time",          EmbeddedMessage);
    ADD_FIELD_INFO("15/3/1",   "hours",             Uint32);
    ADD_FIELD_INFO("15/3/2",   "minutes",           Uint32);
    ADD_FIELD_INFO("15/3/3",   "seconds",           Uint32);
    ADD_FIELD_INFO("15/3/4",   "milliseconds",      Uint32);
    ADD_FIELD_INFO("15/4",     "race-pace-result",  EmbeddedMessage);
    ADD_FIELD_INFO("15/4/1",   "completed",         EmbeddedMessage);
    ADD_FIELD_INFO("15/4/1/1", "hours",             Uint32);
    ADD_FIELD_INFO("15/4/1/2", "minutes",           Uint32);
    ADD_FIELD_INFO("15/4/1/3", "seconds",           Uint32);
    ADD_FIELD_INFO("15/4/1/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("15/4/2",   "heartrate",         Uint32);
    ADD_FIELD_INFO("15/4/3",   "speed",             Float);
    ADD_FIELD_INFO("15/5",     "volume-target",     EmbeddedMessage);
    ADD_FIELD_INFO("15/5/1",   "target-type",       Enumerator);
    ADD_FIELD_INFO("15/5/2",   "duration",          EmbeddedMessage);
    ADD_FIELD_INFO("15/5/2/1", "hours",             Uint32);
    ADD_FIELD_INFO("15/5/2/2", "minutes",           Uint32);
    ADD_FIELD_INFO("15/5/2/3", "seconds",           Uint32);
    ADD_FIELD_INFO("15/5/2/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("15/5/3",   "distance",          Float);
    ADD_FIELD_INFO("15/5/4",   "calores",           Uint32);
    ADD_FIELD_INFO("16",       "exercise-counters", EmbeddedMessage);
    ADD_FIELD_INFO("16/1",     "sprint-count",      Uint32);
    ADD_FIELD_INFO("17", "speed-calibration-offset", Float);
    ADD_FIELD_INFO("18",       "walking-distance",  Float);
    ADD_FIELD_INFO("19",       "walking-duration",  EmbeddedMessage);
    ADD_FIELD_INFO("19/1",     "hours",             Uint32);
    ADD_FIELD_INFO("19/2",     "minutes",           Uint32);
    ADD_FIELD_INFO("19/3",     "seconds",           Uint32);
    ADD_FIELD_INFO("19/4",     "milliseconds",      Uint32);
    ADD_FIELD_INFO("20",       "accumulated-torque",            Uint32);
    ADD_FIELD_INFO("21",       "cycling-power-energy",          Uint32);
    ADD_FIELD_INFO("22",       "sensor-calibration-offset",     EmbeddedMessage);
    ADD_FIELD_INFO("22/1",     "sample-source-type",            Enumerator);
    ADD_FIELD_INFO("22/2",     "speed-cal-offset",              Float);
    ADD_FIELD_INFO("23",       "device_location",               Enumerator);
    ADD_FIELD_INFO("24",       "power_sample_source_device",    EmbeddedMessage);
    ADD_FIELD_INFO("24/1",     "start-index",                   Uint32);
    ADD_FIELD_INFO("24/2",     "source-device",                 EmbeddedMessage);
    ADD_FIELD_INFO("24/2/1",   "name",                          String);
    ADD_FIELD_INFO("24/2/2",   "manufacturer",                  String);
    ADD_FIELD_INFO("24/2/3",   "model",                         String);
    ADD_FIELD_INFO("24/2/4",   "hardware-code",                 String);
    ADD_FIELD_INFO("24/2/5",   "platform-version",              EmbeddedMessage);
    ADD_FIELD_INFO("24/2/5/1", "major",                         Uint32);
    ADD_FIELD_INFO("24/2/5/2", "minor",                         Uint32);
    ADD_FIELD_INFO("24/2/5/3", "patch",                         Uint32);
    ADD_FIELD_INFO("24/2/5/4", "specifier",                     String);
    ADD_FIELD_INFO("24/2/6",   "software-version",              EmbeddedMessage);
    ADD_FIELD_INFO("24/2/6/1", "major",                         Uint32);
    ADD_FIELD_INFO("24/2/6/2", "minor",                         Uint32);
    ADD_FIELD_INFO("24/2/6/3", "patch",                         Uint32);
    ADD_FIELD_INFO("24/2/6/4", "specifier",                     String);
    ADD_FIELD_INFO("24/2/7",   "polarmathsmart-version",        EmbeddedMessage);
    ADD_FIELD_INFO("24/2/7/1", "major",                         Uint32);
    ADD_FIELD_INFO("24/2/7/2", "minor",                         Uint32);
    ADD_FIELD_INFO("24/2/7/3", "patch",                         Uint32);
    ADD_FIELD_INFO("24/2/7/4", "specifier",                     String);
    ADD_FIELD_INFO("24/2/8",   "collector",                     EmbeddedMessage); // This one is recursive!
    ADD_FIELD_INFO("25",       "cardio-load",                   EmbeddedMessage);
    ADD_FIELD_INFO("25/1",     "activity-load",                 Float);
    ADD_FIELD_INFO("25/2",     "exercise-load",                 Float);
    ADD_FIELD_INFO("26",       "cardio-load-interpretation",    Uint32);
    ADD_FIELD_INFO("27",       "perceived-load",                EmbeddedMessage);
    ADD_FIELD_INFO("27/1",     "session-rpe",                   Enumerator);
    ADD_FIELD_INFO("27/2",     "duration",                      Uint32);
    ADD_FIELD_INFO("28",       "perceived-load-interpretation", Uint32);
    ADD_FIELD_INFO("29",       "musle-load",                    Float);
    ADD_FIELD_INFO("30",       "muscle-load-interpretation",    Uint32);
    ADD_FIELD_INFO("100",      "modified",                      EmbeddedMessage);
    ADD_FIELD_INFO("100/1",    "date",                          EmbeddedMessage);
    ADD_FIELD_INFO("100/1/1",  "year",                          Uint32);
    ADD_FIELD_INFO("100/1/2",  "month",                         Uint32);
    ADD_FIELD_INFO("100/1/3",  "day",                           Uint32);
    ADD_FIELD_INFO("100/2",    "time",                          EmbeddedMessage);
    ADD_FIELD_INFO("100/2/1",  "hour",                          Uint32);
    ADD_FIELD_INFO("100/2/2",  "minute",                        Uint32);
    ADD_FIELD_INFO("100/2/3",  "seconds",                       Uint32);
    ADD_FIELD_INFO("100/2/4",  "milliseconds",                  Uint32);
    ADD_FIELD_INFO("100/3",    "trusted",                       Bool);

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
        qWarning() << "Failed to open exercise-create file" << fileName;
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
    ADD_FIELD_INFO("1/3",    "trusted",            Bool);
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
    ADD_FIELD_INFO("8",      "heartrate",          EmbeddedMessage);
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
    ADD_FIELD_INFO("19/2/3",   "trusted",          Bool);
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
    ADD_FIELD_INFO("20/3",   "trusted",            Bool);
    ADD_FIELD_INFO("20/4",   "offset",             Int32);
    ADD_FIELD_INFO("21",     "favorite-id",        EmbeddedMessage);
    ADD_FIELD_INFO("21/1",   "value",              Uint64);
    ADD_FIELD_INFO("21/2",   "last-modified",      EmbeddedMessage);
    ADD_FIELD_INFO("21/2/1",   "date",             EmbeddedMessage);
    ADD_FIELD_INFO("21/2/1/1", "year",             Uint32);
    ADD_FIELD_INFO("21/2/1/2", "month",            Uint32);
    ADD_FIELD_INFO("21/2/1/3", "day",              Uint32);
    ADD_FIELD_INFO("22",       "application-id",                EmbeddedMessage);
    ADD_FIELD_INFO("22/1",     "value",                         Uint64);
    ADD_FIELD_INFO("23",       "cardio-load",                   EmbeddedMessage);
    ADD_FIELD_INFO("23/1",     "activity-load",                 Float);
    ADD_FIELD_INFO("23/2",     "exercise-load",                 Float);
    ADD_FIELD_INFO("24",       "cardio-load-interpretation",    Uint32);
    ADD_FIELD_INFO("25",       "perceived-load",                EmbeddedMessage);
    ADD_FIELD_INFO("25/1",     "session-rpe",                   Enumerator);
    ADD_FIELD_INFO("25/2",     "duration",                      Uint32);
    ADD_FIELD_INFO("26",       "perceived-load-interpretation", Uint32);
    ADD_FIELD_INFO("27",       "musle-load",                    Float);
    ADD_FIELD_INFO("28",       "muscle-load-interpretation",    Uint32);

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
        qWarning() << "Failed to open session-create file" << fileName;
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
    ADD_FIELD_INFO("1/1/1/4",  "milliseconds",     Uint32);
    ADD_FIELD_INFO("1/1/2",    "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("1/1/2/1",  "hours",            Uint32);
    ADD_FIELD_INFO("1/1/2/2",  "minutes",          Uint32);
    ADD_FIELD_INFO("1/1/2/3",  "seconds",          Uint32);
    ADD_FIELD_INFO("1/1/2/4",  "milliseconds",     Uint32);
    ADD_FIELD_INFO("1/1/3",    "distance",         Float);
    ADD_FIELD_INFO("1/1/4",    "ascent",           Float);
    ADD_FIELD_INFO("1/1/5",    "descent",          Float);
    ADD_FIELD_INFO("1/1/6",    "lap-type",         Enumerator);
    ADD_FIELD_INFO("1/2",      "stats",            EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1",    "heartrate",        EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1/1",  "average",          Uint32);
    ADD_FIELD_INFO("1/2/1/2",  "maximum",          Uint32);
    ADD_FIELD_INFO("1/2/1/3",  "minimum",          Uint32);
    ADD_FIELD_INFO("1/2/2",    "speed",            EmbeddedMessage);
    ADD_FIELD_INFO("1/2/2/1",  "average",          Float);
    ADD_FIELD_INFO("1/2/2/2",  "maximum",          Float);
    ADD_FIELD_INFO("1/2/3",    "cadence",          EmbeddedMessage);
    ADD_FIELD_INFO("1/2/3/1",  "average",          Uint32);
    ADD_FIELD_INFO("1/2/3/2",  "maximum",          Uint32);
    ADD_FIELD_INFO("1/2/4",    "power",            EmbeddedMessage);
    ADD_FIELD_INFO("1/2/4/1",  "average",          Uint32);
    ADD_FIELD_INFO("1/2/4/2",  "maximum",          Uint32);
    ADD_FIELD_INFO("1/2/5",    "pedaling",         EmbeddedMessage);
    ADD_FIELD_INFO("1/2/5/1",  "average",          Uint32);
    ADD_FIELD_INFO("1/2/6",    "incline",          EmbeddedMessage);
    ADD_FIELD_INFO("1/2/6/1",  "average",          Float);
    ADD_FIELD_INFO("1/2/7",    "stride",           EmbeddedMessage);
    ADD_FIELD_INFO("1/2/7/1",  "average",          Uint32);
    ADD_FIELD_INFO("1/2/8",    "swimming",         EmbeddedMessage);
    ADD_FIELD_INFO("1/2/8/1",  "strokes",          Uint32);
    ADD_FIELD_INFO("1/2/8/2",  "pool-count",       Uint32);
    ADD_FIELD_INFO("1/2/8/3",  "average-duration", Float);
    ADD_FIELD_INFO("1/2/9",    "left-right-balance", EmbeddedMessage);
    ADD_FIELD_INFO("1/2/9/1",  "average",          Float);
    ADD_FIELD_INFO("2",        "summary",          EmbeddedMessage);
    ADD_FIELD_INFO("2/1",      "best-duration",    EmbeddedMessage);
    ADD_FIELD_INFO("2/1/1",    "hours",            Uint32);
    ADD_FIELD_INFO("2/1/2",    "minutes",          Uint32);
    ADD_FIELD_INFO("2/1/3",    "seconds",          Uint32);
    ADD_FIELD_INFO("2/1/4",    "milliseconds",     Uint32);
    ADD_FIELD_INFO("2/2",      "average-duration", EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1",    "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2",    "minutes",          Uint32);
    ADD_FIELD_INFO("2/2/3",    "seconds",          Uint32);
    ADD_FIELD_INFO("2/2/4",    "milliseconds",     Uint32);
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
        qWarning() << "Failed to open laps file" << fileName;
        return QVariantMap();
    }
    return parseLaps(file);
}

QVariantMap TrainingSession::parsePhysicalInformation(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",        "birthday",            EmbeddedMessage);
    ADD_FIELD_INFO("1/1",      "value",               EmbeddedMessage);
    ADD_FIELD_INFO("1/1/1",    "year",                Uint32);
    ADD_FIELD_INFO("1/1/2",    "month",               Uint32);
    ADD_FIELD_INFO("1/1/3",    "day",                 Uint32);
    ADD_FIELD_INFO("1/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("1/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("1/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("1/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("1/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("1/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("1/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("1/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("1/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("1/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("2",        "gender",              EmbeddedMessage);
    ADD_FIELD_INFO("2/1",      "value",               Enumerator);
    ADD_FIELD_INFO("2/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("2/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("2/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("2/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("2/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("2/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("2/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("2/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("2/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("3",        "weight",              EmbeddedMessage);
    ADD_FIELD_INFO("3/1",      "value",               Float);
    ADD_FIELD_INFO("3/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("3/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("3/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("3/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("3/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("3/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("3/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("3/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("3/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("3/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("3/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("3/3",      "source",              Enumerator); // 0=default, 2=user, 3=measured.
    ADD_FIELD_INFO("4",        "height",              EmbeddedMessage);
    ADD_FIELD_INFO("4/1",      "value",               Float);
    ADD_FIELD_INFO("4/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("4/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("4/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("4/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("4/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("4/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("4/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("4/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("4/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("4/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("4/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("5",        "maximum-heartrate",   EmbeddedMessage);
    ADD_FIELD_INFO("5/1",      "value",               Uint32);
    ADD_FIELD_INFO("5/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("5/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("5/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("5/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("5/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("5/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("5/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("5/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("5/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("5/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("5/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("5/3",      "source",              Enumerator);
    ADD_FIELD_INFO("6",        "resting-heartrate",   EmbeddedMessage);
    ADD_FIELD_INFO("6/1",      "value",               Uint32);
    ADD_FIELD_INFO("6/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("6/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("6/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("6/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("6/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("6/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("6/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("6/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("6/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("6/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("6/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("6/3",      "source",              Enumerator);
    ADD_FIELD_INFO("8",        "aerobic-threshold",   EmbeddedMessage);
    ADD_FIELD_INFO("8/1",      "value",               Uint32);
    ADD_FIELD_INFO("8/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("8/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("8/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("8/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("8/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("8/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("8/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("8/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("8/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("8/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("8/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("8/3",      "source",              Enumerator);
    ADD_FIELD_INFO("9",        "anaerobic-threshold", EmbeddedMessage);
    ADD_FIELD_INFO("9/1",      "value",               Uint32);
    ADD_FIELD_INFO("9/2",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("9/2/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("9/2/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("9/2/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("9/2/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("9/2/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("9/2/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("9/2/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("9/2/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("9/2/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("9/2/3",    "trusted",             Bool);
    ADD_FIELD_INFO("9/3",      "source",              Enumerator);
    ADD_FIELD_INFO("10",       "vo2max",              EmbeddedMessage);
    ADD_FIELD_INFO("10/1",     "value",               Uint32);
    ADD_FIELD_INFO("10/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("10/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("10/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("10/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("10/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("10/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("10/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("10/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("10/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("10/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("10/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("10/3",     "source",              Enumerator);
    ADD_FIELD_INFO("11",       "training-background", EmbeddedMessage);
    ADD_FIELD_INFO("11/1",     "value",               Enumerator);
    ADD_FIELD_INFO("11/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("11/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("11/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("11/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("11/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("11/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("11/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("11/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("11/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("11/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("11/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("12",       "typical-day",         EmbeddedMessage);
    ADD_FIELD_INFO("12/1",     "value",               Enumerator);
    ADD_FIELD_INFO("12/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("12/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("12/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("12/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("12/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("12/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("12/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("12/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("12/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("12/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("12/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("13",       "weekly-recovery",     EmbeddedMessage);
    ADD_FIELD_INFO("13/1",     "value",               Float);
    ADD_FIELD_INFO("13/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("13/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("13/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("13/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("13/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("13/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("13/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("13/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("13/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("13/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("13/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("14",       "speed-calibration-offset", EmbeddedMessage);
    ADD_FIELD_INFO("14/1",     "value",               Float);
    ADD_FIELD_INFO("14/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("14/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("14/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("14/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("14/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("14/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("14/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("14/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("14/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("14/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("14/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("15",       "functional-threshold-power", EmbeddedMessage);
    ADD_FIELD_INFO("15/1",     "value",               Uint32);
    ADD_FIELD_INFO("15/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("15/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("15/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("15/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("15/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("15/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("15/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("15/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("15/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("15/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("15/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("15/3",     "source",              Enumerator);
    ADD_FIELD_INFO("16",       "sensor-calibration-offset", EmbeddedMessage);
    ADD_FIELD_INFO("16/1",     "sample-source-type",  Enumerator);
    ADD_FIELD_INFO("16/2",     "speed-cal-offset",    Float);
    ADD_FIELD_INFO("17",       "sleep-goal",          EmbeddedMessage);
    ADD_FIELD_INFO("17/1",     "sleep-goal-minutes",  Uint32);
    ADD_FIELD_INFO("17/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("17/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("17/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("17/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("17/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("17/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("17/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("17/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("17/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("17/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("17/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("18",       "running-maximum-aerobic-power", EmbeddedMessage);
    ADD_FIELD_INFO("18/1",     "value",               Uint32);
    ADD_FIELD_INFO("18/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("18/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("18/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("18/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("18/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("18/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("18/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("18/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("18/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("18/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("18/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("18/3",     "setting-source",      Enumerator);
    ADD_FIELD_INFO("19",       "running-maximum-aerobic-speed", EmbeddedMessage);
    ADD_FIELD_INFO("19/1",     "value",               Float);
    ADD_FIELD_INFO("19/2",     "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("19/2/1",   "date",                EmbeddedMessage);
    ADD_FIELD_INFO("19/2/1/1", "year",                Uint32);
    ADD_FIELD_INFO("19/2/1/2", "month",               Uint32);
    ADD_FIELD_INFO("19/2/1/3", "day",                 Uint32);
    ADD_FIELD_INFO("19/2/2",   "time",                EmbeddedMessage);
    ADD_FIELD_INFO("19/2/2/1", "hour",                Uint32);
    ADD_FIELD_INFO("19/2/2/2", "minute",              Uint32);
    ADD_FIELD_INFO("19/2/2/3", "seconds",             Uint32);
    ADD_FIELD_INFO("19/2/2/4", "milliseconds",        Uint32);
    ADD_FIELD_INFO("19/2/3",   "trusted",             Bool);
    ADD_FIELD_INFO("19/3",     "setting-source",      Enumerator);
    ADD_FIELD_INFO("100",      "modified",            EmbeddedMessage);
    ADD_FIELD_INFO("100/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("100/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("100/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("100/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("100/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("100/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("100/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("100/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("100/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("100/3",    "trusted",             Bool);
    ADD_FIELD_INFO("101",      "snapshot-start-time", EmbeddedMessage);
    ADD_FIELD_INFO("101/1",    "date",                EmbeddedMessage);
    ADD_FIELD_INFO("101/1/1",  "year",                Uint32);
    ADD_FIELD_INFO("101/1/2",  "month",               Uint32);
    ADD_FIELD_INFO("101/1/3",  "day",                 Uint32);
    ADD_FIELD_INFO("101/2",    "time",                EmbeddedMessage);
    ADD_FIELD_INFO("101/2/1",  "hour",                Uint32);
    ADD_FIELD_INFO("101/2/2",  "minute",              Uint32);
    ADD_FIELD_INFO("101/2/3",  "seconds",             Uint32);
    ADD_FIELD_INFO("101/2/4",  "milliseconds",        Uint32);
    ADD_FIELD_INFO("101/3",    "trusted",             Bool);
    ADD_FIELD_INFO("101/4",    "offset",              Int32);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parsePhysicalInformation(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open physical information file" << fileName;
        return QVariantMap();
    }
    return parsePhysicalInformation(file);
}

QVariantMap TrainingSession::parseRoute(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",     "duration",     Uint32);
    ADD_FIELD_INFO("2",     "latitude",     Double);
    ADD_FIELD_INFO("3",     "longitude",    Double);
    ADD_FIELD_INFO("4",     "altitude",     Sint32);
    ADD_FIELD_INFO("5",     "satellites",   Uint32);
    ADD_FIELD_INFO("6",     "fix",          Bool);            // Obsolete?
    ADD_FIELD_INFO("7",     "gps-offline",  EmbeddedMessage); // Obsolete?
    ADD_FIELD_INFO("7/1",   "start-index",  Uint32);
    ADD_FIELD_INFO("7/2",   "stop-index",   Uint32);
    ADD_FIELD_INFO("8",     "gps-time",     EmbeddedMessage); // Obsolete?
    ADD_FIELD_INFO("8/1",   "date",         EmbeddedMessage);
    ADD_FIELD_INFO("8/1/1", "year",         Uint32);
    ADD_FIELD_INFO("8/1/2", "month",        Uint32);
    ADD_FIELD_INFO("8/1/3", "day",          Uint32);
    ADD_FIELD_INFO("8/2",   "time",         EmbeddedMessage);
    ADD_FIELD_INFO("8/2/1", "hour",         Uint32);
    ADD_FIELD_INFO("8/2/2", "minute",       Uint32);
    ADD_FIELD_INFO("8/2/3", "seconds",      Uint32);
    ADD_FIELD_INFO("8/2/4", "milliseconds", Uint32);
    ADD_FIELD_INFO("8/3",   "trusted",      Bool);
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
    ADD_FIELD_INFO("9/3",   "trusted",      Bool);
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
        qWarning() << "Failed to open route file" << fileName;
        return QVariantMap();
    }
    return parseRoute(file);
}

QVariantMap TrainingSession::parseRRSamples(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1", "value", Uint32);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parseRRSamples(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open rrsamples file" << fileName;
        return QVariantMap();
    }
    return parseRRSamples(file);
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
    ADD_FIELD_INFO("21",    "moving-type-offline",      EmbeddedMessage);
    ADD_FIELD_INFO("21/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("21/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("22",    "left-pedal-power",         EmbeddedMessage);
    ADD_FIELD_INFO("22/1",  "current-power",            Int32);
    ADD_FIELD_INFO("22/2",  "cumulative-revolutions",   Uint32);
    ADD_FIELD_INFO("22/3",  "cumulative-timestamp",     Uint32);
    ADD_FIELD_INFO("22/4",  "min-force",                Sint32);
    ADD_FIELD_INFO("22/5",  "max-force",                Uint32);
    ADD_FIELD_INFO("22/6",  "min-force-angle",          Uint32);
    ADD_FIELD_INFO("22/7",  "max-force-angle",          Uint32);
    ADD_FIELD_INFO("22/8",  "bottom-dead-spot",         Uint32);
    ADD_FIELD_INFO("22/9",  "top-dead-spot",            Uint32);
    ADD_FIELD_INFO("23",    "left-pedal-power-offline", EmbeddedMessage);
    ADD_FIELD_INFO("23/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("23/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("24",    "right-pedal-power",        EmbeddedMessage);
    ADD_FIELD_INFO("24/1",  "current-power",            Int32);
    ADD_FIELD_INFO("24/2",  "cumulative-revolutions",   Uint32);
    ADD_FIELD_INFO("24/3",  "cumulative-timestamp",     Uint32);
    ADD_FIELD_INFO("24/4",  "min-force",                Sint32);
    ADD_FIELD_INFO("24/5",  "max-force",                Uint32);
    ADD_FIELD_INFO("24/6",  "min-force-angle",          Uint32);
    ADD_FIELD_INFO("24/7",  "max-force-angle",          Uint32);
    ADD_FIELD_INFO("24/8",  "bottom-dead-spot",         Uint32);
    ADD_FIELD_INFO("24/9",  "top-dead-spot",            Uint32);
    ADD_FIELD_INFO("25",    "right-pedal-power-offline",EmbeddedMessage);
    ADD_FIELD_INFO("25/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("25/2",  "stop-index",               Uint32);
    ADD_FIELD_INFO("26",    "left-power-calibration",   EmbeddedMessage);
    ADD_FIELD_INFO("26/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("26/2",  "value",                    Float);
    ADD_FIELD_INFO("26/3",  "operation",                Enumerator);
    ADD_FIELD_INFO("26/4",  "cause",                    Enumerator);
    ADD_FIELD_INFO("27",    "right-power-calibration",  EmbeddedMessage);
    ADD_FIELD_INFO("27/1",  "start-index",              Uint32);
    ADD_FIELD_INFO("27/2",  "value",                    Float);
    ADD_FIELD_INFO("27/3",  "operation",                Enumerator);
    ADD_FIELD_INFO("27/4",  "cause",                    Enumerator);
    ADD_FIELD_INFO("28"  ,    "heartrate-variability",  EmbeddedMessage);
    ADD_FIELD_INFO("28/1",    "intervals",              Uint32);
    ADD_FIELD_INFO("28/2",    "offline",                EmbeddedMessage);
    ADD_FIELD_INFO("28/2/1",  "starttime",              EmbeddedMessage);
    ADD_FIELD_INFO("28/2/1/1","hours",                  Uint32);
    ADD_FIELD_INFO("28/2/1/2","minutes",                Uint32);
    ADD_FIELD_INFO("28/2/1/3","seconds",                Uint32);
    ADD_FIELD_INFO("28/2/1/4","milliseconds",           Uint32);
    ADD_FIELD_INFO("28/2/2"  ,"duration",               EmbeddedMessage);
    ADD_FIELD_INFO("28/2/2/1","hours",                  Uint32);
    ADD_FIELD_INFO("28/2/2/2","minutes",                Uint32);
    ADD_FIELD_INFO("28/2/2/3","seconds",                Uint32);
    ADD_FIELD_INFO("28/2/2/4","milliseconds",           Uint32);
    ADD_FIELD_INFO("29",         "intervalled-samples",        EmbeddedMessage);
    ADD_FIELD_INFO("29/1",       "sample-type",                Enumerator);
    ADD_FIELD_INFO("29/2",       "rec-interval-ms",            Uint32);
    ADD_FIELD_INFO("29/3",       "sample-source",              EmbeddedMessage);
    ADD_FIELD_INFO("29/3/1",     "sample-source-type",         Enumerator);
    ADD_FIELD_INFO("29/3/2",     "start-index",                Uint32);
    ADD_FIELD_INFO("29/3/3",     "stop-index",                 Uint32);
    ADD_FIELD_INFO("29/4",       "hr-samples",                 Uint32);
    ADD_FIELD_INFO("29/5",       "cadence-samples",            Uint32);
    ADD_FIELD_INFO("29/6",       "speed-samples",              Float);
    ADD_FIELD_INFO("29/7",       "distance-samples",           Float);
    ADD_FIELD_INFO("29/8",       "fwd-acceleration",           Float);
    ADD_FIELD_INFO("29/9",       "moving-type-samples",        Enumerator);
    ADD_FIELD_INFO("29/10",      "altitude-samples",           Float);
    ADD_FIELD_INFO("29/11",      "altitude-calibration",       EmbeddedMessage);
    ADD_FIELD_INFO("29/11/1",    "start-index",                Uint32);
    ADD_FIELD_INFO("29/11/2",    "value",                      Float);
    ADD_FIELD_INFO("29/11/3",    "operation",                  Enumerator); // 1=Multiply, 2=Sum
    ADD_FIELD_INFO("29/11/4",    "cause",                      Enumerator); // 0=Walk, 1=Run, 2=Stand
    ADD_FIELD_INFO("29/12",      "temperature-samples",        Float);
    ADD_FIELD_INFO("29/13",      "stride-length-samples",      Uint32);
    ADD_FIELD_INFO("29/14",      "stride-calibration",         EmbeddedMessage);
    ADD_FIELD_INFO("29/14/1",    "start-index",                Uint32);
    ADD_FIELD_INFO("29/14/2",    "value",                      Float);
    ADD_FIELD_INFO("29/14/3",    "operation",                  Enumerator); // 1=Multiply, 2=Sum
    ADD_FIELD_INFO("29/14/4",    "cause",                      Enumerator); // 0=Walk, 1=Run, 2=Stand
    ADD_FIELD_INFO("29/15",      "left-pedal-power-samples",   EmbeddedMessage);
    ADD_FIELD_INFO("29/15/1",    "current-power",              Int32);
    ADD_FIELD_INFO("29/15/2",    "cumulative-crank-revs",      Uint32);
    ADD_FIELD_INFO("29/15/3",    "cumulative-timestamp",       Uint32);
    ADD_FIELD_INFO("29/15/4",    "min-force-magnitude",        Sint32);
    ADD_FIELD_INFO("29/15/5",    "max-force-magnitude",        Int32);
    ADD_FIELD_INFO("29/15/6",    "min-force-angle",            Uint32);
    ADD_FIELD_INFO("29/15/7",    "max-force-angle",            Uint32);
    ADD_FIELD_INFO("29/15/8",    "bottom-dead-spot",           Uint32);
    ADD_FIELD_INFO("29/15/9",    "top-dead-spot",              Uint32);
    ADD_FIELD_INFO("29/15/10",    "pedal-power-balance",       Uint32);
    ADD_FIELD_INFO("29/15/11",    "min-torque-magnitude",      Int32);
    ADD_FIELD_INFO("29/15/12",    "man-torque-magnitude",      Int32);
    ADD_FIELD_INFO("29/16",       "right-pedal-power-samples", EmbeddedMessage);
    ADD_FIELD_INFO("29/16/1",     "current-power",             Int32);
    ADD_FIELD_INFO("29/16/2",     "cumulative-crank-revs",     Uint32);
    ADD_FIELD_INFO("29/16/3",     "cumulative-timestamp",      Uint32);
    ADD_FIELD_INFO("29/16/4",     "min-force-magnitude",       Sint32);
    ADD_FIELD_INFO("29/16/5",     "max-force-magnitude",       Int32);
    ADD_FIELD_INFO("29/16/6",     "min-force-angle",           Uint32);
    ADD_FIELD_INFO("29/16/7",     "max-force-angle",           Uint32);
    ADD_FIELD_INFO("29/16/8",     "bottom-dead-spot",          Uint32);
    ADD_FIELD_INFO("29/16/9",     "top-dead-spot",             Uint32);
    ADD_FIELD_INFO("29/16/10",    "pedal-power-balance",       Uint32);
    ADD_FIELD_INFO("29/16/11",    "min-torque-magnitude",      Int32);
    ADD_FIELD_INFO("29/16/12",    "man-torque-magnitude",      Int32);
    ADD_FIELD_INFO("29/17",       "left-power-calibration",    EmbeddedMessage);
    ADD_FIELD_INFO("29/17/1",     "start-index",               Uint32);
    ADD_FIELD_INFO("29/17/2",     "value",                     Float);
    ADD_FIELD_INFO("29/17/3",     "operation",                 Enumerator); // 1=Multiply, 2=Sum
    ADD_FIELD_INFO("29/17/4",     "cause",                     Enumerator); // 0=Walk, 1=Run, 2=Stand
    ADD_FIELD_INFO("29/18",       "right-power-calibration",   EmbeddedMessage);
    ADD_FIELD_INFO("29/18/1",     "start-index",               Uint32);
    ADD_FIELD_INFO("29/18/2",     "value",                     Float);
    ADD_FIELD_INFO("29/18/3",     "operation",                 Enumerator); // 1=Multiply, 2=Sum
    ADD_FIELD_INFO("29/18/4",     "cause",                     Enumerator); // 0=Walk, 1=Run, 2=Stand
    ADD_FIELD_INFO("29/19",       "rr-samples",                EmbeddedMessage);
    ADD_FIELD_INFO("29/19/1",     "rr-intervals",              Uint32);
    ADD_FIELD_INFO("29/19/2",     "rr-sensor-offline",         EmbeddedMessage);
    ADD_FIELD_INFO("29/19/2/1",   "start-time",                EmbeddedMessage);
    ADD_FIELD_INFO("29/19/2/1/1", "hour",                      Uint32);
    ADD_FIELD_INFO("29/19/2/1/2", "minute",                    Uint32);
    ADD_FIELD_INFO("29/19/2/1/3", "seconds",                   Uint32);
    ADD_FIELD_INFO("29/19/2/1/4", "milliseconds",              Uint32);
    ADD_FIELD_INFO("29/19/2/2",   "time-interval",             EmbeddedMessage);
    ADD_FIELD_INFO("29/19/2/2/1", "hour",                      Uint32);
    ADD_FIELD_INFO("29/19/2/2/2", "minute",                    Uint32);
    ADD_FIELD_INFO("29/19/2/2/3", "seconds",                   Uint32);
    ADD_FIELD_INFO("29/19/2/2/4", "milliseconds",              Uint32);
    ADD_FIELD_INFO("29/20",       "acceleration-mad-samples",  Float);
    ADD_FIELD_INFO("30",          "pause-times",               EmbeddedMessage);
    ADD_FIELD_INFO("30/1",        "starttime",                 EmbeddedMessage);
    ADD_FIELD_INFO("30/1/1",      "hours",                     Uint32);
    ADD_FIELD_INFO("30/1/2",      "minutes",                   Uint32);
    ADD_FIELD_INFO("30/1/3",      "seconds",                   Uint32);
    ADD_FIELD_INFO("30/1/4",      "milliseconds",              Uint32);
    ADD_FIELD_INFO("30/2"  ,      "duration",                  EmbeddedMessage);
    ADD_FIELD_INFO("30/2/1",      "hours",                     Uint32);
    ADD_FIELD_INFO("30/2/2",      "minutes",                   Uint32);
    ADD_FIELD_INFO("30/2/3",      "seconds",                   Uint32);
    ADD_FIELD_INFO("30/2/4",      "milliseconds",              Uint32);

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
        qWarning() << "Failed to open samples file" << fileName;
        return QVariantMap();
    }
    return parseSamples(file);
}

QVariantMap TrainingSession::parseStatistics(QIODevice &data) const
{
    ProtoBuf::Message::FieldInfoMap fieldInfo;
    ADD_FIELD_INFO("1",    "heartrate",      EmbeddedMessage);
    ADD_FIELD_INFO("1/1",  "minimum",        Uint32);
    ADD_FIELD_INFO("1/2",  "average",        Uint32);
    ADD_FIELD_INFO("1/3",  "maximum",        Uint32);
    ADD_FIELD_INFO("2",    "speed",          EmbeddedMessage);
    ADD_FIELD_INFO("2/1",  "average",        Float);
    ADD_FIELD_INFO("2/2",  "maximum",        Float);
    ADD_FIELD_INFO("3",    "cadence",        EmbeddedMessage);
    ADD_FIELD_INFO("3/1",  "average",        Uint32);
    ADD_FIELD_INFO("3/2",  "maximum",        Uint32);
    ADD_FIELD_INFO("4",    "altitude",       EmbeddedMessage);
    ADD_FIELD_INFO("4/1",  "minimum",        Float);
    ADD_FIELD_INFO("4/2",  "average",        Float);
    ADD_FIELD_INFO("4/3",  "maximum",        Float);
    ADD_FIELD_INFO("5",    "power",          EmbeddedMessage);
    ADD_FIELD_INFO("5/1",  "average",        Uint32);
    ADD_FIELD_INFO("5/2",  "maximum",        Uint32);
    ADD_FIELD_INFO("6",    "lr_balance",     EmbeddedMessage);
    ADD_FIELD_INFO("6/1",  "average",        Float);
    ADD_FIELD_INFO("7",    "temperature",    EmbeddedMessage);
    ADD_FIELD_INFO("7/1",  "minimum",        Float);
    ADD_FIELD_INFO("7/2",  "average",        Float);
    ADD_FIELD_INFO("7/3",  "maximum",        Float);
    ADD_FIELD_INFO("8",    "activity",       EmbeddedMessage);
    ADD_FIELD_INFO("8/1",  "average",        Float);
    ADD_FIELD_INFO("9",    "stride",         EmbeddedMessage);
    ADD_FIELD_INFO("9/1",  "average",        Uint32);
    ADD_FIELD_INFO("9/2",  "maximum",        Uint32);
    ADD_FIELD_INFO("10",   "include",        EmbeddedMessage);
    ADD_FIELD_INFO("10/1", "average",        Float);
    ADD_FIELD_INFO("10/2", "maximum",        Float);
    ADD_FIELD_INFO("11",   "declince",       EmbeddedMessage);
    ADD_FIELD_INFO("11/1", "average",        Float);
    ADD_FIELD_INFO("11/2", "maximum",        Float);
    ADD_FIELD_INFO("12",       "swimming",          EmbeddedMessage);
    ADD_FIELD_INFO("12/1",     "distance",          Float);
    ADD_FIELD_INFO("12/2",     "freestyle",         EmbeddedMessage);
    ADD_FIELD_INFO("12/2/1",   "distance",          Float);
    ADD_FIELD_INFO("12/2/2",   "strokes",           Uint32);
    ADD_FIELD_INFO("12/2/3",   "duration",          EmbeddedMessage);
    ADD_FIELD_INFO("12/2/3/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/2/3/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/2/3/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/2/3/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/2/4",   "average-heartrate", Uint32);
    ADD_FIELD_INFO("12/2/5",   "maximum-heartate",  Uint32);
    ADD_FIELD_INFO("12/2/6",   "average-swolf",     Uint32);
    ADD_FIELD_INFO("12/2/7",   "pool-time",         EmbeddedMessage);
    ADD_FIELD_INFO("12/2/7/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/2/7/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/2/7/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/2/7/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/3",     "backstroke",        EmbeddedMessage);
    ADD_FIELD_INFO("12/3/1",   "distance",          Float);
    ADD_FIELD_INFO("12/3/2",   "strokes",           Uint32);
    ADD_FIELD_INFO("12/3/3",   "duration",          EmbeddedMessage);
    ADD_FIELD_INFO("12/3/3/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/3/3/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/3/3/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/3/3/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/3/4",   "average-heartrate", Uint32);
    ADD_FIELD_INFO("12/3/5",   "maximum-heartate",  Uint32);
    ADD_FIELD_INFO("12/3/6",   "average-swolf",     Uint32);
    ADD_FIELD_INFO("12/3/7",   "pool-time",         EmbeddedMessage);
    ADD_FIELD_INFO("12/3/7/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/3/7/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/3/7/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/3/7/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/4",     "breaststroke",      EmbeddedMessage);
    ADD_FIELD_INFO("12/4/1",   "distance",          Float);
    ADD_FIELD_INFO("12/4/2",   "strokes",           Uint32);
    ADD_FIELD_INFO("12/4/3",   "duration",          EmbeddedMessage);
    ADD_FIELD_INFO("12/4/3/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/4/3/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/4/3/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/4/3/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/4/4",   "average-heartrate", Uint32);
    ADD_FIELD_INFO("12/4/5",   "maximum-heartate",  Uint32);
    ADD_FIELD_INFO("12/4/6",   "average-swolf",     Float);
    ADD_FIELD_INFO("12/4/7",   "pool-time",         EmbeddedMessage);
    ADD_FIELD_INFO("12/4/7/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/4/7/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/4/7/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/4/7/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/5",     "butterfly",         EmbeddedMessage);
    ADD_FIELD_INFO("12/5/1",   "distance",          Float);
    ADD_FIELD_INFO("12/5/2",   "strokes",           Uint32);
    ADD_FIELD_INFO("12/5/3",   "duration",          EmbeddedMessage);
    ADD_FIELD_INFO("12/5/3/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/5/3/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/5/3/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/5/3/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/5/4",   "average-heartrate", Uint32);
    ADD_FIELD_INFO("12/5/5",   "maximum-heartate",  Uint32);
    ADD_FIELD_INFO("12/5/6",   "average-swolf",     Uint32);
    ADD_FIELD_INFO("12/5/7",   "pool-time",         EmbeddedMessage);
    ADD_FIELD_INFO("12/5/7/1", "hours",             Uint32);
    ADD_FIELD_INFO("12/5/7/2", "minutes",           Uint32);
    ADD_FIELD_INFO("12/5/7/3", "seconds",           Uint32);
    ADD_FIELD_INFO("12/5/7/4", "milliseconds",      Uint32);
    ADD_FIELD_INFO("12/6",     "strokes",           Uint32);
    ADD_FIELD_INFO("12/7",     "pools",             Uint32);
    ADD_FIELD_INFO("12/8",     "pool-info",         EmbeddedMessage);
    ADD_FIELD_INFO("12/8/1",   "length",            Float);
    ADD_FIELD_INFO("12/8/2",   "units",             Enumerator);
    ProtoBuf::Message parser(fieldInfo);

    if (isGzipped(data)) {
        QByteArray array = unzip(data.readAll());
        return parser.parse(array);
    } else {
        return parser.parse(data);
    }
}

QVariantMap TrainingSession::parseStatistics(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open stats file" << fileName;
        return QVariantMap();
    }
    return parseStatistics(file);
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
    ADD_FIELD_INFO("1/2/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("2",     "power",            EmbeddedMessage);
    ADD_FIELD_INFO("2/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("2/1/1", "low",              Uint32);
    ADD_FIELD_INFO("2/1/2", "high",             Uint32);
    ADD_FIELD_INFO("2/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("2/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("2/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("2/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("2/2/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3",     "fatfit",           EmbeddedMessage);
    ADD_FIELD_INFO("3/1",   "limit",            Uint32);
    ADD_FIELD_INFO("3/2",   "fit-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("3/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/2/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("3/3",   "fat-duration",     EmbeddedMessage);
    ADD_FIELD_INFO("3/3/1", "hours",            Uint32);
    ADD_FIELD_INFO("3/3/2", "minutes",          Uint32);
    ADD_FIELD_INFO("3/3/3", "seconds",          Uint32);
    ADD_FIELD_INFO("3/3/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("4",     "speed",            EmbeddedMessage);
    ADD_FIELD_INFO("4/1",   "limits",           EmbeddedMessage);
    ADD_FIELD_INFO("4/1/1", "low",              Float);
    ADD_FIELD_INFO("4/1/2", "high",             Float);
    ADD_FIELD_INFO("4/2",   "duration",         EmbeddedMessage);
    ADD_FIELD_INFO("4/2/1", "hours",            Uint32);
    ADD_FIELD_INFO("4/2/2", "minutes",          Uint32);
    ADD_FIELD_INFO("4/2/3", "seconds",          Uint32);
    ADD_FIELD_INFO("4/2/4", "milliseconds",     Uint32);
    ADD_FIELD_INFO("4/3",   "distance",         Float);
    ADD_FIELD_INFO("10",    "heartrate-source", Enumerator);
    ADD_FIELD_INFO("11",    "power-source",     Enumerator);
    ADD_FIELD_INFO("12",    "speed-source",     Enumerator);
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
        qWarning() << "Failed to open zones file" << fileName;
        return QVariantMap();
    }
    return parseZones(file);
}

void TrainingSession::setGpxOption(const GpxOption option, const bool enabled)
{
    if (enabled) {
        gpxOptions |= option;
    } else {
        gpxOptions &= ~option;
    }
}

void TrainingSession::setGpxOptions(const GpxOptions options)
{
    gpxOptions = options;
}

void TrainingSession::setHrmOption(const HrmOption option, const bool enabled)
{
    if (enabled) {
        hrmOptions |= option;
    } else {
        hrmOptions &= ~option;
    }
}

void TrainingSession::setHrmOptions(const HrmOptions options)
{
    hrmOptions = options;
}

void TrainingSession::setTcxOption(const TcxOption option, const bool enabled)
{
    if (enabled) {
        tcxOptions |= option;
    } else {
        tcxOptions &= ~option;
    }
}

void TrainingSession::setTcxOptions(const TcxOptions options)
{
    tcxOptions = options;
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

QString getFileName(const QString &file)
{
    const QFileInfo info(file);
    return info.fileName();
}

QString hrmTime(const QVariantMap &map)
{
    const QVariantMap::const_iterator
        hours        = map.constFind(QLatin1String("hours")),
        minutes      = map.constFind(QLatin1String("minutes")),
        seconds      = map.constFind(QLatin1String("seconds")),
        milliseconds = map.constFind(QLatin1String("milliseconds"));
    return QString::fromLatin1("%1:%2:%3.%4")
        .arg((hours   == map.constEnd()) ? 0 : first(  hours.value()).toUInt(), 2, 10, QLatin1Char('0'))
        .arg((minutes == map.constEnd()) ? 0 : first(minutes.value()).toUInt(), 2, 10, QLatin1Char('0'))
        .arg((seconds == map.constEnd()) ? 0 : first(seconds.value()).toUInt(), 2, 10, QLatin1Char('0'))
        .arg((milliseconds == map.constEnd()) ? 0.0 : qRound(qMin(900u, first(milliseconds.value()).toUInt())/100.0));
}

QString hrmTime(const QTime &time)
{
    return QString::fromLatin1("%1.%2")
            .arg(time.toString(QLatin1String("HH:mm:ss")))
            .arg(qRound(time.msec()/100.0));
}

bool sensorOffline(const QVariantList &list, const int index)
{
    foreach (const QVariant &entry, list) {
        const QVariantMap map = entry.toMap();
        const QVariant startIndex = first(map.value(QLatin1String("start-index")));
        const QVariant endIndex = first(map.value(QLatin1String("start-index")));
        if ((!startIndex.canConvert(QMetaType::Int)) ||
            (!endIndex.canConvert(QMetaType::Int))) {
            qWarning() << "Ignoring invalid 'offline' entry" << entry;
            continue;
        }
        if ((startIndex.toInt() <= index) && (index <= endIndex.toInt())) {
            return true; // Sensor was offline.
        }
    }
    return false; // Sensor was not offline.
}

bool haveAnySamples(const QVariantMap &samples, const QString &type)
{
    const int size = samples.value(type).toList().length();
    for (int index = 0; index < size; ++index) {
        if (!sensorOffline(samples.value(type + QLatin1String("-offline")).toList(), index)) {
            return true;
        }
    }
    return false;
}

QString TrainingSession::getOutputBaseFileName(const QString &format)
{
    const QFileInfo inputBaseNameInfo(baseName);
    if (format.isEmpty()) {
        return inputBaseNameInfo.fileName();
    }
    QRegExp inputFileNameParts(
        QLatin1String("v2-users-([^-]+)-training-sessions-([^-]+)"));
    if (format.contains(QLatin1String("$userId"   )) ||
        format.contains(QLatin1String("$sessionId"))) {
        if (!inputFileNameParts.exactMatch(inputBaseNameInfo.fileName())) {
            qWarning() << "Base name does not match format" << baseName;
            return QString();
        }
    }

    QString fileName = format;

    // If any of these placeholders are used, ensure we've parsed the base details.
    if (format.contains(QLatin1String("$date"       )) ||
        format.contains(QLatin1String("$dateUTC"    )) ||
        format.contains(QLatin1String("$time"       )) ||
        format.contains(QLatin1String("$timeUTC"    )) ||
        format.contains(QLatin1String("$userId"     )) ||
        format.contains(QLatin1String("$sessionId"  )) ||
        format.contains(QLatin1String("$sessionName"))) {
        if (parsedSession.isEmpty()) {
            parsedSession = parseCreateSession(baseName + QLatin1String("-create"));
        }
    }

    fileName.replace(QLatin1String("$baseName"), inputBaseNameInfo.fileName());

    if (format.contains(QLatin1String("$date"      )) ||
        format.contains(QLatin1String("$dateUTC"   )) ||
        format.contains(QLatin1String("$dateExt"   )) ||
        format.contains(QLatin1String("$dateExtUTC")) ||
        format.contains(QLatin1String("$time"      )) ||
        format.contains(QLatin1String("$timeUTC"   )) ||
        format.contains(QLatin1String("$timeExt"   )) ||
        format.contains(QLatin1String("$timeExtUTC")))
    {
        const QDateTime startTime =
            getDateTime(firstMap(parsedSession.value(QLatin1String("start"))));
        fileName.replace(QLatin1String("$dateExtUTC"),
             startTime.toUTC().toString(QLatin1String("yyyy-MM-dd")));
        fileName.replace(QLatin1String("$dateExt"),
             startTime.toString(QLatin1String("yyyy-MM-dd")));
        fileName.replace(QLatin1String("$dateUTC"),
             startTime.toUTC().toString(QLatin1String("yyyyMMdd")));
        fileName.replace(QLatin1String("$date"),
             startTime.toString(QLatin1String("yyyyMMdd")));
        fileName.replace(QLatin1String("$timeExtUTC"),
            startTime.toUTC().toString(QLatin1String("HH:mm:ss")));
        fileName.replace(QLatin1String("$timeExt"),
            startTime.toString(QLatin1String("HH:mm:ss")));
        fileName.replace(QLatin1String("$timeUTC"),
            startTime.toUTC().toString(QLatin1String("HHmmss")));
        fileName.replace(QLatin1String("$time"),
            startTime.toString(QLatin1String("HHmmss")));
    }

    if (format.contains(QLatin1String("$userId"))) {
        fileName.replace(QLatin1String("$userId"), inputFileNameParts.cap(1));
    }

    if (format.contains(QLatin1String("$username"))) {
        const QString user = QProcessEnvironment::systemEnvironment().value(
            #ifdef Q_OS_WIN
            QLatin1String("USERNAME"),
            #else
            QLatin1String("USER"),
            #endif
            QLatin1String("unknown")
        );
        fileName.replace(QLatin1String("$username"), user);
    }

    if (format.contains(QLatin1String("$sessionId"))) {
        fileName.replace(QLatin1String("$sessionId"), inputFileNameParts.cap(2));
    }

    // If there are any $sessionName references
    if (fileName.contains(QStringLiteral("$sessionName"))) {
        // Fetch the session name from the sesion.
        QString sessionName = first(firstMap(parsedSession.value(QLatin1String("session-name")))
                                    .value(QLatin1String("text"))).toString();

        // If session name is empty (eg common for Vantage V), then fallback to the exercise name.
        if (sessionName.isEmpty()) {
            // If we haven't parsed the exercise data yet (really only happens in unit test), do so.
            if (exerciseCount() < 1) {
                parse();
            }

            // Build a unique set of sport names from the individual exercises in the session.
            QSet<QString> sportNames;
            foreach (const QVariant &exercise, parsedExercises) {
                const QString sportName = getPolarSportName(first(firstMap(exercise.toMap()
                    .value(CREATE).toMap().value(QStringLiteral("sport")))
                    .value(QStringLiteral("value"))).toULongLong());
                qDebug() << "No session name, found Polar sport name" << sportName;
                if (!sportName.isNull()) {
                    sportNames.insert(sportName);
                }
            }

            // Pick an appropriate session name from the sport names.
            if (sportNames.isEmpty()) {
                qWarning() << "No session name, and no recognised sport names either";
                sessionName = QStringLiteral("Unknown session");
            } else if (sportNames.size() > 1) {
                qWarning() << "No session name, and multiple unique sport names";
                sessionName = QStringLiteral("Multisport");
            } else {
                sessionName = *sportNames.constBegin();
            }
        }
        fileName.replace(QLatin1String("$sessionName"), sessionName);
    }
    return fileName;
}

QStringList TrainingSession::getOutputFileNames(const QString &fileNameFormat,
                                                const OutputFormats outputFormats,
                                                QString outputDirName)
{
    // Default the output directory match the input files, if not specified.
    if (outputDirName.isEmpty()) {
        outputDirName = QFileInfo(this->baseName).absoluteDir().absolutePath();
    }

    const QString baseName = outputDirName + QLatin1Char('/') +
        getOutputBaseFileName(fileNameFormat);

    QStringList fileNames;

    if (outputFormats & GpxOutput) {
        fileNames.append(baseName + QLatin1String(".gpx"));
    }

    if (outputFormats & HrmOutput) {
        const QFileInfo fileInfo(this->baseName);
        const int exerciseCount = fileInfo.dir().entryInfoList(QStringList(
            fileInfo.fileName() + QLatin1String("-exercises-*-create"))).count();
        if (exerciseCount == 1) {
            fileNames.append(baseName + QLatin1String(".hrm"));
            if (hrmOptions.testFlag(RrFiles)) {
                fileNames.append(baseName + QLatin1String(".rr.hrm"));
            }
        } else {
            for (int index = 0; index < exerciseCount; ++index) {
                fileNames.append(QString::fromLatin1("%1.%2.hrm")
                    .arg(baseName).arg(index));
                if (hrmOptions.testFlag(RrFiles)) {
                    fileNames.append(QString::fromLatin1("%1.%2.rr.hrm")
                        .arg(baseName).arg(index));
                }
            }
        }
    }

    if (outputFormats & TcxOutput) {
        fileNames.append(baseName + QLatin1String(".tcx"));
    }

    return fileNames;
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
    if (gpxOptions.testFlag(CluetrustGpxDataExtension)) {
        gpx.setAttribute(QLatin1String("xmlns:gpxdata"),
                         QLatin1String("http://www.cluetrust.com/XML/GPXDATA/1/0"));
    }
    if (gpxOptions.testFlag(GarminAccelerationExtension)) {
        gpx.setAttribute(QLatin1String("xmlns:gpxax"),
                         QLatin1String("http://www.garmin.com/xmlschemas/AccelerationExtension/v1"));
    }
    if (gpxOptions.testFlag(GarminTrackPointExtension)) {
        gpx.setAttribute(QLatin1String("xmlns:gpxtpx"),
                         QLatin1String("http://www.garmin.com/xmlschemas/TrackPointExtension/v1"));
    }
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

        const QVariantMap route = map.value(ROUTE).toMap();
        if (!route.isEmpty()) {
            // Get the starting time.
            const QDateTime startTime = getDateTime(firstMap(
                route.value(QLatin1String("timestamp"))));

            // Get the "samples" samples.
            const QVariantMap samples = map.value(SAMPLES).toMap();
            const QVariantList cadence             = samples.value(QLatin1String("cadence")).toList();
            const QVariantList distance            = samples.value(QLatin1String("distance")).toList();
            const QVariantList forwardAcceleration = samples.value(QLatin1String("fwd-acceleration")).toList();
            const QVariantList heartrate           = samples.value(QLatin1String("heartrate")).toList();
            const QVariantList temperature         = samples.value(QLatin1String("temperature")).toList();

            // Get the "route" samples.
            const QVariantList altitude   = route.value(QLatin1String("altitude")).toList();
            const QVariantList duration   = route.value(QLatin1String("duration")).toList();
            const QVariantList latitude   = route.value(QLatin1String("latitude")).toList();
            const QVariantList longitude  = route.value(QLatin1String("longitude")).toList();
            const QVariantList satellites = route.value(QLatin1String("satellites")).toList();
            if ((duration.size() != altitude.size())  ||
                (duration.size() != latitude.size())  ||
                (duration.size() != longitude.size()) ||
                (duration.size() != satellites.size())) {
                qWarning() << "Sample lists not all equal sizes:" << duration.size()
                           << altitude.size() << latitude.size()
                           << longitude.size() << satellites.size();
            }

            // Build a list of lap split times.
            QVariantList laps = map.value(LAPS).toMap().value(QLatin1String("laps")).toList();
            if (laps.isEmpty()) {
                laps = map.value(AUTOLAPS).toMap().value(QLatin1String("laps")).toList();
            }
            QList<quint64> splits;
            foreach (const QVariant &lap, laps) {
                const quint64 splitTime = getDuration(firstMap(firstMap(
                    lap.toMap().value(QLatin1String("header")))
                    .value(QLatin1String("split-time"))));
                if (splitTime > 0) {
                    splits.append(splitTime);
                }
            }
            #if defined Q_CC_MSVC && defined Q_OS_WIN64 && (QT_VERSION <= QT_VERSION_CHECK(5, 4, 0))
            qSort(splits); // QTBUG-41092
            #else
            std::sort(splits.begin(), splits.end());
            #endif

            // Add trkseg elements containing the actual GPS data.
            QDomElement trkseg = doc.createElement(QLatin1String("trkseg"));
            trk.appendChild(trkseg);
            for (int index = 0; index < duration.size(); ++index) {
                const quint32 timeOffset = duration.at(index).toUInt();
                if ((!splits.isEmpty()) && (timeOffset > splits.first())) {
                    trkseg = doc.createElement(QLatin1String("trkseg"));
                    trk.appendChild(trkseg);
                    splits.removeFirst();
                }

                QDomElement trkpt = doc.createElement(QLatin1String("trkpt"));
                trkpt.setAttribute(QLatin1String("lat"), VARIANT_TO_STRING(latitude.at(index)));
                trkpt.setAttribute(QLatin1String("lon"), VARIANT_TO_STRING(longitude.at(index)));
                trkpt.appendChild(doc.createElement(QLatin1String("ele")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(altitude.at(index))));
                trkpt.appendChild(doc.createElement(QLatin1String("time")))
                    .appendChild(doc.createTextNode(startTime.addMSecs(
                        timeOffset).toString(Qt::ISODate)));
                trkpt.appendChild(doc.createElement(QLatin1String("sat")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(satellites.at(index))));

                if (gpxOptions.testFlag(CluetrustGpxDataExtension)   ||
                    gpxOptions.testFlag(GarminAccelerationExtension) ||
                    gpxOptions.testFlag(GarminTrackPointExtension))
                {
                    QDomElement extensions = doc.createElement(QLatin1String("extensions"));

                    if (gpxOptions.testFlag(CluetrustGpxDataExtension)) {
                        if ((index < heartrate.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("heartrate-offline")).toList(), index))) {
                            extensions.appendChild(doc.createElement(QLatin1String("gpxdata:hr")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(heartrate.at(index).toUInt())));
                        }

                        if ((index < cadence.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("altitude-offline")).toList(), index))) {
                            extensions.appendChild(doc.createElement(QLatin1String("gpxdata:cadence")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(cadence.at(index).toUInt())));
                        }

                        if (index < temperature.length()) {
                            extensions.appendChild(doc.createElement(QLatin1String("gpxdata:temp")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(temperature.at(index).toFloat())));
                        }

                        if ((index < distance.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("distance-offline")).toList(), index))) {
                            /// @todo  Include optional gpxdata:sensor="wheel|pedometer" attribute.
                            extensions.appendChild(doc.createElement(QLatin1String("gpxdata:distance")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(distance.at(index).toUInt())));
                        }
                    }

                    if (gpxOptions.testFlag(GarminAccelerationExtension)) {
                        QDomElement accelerationExtension = doc.createElement(
                            QLatin1String("gpxax:AccelerationExtension"));

                        if ((index < forwardAcceleration.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("fwd-acceleration-offline")).toList(), index))) {
                            QDomElement accel = doc.createElement(QLatin1String("gpxax:accel"));
                            accel.setAttribute(QLatin1String("x"), QString::fromLatin1("%1")
                                .arg(forwardAcceleration.at(index).toFloat()));
                            accel.setAttribute(QLatin1String("y"), QLatin1String("0"));
                            accel.setAttribute(QLatin1String("z"), QLatin1String("0"));
                            accelerationExtension.appendChild(accel);
                        }

                        extensions.appendChild(accelerationExtension);
                    }

                    if (gpxOptions.testFlag(GarminTrackPointExtension)) {
                        QDomElement trackPointExtension = doc.createElement(
                            QLatin1String("gpxtpx:TrackPointExtension"));

                        if (index < temperature.length()) {
                            trackPointExtension.appendChild(doc.createElement(QLatin1String("gpxtpx:atemp")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(temperature.at(index).toFloat())));
                        }

                        if ((index < heartrate.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("heartrate-offline")).toList(), index))) {
                            const uint hr = heartrate.at(index).toUInt();
                            if ((hr >= 1) && (hr <= 255)) { // Schema enforced.
                                trackPointExtension.appendChild(doc.createElement(QLatin1String("gpxtpx:hr")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1").arg(hr)));
                            }
                        }

                        if ((index < cadence.length()) &&
                            (!sensorOffline(samples.value(QLatin1String("altitude-offline")).toList(), index))) {
                            const uint cad = cadence.at(index).toUInt();
                            if (cad <= 254) { // Schema enforced.
                            trackPointExtension.appendChild(doc.createElement(QLatin1String("gpxtpx:cad")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1").arg(cad)));
                            }
                        }

                        extensions.appendChild(trackPointExtension);
                    }

                    trkpt.appendChild(extensions);
                }
                trkseg.appendChild(trkpt);
            }
        }
    }
    return doc;
}

// Sometimes Polar devices generate a separate rrsamples data file which is just
// a flat list of R-R intervals.  However, other times, Polar devices include HRV
// data interspersed with other exercise sample data - which is a bit odd considering
// that R-R intervals are, by there very nature, not sampled at regular intervals as
// all of the other exercise sample data is.  Anyway, toHRM will use rrsamples if
// available, and fallback to exercise sample HRM values if not. To assist in the
// latter case, this funciton flattens the exercise sample HRV data into a list
// of intervals the same as the rrsample data files would, making both sources look
// the same throughout the toHRM function.
QVariantList flattenHrvSamplesForHrm(const QVariantMap &samples)
{
    QVariantList rrsamples;
    foreach (const QVariant &hrv, samples.value(QLatin1String("heartrate-variability")).toList()) {
        rrsamples.append(hrv.toMap().value(QLatin1String("intervals")).toList());
        // Note, ignorig hrv["offline"] values - no way to apply them to HRM.
    }
    return rrsamples;
}

/// @see http://www.polar.com/files/Polar_HRM_file%20format.pdf
QStringList TrainingSession::toHRM(const bool rrDataOnly) const
{
    QStringList hrmList;

    foreach (const QVariant &exercise, parsedExercises) {
        const QVariantMap map = exercise.toMap();
        const QVariantMap autoLaps   = map.value(AUTOLAPS).toMap();
        const QVariantMap create     = map.value(CREATE).toMap();
        const QVariantMap manualLaps = map.value(LAPS).toMap();
        const QVariantMap samples    = map.value(SAMPLES).toMap();
        const QVariantMap stats      = map.value(STATISTICS).toMap();
        const QVariantMap zones      = map.value(ZONES).toMap();

        const QVariantList rrsamples = map.contains(RRSAMPLES)
            ? map.value(RRSAMPLES).toMap().value(QLatin1String("value")).toList()
            : flattenHrvSamplesForHrm(samples);

        const bool haveAltitude     = ((!rrDataOnly) && (haveAnySamples(samples, QLatin1String("altitude"))));
        const bool haveCadence      = ((!rrDataOnly) && (haveAnySamples(samples, QLatin1String("cadence"))));
        const bool havePowerLeft    = ((!rrDataOnly) && (haveAnySamples(samples, QLatin1String("left-pedal-power"))));
        const bool havePowerRight   = ((!rrDataOnly) && (haveAnySamples(samples, QLatin1String("right-pedal-power"))));
        const bool havePower        = (havePowerLeft || havePowerRight);
        const bool havePowerBalance = havePower;
        const bool haveSpeed        = ((!rrDataOnly) && (haveAnySamples(samples, QLatin1String("speed"))));

        QString hrmData;
        QTextStream stream(&hrmData);

        // [Params]
        stream <<
            "[Params]\r\n"
            "Version=106\r\n"
            "Monitor=1\r\n"
            "SMode=";
        stream << (haveSpeed        ? '1' : '0'); // a) Speed
        stream << (haveCadence      ? '1' : '0'); // b) Cadence
        stream << (haveAltitude     ? '1' : '0'); // c) Altitude
        stream << (havePower        ? '1' : '0'); // d) Power
        stream << (havePowerBalance ? '1' : '0'); // e) Power Left Right Balance
        // Note, we're forcing Power Pedalling Index on when we have power data,
        // even though we have no such pedalling index data. This is for PPT5's
        // benefit.  See issue #57 - https://github.com/pcolby/bipolar/issues/57
        stream << (havePowerBalance ? '1' : '0'); // f) Power Pedalling Index
        stream <<
            "0" // g) HR/CC data (available only with Polar XTrainer Plus).
            "0" // h) US / Euro unit (always metric).
            "0" // i) Air pressure (not available).
            "\r\n";

        const QDateTime startTime = getDateTime(firstMap(create.value(QLatin1String("start"))));
        const quint64 recordInterval = getDuration(firstMap(samples.value(QLatin1String("record-interval"))));
        stream << "Date="      << startTime.toString(QLatin1String("yyyyMMdd")) << "\r\n";
        stream << "StartTime=" << hrmTime(startTime.time()) << "\r\n";
        stream << "Length="    << hrmTime(firstMap(create.value(QLatin1String("duration")))) << "\r\n";
        stream << "Interval="  << (rrDataOnly ? 238 : qRound(recordInterval / 1000.0)) << "\r\n";

        // In the absence of available training target phases data, just include
        // one of the static target HR zones (better than nothing). We'll use
        // the one with the greatest duration (why not?).
        QVariantList hrZones = zones.value(QLatin1String("heartrate")).toList();
        quint64 hrZoneMaxDuration = 0;
        QVariantMap longestHrZone;
        for (int index = 0; (hrZones.length() > 3) && (index < hrZones.length()); ++index) {
            const QVariantMap hrZone = hrZones.at(index).toMap();
            const quint64 duration = getDuration(firstMap(hrZone.value(QLatin1String("duration"))));
            if ((duration > hrZoneMaxDuration) || (hrZoneMaxDuration == 0)) {
                longestHrZone = hrZone;
                hrZoneMaxDuration = duration;
            }
        }
        const QVariantMap phase1Limits = firstMap(longestHrZone.value(QLatin1String("limits")));
        const quint32 phase1LimitHigh = first(phase1Limits.value(QLatin1String("high"))).toUInt();
        const quint32 phase1LimitLow  = first(phase1Limits.value(QLatin1String("low"))).toUInt();
        stream << "Upper1=" << phase1LimitHigh << "\r\n";
        stream << "Lower1=" << phase1LimitLow << "\r\n";
        stream << "Upper2=0\r\n";
        stream << "Lower2=0\r\n";
        stream << "Upper3=0\r\n";
        stream << "Lower3=0\r\n";
        stream << "Timer1=" << hrmTime(firstMap(longestHrZone.value(QLatin1String("duration")))) << "\r\n";
        stream << "Timer2=00:00:00.0\r\n";
        stream << "Timer3=00:00:00.0\r\n";
        stream << "ActiveLimit=0\r\n";

        const quint32 hrMax = first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("maximum-heartrate"))).value(QLatin1String("value"))).toUInt();
        const quint32 hrRest = first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("resting-heartrate"))).value(QLatin1String("value"))).toUInt();
        stream << "MaxHR="  << hrMax  << "\r\n";
        stream << "RestHR=" << hrRest << "\r\n";
        stream << "StartDelay=0\r\n"; ///< "Vantage NV RR data only".
        stream << "VO2max=" << first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("vo2max"))).value(QLatin1String("value"))).toUInt() << "\r\n";
        stream << "Weight=" << first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("weight"))).value(QLatin1String("value"))).toFloat() << "\r\n";

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
        stream << "\r\n";

        // [HRZones]
        QMap<quint32, quint32> hrLimits; // Map hr-high to hr-low.
        foreach (const QVariant &entry, zones.value(QLatin1String("heartrate")).toList()) {
            const QVariantMap limits = firstMap(entry.toMap().value(QLatin1String("limits")));
            hrLimits.insert(
                first(limits.value(QLatin1String("high"))).toUInt(),
                first(limits.value(QLatin1String("low"))).toUInt());
        }
        // Limit to maximum of 10 HRZones (as implied by HRM v1.4).
        if (hrLimits.size() > 10) {
            hrZones.erase(hrZones.begin());
        }
        stream << "\r\n[HRZones]\r\n";
        const QList<quint32> hrLimitsKeys = hrLimits.keys();
        for (int index = hrLimitsKeys.length() - 2; index >=0; --index) {
            stream << hrLimitsKeys.at(index) << "\r\n"; // Zone 1 to n upper limits.
        }
        if (!hrLimits.empty()) {
            #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            stream << hrLimits.first() << "\r\n"; // Zone n lower limit.
            #else
            stream << hrLimits.constBegin().value() << "\r\n";
            #endif
        }
        for (int index = hrLimits.size() + (hrLimits.isEmpty() ? 1 : 0); index < 11; ++index) {
            stream << "0\r\n"; // "0" entries for a total of 11 HRZones entries.
        }

        // [SwapTimes]
        stream << "[SwapTimes]\r\n";
        /// @todo Add phase swap times here if/when the training phases data becomes available.

        // [HRCCModeCh] "HR/CC mode swaps are a available only with Polar XTrainer Plus."

        // [IntTimes]
        QMap<QString, QVariantMap> laps;
        foreach (const QVariant &lap, autoLaps.value(QLatin1String("laps")).toList()) {
            QVariantMap lapMap = lap.toMap();
            const QString splitTime = hrmTime(firstMap(firstMap(
                lapMap.value(QLatin1String("header")))
                .value(QLatin1String("split-time"))));
            lapMap.insert(QLatin1String("_isAuto"), QVariant(true));
            laps.insert(splitTime, lapMap);
        }
        foreach (const QVariant &lap, manualLaps.value(QLatin1String("laps")).toList()) {
            QVariantMap lapMap = lap.toMap();
            const QString splitTime = hrmTime(firstMap(firstMap(
                lapMap.value(QLatin1String("header")))
                .value(QLatin1String("split-time"))));
            lapMap.insert(QLatin1String("_isAuto"), QVariant(false));
            laps.insert(splitTime, lapMap);
        }
        if (!laps.isEmpty()) {
            stream << "\r\n[IntTimes]\r\n";
            foreach (const QString &splitTime, laps.keys()) {
                const QVariantMap &lap = laps.value(splitTime);
                const QVariantMap header = firstMap(lap.value(QLatin1String("header")));
                const QVariantMap stats = firstMap(lap.value(QLatin1String("stats")));
                const QVariantMap hrStats = firstMap(stats.value(QLatin1String("heartrate")));
                // Row 1
                stream << hrmTime(firstMap(header.value(QLatin1String("split-time"))));
                stream << '\t' << first(hrStats.value(QLatin1String("average"))).toUInt();
                stream << '\t' << first(hrStats.value(QLatin1String("minimum"))).toUInt();
                stream << '\t' << first(hrStats.value(QLatin1String("average"))).toUInt();
                stream << '\t' << first(hrStats.value(QLatin1String("maximum"))).toUInt();
                stream << "\r\n";
                // Row 2
                stream << "28"; // All three "extra data" fields present (on row 3).
                stream << "\t0"; // Recovery time (seconds); data not available.
                stream << "\t0"; // Recovery HR (bpm); data not available.
                stream << "\t" << qRound(first(firstMap(stats.value(QLatin1String("speed")))
                    .value(QLatin1String("maximum"))).toFloat() * 128.0);
                stream << "\t" << first(firstMap(stats.value(QLatin1String("cadence")))
                    .value(QLatin1String("maximum"))).toUInt();
                stream << "\t0"; // Momentary altitude; not available per lap.
                stream << "\r\n";
                // Row 3: HRM allows up to three "extra data" fields. Here we
                // choose to leave out descent if power is available.
                if (havePower) {
                    stream << first(firstMap(header.value(QLatin1String("power")))
                        .value(QLatin1String("average"))).toUInt() * 10;
                } else {
                    stream << qRound(first(header.value(QLatin1String("descent"))).toFloat() * 10.0);
                }
                stream << '\t' << (first(firstMap(stats.value(QLatin1String("pedaling")))
                    .value(QLatin1String("average"))).toUInt() * 10);
                stream << '\t' << qRound(first(firstMap(stats.value(QLatin1String("incline")))
                    .value(QLatin1String("max"))).toFloat() * 10.0);
                stream << '\t' << qRound(first(header.value(QLatin1String("ascent"))).toFloat() / 10.0);
                stream << '\t' << qRound(first(header.value(QLatin1String("distance"))).toFloat() / 100.0);
                stream << "\r\n";
                // Row 4
                switch (first(header.value(QLatin1String("lap-type"))).toInt()) {
                case 1:  stream << 1; break; // Distance -> interval
                case 2:  stream << 1; break; // Duration -> interval
                case 3:  stream << 0; break; // Location -> normal lap
                default: stream << 0; // Absent (ie manual) -> normal lap
                }
                stream << '\t' << qRound(first(header.value(QLatin1String("distance"))).toFloat());
                stream << '\t' << first(header.value(QLatin1String("power"))).toUInt();
                stream << '\t' << first(firstMap(stats.value(QLatin1String("temperature")))
                    .value(QLatin1String("average"))).toFloat();
                stream << "\t0"; // "Internal phase/lap information"
                stream << "\t0"; // Air pressure not available in protobuf data.
                stream << "\r\n";
                // Row 5
                stream << first(firstMap(stats.value(QLatin1String("stride")))
                    .value(QLatin1String("average"))).toUInt();
                stream << '\t' << (lap.value(QLatin1String("_isAuto")).toBool() ? '1' : '0');
                stream << "\t0\t0\t0\t0\r\n";
            }
        }

        // [IntNotes]
        if (!laps.isEmpty()) {
            stream << "\r\n[IntNotes]\r\n";
            const QStringList keys = laps.keys();
            for (int index = 0; index < keys.length(); ++index) {
                const QVariantMap &lap = laps.value(keys.at(index));
                const QVariantMap header = firstMap(lap.value(QLatin1String("header")));
                switch (first(header.value(QLatin1String("lap-type"))).toInt()) {
                case 1:  stream << (index+1) << "\tDistance based lap\r\n"; break;
                case 2:  stream << (index+1) << "\tDuration based lap\r\n"; break;
                case 3:  stream << (index+1) << "\tLocation based lap\r\n"; break;
                default: stream << (index+1) << "\tManual lap\r\n";
                }
            }
        }

        // [ExtraData] This section describes the semantics of the "extra" data.
        // The data itself is included in the [IntTimes] section ("row 3"),
        // where HRM2 v1.4 says the acceptable range is 0..3000, and values are
        // all multiplied by 10, to extra data can hold 0..300 units.
        if (!laps.isEmpty()) {
            stream << "\r\n[ExtraData]\r\n";
            if (havePower) {
                stream << "Power\r\nW\t3000\t0\r\n";
            } else {
                stream << "Descent\r\nMeters\t1000\t0\r\n";
            }
            stream << "Pedaling Index\r\n%\t100\t0\r\n";
            stream << "Max Incline\r\nDegrees\t90\t0\r\n";
        }

        // [LapNames] This HRM section is undocumented, but supported by PPT5.
        if ((hrmOptions.testFlag(LapNames)) && (!laps.isEmpty())) {
            stream << "\r\n[LapNames]\r\n";
            const QStringList keys = laps.keys();
            for (int index = 0; index < keys.length(); ++index) {
                const QVariantMap &lap = laps.value(keys.at(index));
                stream << (index+1) << '\t'
                       << (lap.value(QLatin1String("_isAuto")).toBool() ? '2' : '1')
                       << "\r\n"; // 2 = Auto, 1 = Manual.
            }
        }

        // [Summary-123] This will need updating if/when phases data is available.
        const QVariantList heartrate = samples.value(QLatin1String("heartrate")).toList();
        int summary123Row1[5] = { 0, 0, 0, 0, 0};
        for (int index = 0; index < heartrate.length(); ++index) {
            const quint32 hr = ((index < heartrate.length()) ? heartrate.at(index).toUInt() : (uint)0);
            if (hr > hrMax)
                summary123Row1[0]++;
            else if (hr > phase1LimitHigh)
                summary123Row1[2]++;
            else if (hr > phase1LimitLow)
                summary123Row1[2]++;
            else if (hr > hrRest)
                summary123Row1[3]++;
            else
                summary123Row1[4]++;
        }
        stream << "\r\n[Summary-123]\r\n";
        stream << qRound(heartrate.length() * recordInterval / 1000.0);
        for (size_t index = 0; index < (sizeof(summary123Row1)/sizeof(summary123Row1[0])); ++index) {
            stream << '\t' << qRound(summary123Row1[index] * recordInterval / 1000.0);
        }
        stream << "\r\n";
        stream << "0\t0\t0\t0\t0\t0\r\n";
        stream << "0\t0\t0\t0\r\n";
        stream << "0\t0\t0\t0\t0\t0\r\n";
        stream << "0\t0\t0\t0\r\n";
        stream << "0\t" << heartrate.length() << "\r\n";

        // [Summary-TH]
        const quint32 anaerobicThreshold = first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("anaerobic-threshold"))).value(QLatin1String("value"))).toUInt();
        const quint32 aerobicThreshold = first(firstMap(parsedPhysicalInformation.value(
            QLatin1String("aerobic-threshold"))).value(QLatin1String("value"))).toUInt();
        int summaryThRow1[5] = { 0, 0, 0, 0, 0};
        for (int index = 0; index < heartrate.length(); ++index) {
            const quint32 hr = ((index < heartrate.length()) ? heartrate.at(index).toUInt() : (uint)0);
            if (hr > hrMax)
                summaryThRow1[0]++;
            else if (hr > anaerobicThreshold)
                summaryThRow1[1]++;
            else if (hr > aerobicThreshold)
                summaryThRow1[2]++;
            else if (hr > hrRest)
                summaryThRow1[3]++;
            else
                summaryThRow1[4]++;
        }
        stream << "\r\n[Summary-TH]\r\n"; // WebSync includes 0's when empty.
        stream << qRound(heartrate.length() * recordInterval / 1000.0);
        for (size_t index = 0; index < (sizeof(summaryThRow1)/sizeof(summaryThRow1[0])); ++index) {
            stream << '\t' << qRound(summaryThRow1[index] * recordInterval / 1000.0);
        }
        stream << "\r\n";
        stream << hrMax;
        stream << '\t' << anaerobicThreshold;
        stream << '\t' << aerobicThreshold;
        stream << '\t' << hrRest;
        stream << "\r\n";
        stream << "0\t" << heartrate.length() << "\r\n";

        // [Trip]
        stream << "\r\n[Trip]\r\n";
        stream << qRound(first(create.value(QLatin1String("distance"))).toFloat()/100.0) << "\r\n";
        stream << qRound(first(create.value(QLatin1String("ascent"))).toFloat()) << "\r\n";
        stream << qRound(getDuration(firstMap(create.value(QLatin1String("duration"))))/1000.0) << "\r\n";
        stream << qRound(first(firstMap(stats.value(QLatin1String("altitude"))).value(QLatin1String("average"))).toFloat()) << "\r\n";
        stream << qRound(first(firstMap(stats.value(QLatin1String("altitude"))).value(QLatin1String("maximum"))).toFloat()) << "\r\n";
        stream << qRound(first(firstMap(stats.value(QLatin1String("speed"))).value(QLatin1String("average"))).toFloat() * 128.0) << "\r\n";
        stream << qRound(first(firstMap(stats.value(QLatin1String("speed"))).value(QLatin1String("maximum"))).toFloat() * 128.0) << "\r\n";
        stream << "0\r\n"; // Odometer value at the end of an exercise.

        // [HRData]
        stream << "\r\n[HRData]\r\n";
        if (rrDataOnly) {
            foreach (const QVariant &sample, rrsamples) {
                stream << sample.toUInt() << "\r\n";
            }
        } else {
            const QVariantList altitude   = samples.value(QLatin1String("altitude")).toList();
            const QVariantList cadence    = samples.value(QLatin1String("cadence")).toList();
            const QVariantList speed      = samples.value(QLatin1String("speed")).toList();
            const QVariantList powerLeft  = samples.value(QLatin1String("left-pedal-power")).toList();
            const QVariantList powerRight = samples.value(QLatin1String("right-pedal-power")).toList();
            for (int index = 0; index < heartrate.length(); ++index) {
                stream << ((index < heartrate.length())
                    ? heartrate.at(index).toUInt() : (uint)0);
                if (haveSpeed) {
                    stream << '\t' << ((index < speed.length())
                        ? qRound(speed.at(index).toFloat() * 10.0) : (int)0);
                }
                if (haveCadence) {
                    stream << '\t' << ((index < cadence.length())
                        ? cadence.at(index).toUInt() : (uint)0);
                }
                if (haveAltitude) {
                    stream << '\t' << ((index < altitude.length())
                        ? qRound(altitude.at(index).toFloat()) : (int)0);
                }
                if (havePower) {
                    const int currentPowerLeft =
                            ((index < powerLeft.length()) &&
                             (!sensorOffline(samples.value(QLatin1String("left-pedal-power-offline")).toList(), index))) ?
                        first(powerLeft.at(index).toMap().value(QLatin1String("current-power"))).toInt() : 0;
                    const int currentPowerRight =
                            ((index < powerRight.length()) &&
                             (!sensorOffline(samples.value(QLatin1String("right-pedal-power-offline")).toList(), index))) ?
                        first(powerRight.at(index).toMap().value(QLatin1String("current-power"))).toInt() : 0;
                    if (currentPowerLeft < 0) {
                        qWarning() << "Negative left power sample at index" << index << ":" << currentPowerLeft;
                    }
                    if (currentPowerRight < 0) {
                        qWarning() << "Negative right power sample at index" << index << ":" << currentPowerRight;
                    }
                    const int currentPower = (havePowerLeft && havePowerRight)
                        ? qMax(currentPowerLeft, 0) + qMax(currentPowerRight, 0)
                        : qMax((havePowerLeft ? currentPowerLeft : currentPowerRight) * 2, 0);
                    Q_ASSERT(currentPower >= 0); // None of the above branches can yield a negative power.
                    Q_ASSERT(currentPower >= qMax(currentPowerLeft, 0));
                    Q_ASSERT(currentPower >= qMax(currentPowerRight, 0));
                    stream << '\t' << currentPower;
                    if (havePowerBalance) {
                        // In case we only have right power, not left.
                        const int powerLeft = havePowerLeft ? qMax(currentPowerLeft, 0) : currentPower - qMax(currentPowerRight, 0);
                        Q_ASSERT(powerLeft >= 0); // Guaranteed by the previous statement, in conjunction with the Q_ASSERTs above.
                        // Convert the left and right powers into a left-right balance percentage.
                        const int leftBalance = (currentPower == 0)
                            ? (powerLeft == qMax(currentPowerRight, 0)) ? 50 : (powerLeft < currentPowerRight) ? 0 : 100
                            : qRound(100.0 * (float)powerLeft / (float)currentPower);
                        if ((0 > leftBalance) || (leftBalance > 100)) {
                            qWarning() << "leftBalance of" << leftBalance << "is outside the range 0..100%";
                            // We enforce the range [0..100] below.
                        }
                        /// @todo Include Pedalling Index here, if/when available.
                        stream << '\t' << qMax(qMin(leftBalance, 100), 0);
                    }
                }
                // Air pressure - not available in protobuf data.
                stream << "\r\n";
            }
        }

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
    if (tcxOptions.testFlag(GarminActivityExtension)) {
        tcx.setAttribute(QLatin1String("xmlns:ax2"),
                         QLatin1String("http://www.garmin.com/xmlschemas/ActivityExtension/v2"));
    }
    if (tcxOptions.testFlag(GarminCourseExtension)) {
        tcx.setAttribute(QLatin1String("xmlns:cx1"),
                         QLatin1String("http://www.garmin.com/xmlschemas/CourseExtension/v1"));
    }
    doc.appendChild(tcx);

    QDomElement activities = doc.createElement(QLatin1String("Activities"));

    QDomElement multiSportSession;
    if ((parsedExercises.size() > 1) && (!parsedSession.isEmpty())) {
        multiSportSession = doc.createElement(QLatin1String("MultiSportSession"));
        QDateTime id = getDateTime(firstMap(parsedSession.value(QLatin1String("start"))));
        if (tcxOptions.testFlag(ForceTcxUTC)) {
            id = id.toUTC();
        }
        multiSportSession.appendChild(doc.createElement(QLatin1String("Id")))
            .appendChild(doc.createTextNode(id.toString(Qt::ISODate)));
        activities.appendChild(multiSportSession);
    }

    foreach (const QVariant &exercise, parsedExercises) {
        const QVariantMap map = exercise.toMap();
        if (!map.contains(CREATE)) {
            qWarning() << "Skipping exercise with no 'create' request data";
            continue;
        }
        const QVariantMap create  = map.value(CREATE).toMap();
        const QVariantMap route   = map.value(ROUTE).toMap();
        const QVariantMap samples = map.value(SAMPLES).toMap();
        const quint64 recordInterval = getDuration(
            firstMap(samples.value(QLatin1String("record-interval"))));

        // Get the "samples" samples.
        const QVariantList altitude    = samples.value(QLatin1String("altitude")).toList();
        const QVariantList cadence     = samples.value(QLatin1String("cadence")).toList();
        const QVariantList powerLeft   = samples.value(QLatin1String("left-pedal-power")).toList();
        const QVariantList powerRight  = samples.value(QLatin1String("right-pedal-power")).toList();
        const QVariantList distance    = samples.value(QLatin1String("distance")).toList();
        const QVariantList heartrate   = samples.value(QLatin1String("heartrate")).toList();
        const QVariantList speed       = samples.value(QLatin1String("speed")).toList();
        const QVariantList temperature = samples.value(QLatin1String("temperature")).toList();

        // Get the "route" samples.
        const QVariantList duration    = route.value(QLatin1String("duration")).toList();
        const QVariantList gpsAltitude = route.value(QLatin1String("altitude")).toList();
        const QVariantList latitude    = route.value(QLatin1String("latitude")).toList();
        const QVariantList longitude   = route.value(QLatin1String("longitude")).toList();
        const QVariantList satellites  = route.value(QLatin1String("satellites")).toList();

        const int maxIndex =
            qMax(altitude.length(),
            qMax(cadence.length(),
            qMax(distance.length(),
            qMax(heartrate.length(),
            qMax(speed.length(),
          //qMax(temperature.length(), // We don't use temperature in TCX yet.
            qMax(duration.length(),
            qMax(gpsAltitude.length(),
            qMax(latitude.length(),
            qMax(longitude.length(),
            qMax(satellites.length(), 0))))))))));

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

        // Get the sport type.
        activity.setAttribute(QLatin1String("Sport"),
            getTcxSport(first(firstMap(create.value(QLatin1String("sport")))
                .value(QLatin1String("value"))).toULongLong()));

        // Get the starting time.
        QDateTime startTime = getDateTime(firstMap(create.value(QLatin1String("start"))));
        if (tcxOptions.testFlag(ForceTcxUTC)) {
            startTime = startTime.toUTC();
        }
        activity.appendChild(doc.createElement(QLatin1String("Id")))
            .appendChild(doc.createTextNode(startTime.toString(Qt::ISODate)));

        // Build a map of lap split times to lap data.
        QVariantList laps = map.value(LAPS).toMap().value(QLatin1String("laps")).toList();
        if (laps.isEmpty()) {
            laps = map.value(AUTOLAPS).toMap().value(QLatin1String("laps")).toList();
        }
        QMap<quint64, QVariantMap> splits;
        foreach (const QVariant &lap, laps) {
            const QVariantMap lapData = lap.toMap();
            const quint64 splitTime = getDuration(firstMap(firstMap(
                lapData.value(QLatin1String("header")))
                .value(QLatin1String("split-time"))));
            if (splitTime > 0) {
                splits.insert(splitTime, lapData);
            }
        }

        // Add each of the laps to the Activity element.
        QDomElement lap;
        QVariantMap base = create; // The base data for this lap.
        QVariantMap stats = map.value(STATISTICS).toMap();
        QDomElement track = doc.createElement(QLatin1String("Track"));
        quint64 durationRemaining = getDuration(firstMap(create.value(QLatin1String("duration"))));
        double distanceRemaining = first(create.value(QLatin1String("distance"))).toDouble();
        for (int index = 0; index < maxIndex; ++index) {
            #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
            if ((lap.isNull()) || ((!splits.isEmpty()) && (index * recordInterval > splits.firstKey()))) {
            #else
            if ((lap.isNull()) || ((!splits.isEmpty()) && (index * recordInterval > splits.constBegin().key()))) {
            #endif
                quint64 trailingDuration = 0;
                double trailingDistance = 0.0;
                if ((!lap.isNull()) && (!splits.isEmpty())) {
                    #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                    splits.remove(splits.firstKey());
                    #else
                    splits.remove(splits.constBegin().key());
                    #endif
                }
                if (!splits.isEmpty()) {
                    #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                    const QVariantMap lapData = splits.first();
                    #else
                    const QVariantMap lapData = splits.constBegin().value();
                    #endif
                    base = firstMap(lapData.value(QLatin1String("header")));
                    stats = firstMap(lapData.value(QLatin1String("stats")));
                    durationRemaining -= getDuration(firstMap(base.value(QLatin1String("duration"))));
                    distanceRemaining -= first(base.value(QLatin1String("distance"))).toDouble();
                } else if (index != 0) {
                    base = QVariantMap();
                    trailingDuration = durationRemaining;
                    trailingDistance = distanceRemaining;
                    stats = QVariantMap();
                }

                // Create the Lap element, and set its StartTime attribute.
                #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                QDateTime lapStartTime = startTime.addMSecs(index * recordInterval);
                #else /// @todo Remove this hack when Qt 5.2+ is available on Travis CI.
                QDateTime lapStartTime = startTime.toUTC()
                    .addMSecs(index * recordInterval).addSecs(startTime.utcOffset());
                lapStartTime.setUtcOffset(startTime.utcOffset());
                #endif
                if (tcxOptions.testFlag(ForceTcxUTC)) {
                    lapStartTime = lapStartTime.toUTC();
                }
                lap = doc.createElement(QLatin1String("Lap"));
                lap.setAttribute(QLatin1String("StartTime"),
                    lapStartTime.toString(Qt::ISODate));
                activity.appendChild(lap);

                // Add the per-lap (or per-exercise) statistics.
                addLapStats(doc, lap, base, stats, trailingDuration, trailingDistance);

                track = doc.createElement(QLatin1String("Track"));
                lap.appendChild(track);

                // Add any enabled extensions.
                if (tcxOptions.testFlag(GarminActivityExtension) ||
                    tcxOptions.testFlag(GarminCourseExtension))
                {
                    QDomElement extensions = doc.createElement(QLatin1String("Extensions"));
                    lap.appendChild(extensions);

                    // Add the Garmin Activity Extension.
                    if (tcxOptions.testFlag(GarminActivityExtension)) {
                        QDomElement lx = doc.createElement(QLatin1String("LX"));
                        lx.setAttribute(QLatin1String("xmlns"),
                            QLatin1String("http://www.garmin.com/xmlschemas/ActivityExtension/v2"));
                        extensions.appendChild(lx);

                        if (stats.contains(QLatin1String("speed"))) {
                            lx.appendChild(doc.createElement(QLatin1String("AvgSpeed")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(first(firstMap(stats.value(QLatin1String("speed")))
                                        .value(QLatin1String("average"))).toDouble() / 3.6)));
                        }

                        if (stats.contains(QLatin1String("cadence"))) {
                            const QVariantMap cadence = firstMap(stats.value(QLatin1String("cadence")));

                            const QString sensor = getTcxCadenceSensor(
                                first(firstMap(create.value(QLatin1String("sport")))
                                                .value(QLatin1String("value"))).toULongLong());

                            if (sensor != QLatin1String("Footpod")) {
                                lx.appendChild(doc.createElement(QLatin1String("MaxBikeCadence")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                        .arg(first(cadence.value(QLatin1String("maximum"))).toUInt())));
                            }

                            lx.appendChild(doc.createElement(QLatin1String("AvgRunCadence")))
                                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                    .arg(first(cadence.value(QLatin1String("average"))).toUInt())));

                            if (sensor == QLatin1String("Footpod")) {
                                lx.appendChild(doc.createElement(QLatin1String("MaxRunCadence")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                        .arg(first(cadence.value(QLatin1String("maximum"))).toUInt())));
                            }

                            /// @todo Steps

                            // Note, AvgWatts is defined by both the Garmin Activity
                            // Extension and the Garmin Course Extension schemas.
                            const QVariantMap power = firstMap(base.value(QLatin1String("power")));
                            if (power.contains(QLatin1String("average"))) {
                                lx.appendChild(doc.createElement(QLatin1String("AvgWatts")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                        .arg(first(power.value(QLatin1String("average"))).toUInt())));
                            }
                            if (power.contains(QLatin1String("maximum"))) {
                                lx.appendChild(doc.createElement(QLatin1String("MaxWatts")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                        .arg(first(power.value(QLatin1String("maximum"))).toUInt())));
                            }
                        }

                        if (tcxOptions.testFlag(GarminCourseExtension)) {
                            QDomElement cx = doc.createElement(QLatin1String("CX"));
                            cx.setAttribute(QLatin1String("xmlns"),
                                QLatin1String("http://www.garmin.com/xmlschemas/CourseExtension/v1"));
                            extensions.appendChild(cx);

                            // Note, AvgWatts is defined by both the Garmin Activity
                            // Extension and the Garmin Course Extension schemas.
                            const QVariantMap power = firstMap(base.value(QLatin1String("power")));
                            if (power.contains(QLatin1String("average"))) {
                                cx.appendChild(doc.createElement(QLatin1String("AvgWatts")))
                                    .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                                        .arg(first(power.value(QLatin1String("average"))).toUInt())));
                            }
                        }
                    }
                }
            }

            QDomElement trackPoint = doc.createElement(QLatin1String("Trackpoint"));

            if ((index < latitude.length()) && (index < longitude.length())) {
                QDomElement position = doc.createElement(QLatin1String("Position"));
                position.appendChild(doc.createElement(QLatin1String("LatitudeDegrees")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(latitude.at(index))));
                position.appendChild(doc.createElement(QLatin1String("LongitudeDegrees")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(longitude.at(index))));
                trackPoint.appendChild(position);
            }

            if ((index < altitude.length()) &&
                (!sensorOffline(samples.value(QLatin1String("altitude-offline")).toList(), index))) {
                trackPoint.appendChild(doc.createElement(QLatin1String("AltitudeMeters")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(altitude.at(index))));
            }
            if ((index < distance.length()) &&
                (!sensorOffline(samples.value(QLatin1String("distance-offline")).toList(), index))) {
                trackPoint.appendChild(doc.createElement(QLatin1String("DistanceMeters")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(distance.at(index))));
            }
            if ((index < heartrate.length()) && (heartrate.at(index).toInt() > 0) &&
                (!sensorOffline(samples.value(QLatin1String("heartrate-offline")).toList(), index))) {
                trackPoint.appendChild(doc.createElement(QLatin1String("HeartRateBpm")))
                    .appendChild(doc.createElement(QLatin1String("Value")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(heartrate.at(index))));
            }
            if ((index < cadence.length()) && (cadence.at(index).toInt() >= 0) &&
                (!sensorOffline(samples.value(QLatin1String("cadence-offline")).toList(), index))) {
                trackPoint.appendChild(doc.createElement(QLatin1String("Cadence")))
                    .appendChild(doc.createTextNode(VARIANT_TO_STRING(cadence.at(index))));
            }

            if (tcxOptions.testFlag(GarminActivityExtension)) {
                QDomElement tpx = doc.createElement(QLatin1String("TPX"));
                tpx.setAttribute(QLatin1String("xmlns"),
                    QLatin1String("http://www.garmin.com/xmlschemas/ActivityExtension/v2"));
                trackPoint.appendChild(doc.createElement(QLatin1String("Extensions")))
                    .appendChild(tpx);

                if ((index < speed.length()) && (speed.at(index).toInt() >= 0) &&
                    (!sensorOffline(samples.value(QLatin1String("speed-offline")).toList(), index))) {
                    tpx.appendChild(doc.createElement(QLatin1String("Speed")))
                        .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                            .arg(speed.at(index).toDouble() / 3.6)));
                }

                if ((index < cadence.length()) && (cadence.at(index).toInt() >= 0) &&
                    (!sensorOffline(samples.value(QLatin1String("cadence-offline")).toList(), index))) {
                    const QString sensor = getTcxCadenceSensor(
                        first(firstMap(create.value(QLatin1String("sport")))
                                        .value(QLatin1String("value"))).toULongLong());
                    if (!sensor.isEmpty()) {
                        tpx.setAttribute(QLatin1String("CadenceSensor"), sensor);
                    }
                    if (sensor == QLatin1String("Footpod")) {
                        tpx.appendChild(doc.createElement(QLatin1String("RunCadence")))
                            .appendChild(doc.createTextNode(VARIANT_TO_STRING(cadence.at(index))));
                    }
                }

                const QVariant currentPowerLeft = (index < powerLeft.length()) ?
                    first(powerLeft.at(index).toMap().value(QLatin1String("current-power"))) : QVariant();
                const QVariant currentPowerRight = (index < powerRight.length()) ?
                    first(powerRight.at(index).toMap().value(QLatin1String("current-power"))) : QVariant();
                if ((currentPowerLeft.isValid()) && (currentPowerLeft.toInt() < 0)) {
                    qWarning() << "Negative left power sample at index" << index << ":" << currentPowerLeft.toInt();
                }
                if ((currentPowerRight.isValid()) && (currentPowerRight.toInt() < 0)) {
                    qWarning() << "Negative right power sample at index" << index << ":" << currentPowerRight.toInt();
                }

                const QVariant currentPower =
                    (currentPowerLeft.isValid() && currentPowerRight.isValid())
                        ? qMax(currentPowerLeft.toInt(), 0) + qMax(currentPowerRight.toInt(), 0)
                        : currentPowerLeft.isValid() ? qMax(currentPowerLeft.toInt() * 2, 0)
                        : currentPowerRight.isValid() ? qMax(currentPowerRight.toInt() * 2, 0)
                        : QVariant();
                Q_ASSERT(currentPower.toInt() >= 0);

                if (currentPower.isValid()) {
                    tpx.appendChild(doc.createElement(QLatin1String("Watts")))
                        .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                            .arg(qMax(currentPower.toInt(), 0))));
                }

            }

            if (trackPoint.hasChildNodes()) {
                #if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                QDateTime trackPointTime = startTime.addMSecs(index * recordInterval);
                #else /// @todo Remove this hack when Qt 5.2+ is available on Travis CI.
                QDateTime trackPointTime = startTime.toUTC()
                    .addMSecs(index * recordInterval).addSecs(startTime.utcOffset());
                trackPointTime.setUtcOffset(startTime.utcOffset());
                #endif
                if (tcxOptions.testFlag(ForceTcxUTC)) {
                    trackPointTime = trackPointTime.toUTC();
                }
                trackPoint.insertBefore(doc.createElement(QLatin1String("Time")), QDomNode())
                    .appendChild(doc.createTextNode(trackPointTime.toString(Qt::ISODate)));
                track.appendChild(trackPoint);
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
            while (versionParts.length() < 4) {
                versionParts.append(QLatin1String("0"));
            }
            version.appendChild(doc.createElement(QLatin1String("VersionMajor")))
                .appendChild(doc.createTextNode(versionParts.at(0)));
            version.appendChild(doc.createElement(QLatin1String("VersionMinor")))
                .appendChild(doc.createTextNode(versionParts.at(1)));
            version.appendChild(doc.createElement(QLatin1String("BuildMajor")))
                .appendChild(doc.createTextNode(versionParts.at(2)));
            version.appendChild(doc.createElement(QLatin1String("BuildMinor")))
                .appendChild(doc.createTextNode(versionParts.at(3)));
            QString buildType = QLatin1String("Release");
            VersionInfo versionInfo;
            const QString specialBuild = versionInfo.fileInfo(QLatin1String("SpecialBuild"));
            if (!specialBuild.isEmpty()) {
                buildType = specialBuild;
            }
            build.appendChild(doc.createElement(QLatin1String("Type")))
                .appendChild(doc.createTextNode(buildType));
            build.appendChild(doc.createElement(QLatin1String("Time")))
                .appendChild(doc.createTextNode(
                    buildTime.isEmpty() ? QString::fromLatin1(__DATE__ " " __TIME__) : buildTime));
            #ifdef BUILD_USER
            #define BIPOLAR_STRINGIFY(string) #string
            #define BIPOLAR_EXPAND_AND_STRINGIFY(macro) BIPOLAR_STRINGIFY(macro)
            build.appendChild(doc.createElement(QLatin1String("Builder")))
                .appendChild(doc.createTextNode(QLatin1String(
                    BIPOLAR_EXPAND_AND_STRINGIFY(BUILD_USER))));
            #undef BIPOLAR_EXPAND_AND_STRINGIFY
            #undef BIPOLAR_STRINGIFY
            #endif
        }

        /// @todo  Make this dynamic if/when app is localized.
        author.appendChild(doc.createElement(QLatin1String("LangID")))
            .appendChild(doc.createTextNode(QLatin1String("EN")));
        author.appendChild(doc.createElement(QLatin1String("PartNumber")))
            .appendChild(doc.createTextNode(QLatin1String("434-F4C42-59")));
    }
    return doc;
}

void TrainingSession::addLapStats(QDomDocument &doc, QDomElement &lap,
                                  const QVariantMap &base,
                                  const QVariantMap &stats,
                                  const quint64 duration,
                                  const double distance) const
{
    // Note, we're using an explicit precision argument to QString::arg here
    // because QString::arg defaults to the precision to -1, which in turn
    // (within QLocale::doubleToString) defaults to a maximum of 6 significant
    // digits (not decimal digits, as the QString::arg docs claim), which is not
    // always enough for the TotalTimeSeconds value to be accurate to the
    // millisecond data that the V800 provides.  We use 20 digits here, since
    // that's the maximum that could ever be present in a quint64 integer,
    // however that's likely to be massive overkill for our use case (but does
    // no harm, since only the necessary digits are printed anyway).
    lap.appendChild(doc.createElement(QLatin1String("TotalTimeSeconds")))
        .appendChild(doc.createTextNode(QString::fromLatin1("%1").arg(
            qMax(duration, getDuration(firstMap(base.value(QLatin1String("duration")))))/1000.0,
            0, 'g', 20))); // Since quint64 can have supply more than 20 digits.
    lap.appendChild(doc.createElement(QLatin1String("DistanceMeters")))
        .appendChild(doc.createTextNode(QString::fromLatin1("%1").arg(qMax(
            distance, first(base.value(QLatin1String("distance"))).toDouble()))));
    if (stats.contains(QLatin1String("speed"))) {
        lap.appendChild(doc.createElement(QLatin1String("MaximumSpeed")))
            .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                .arg(first(firstMap(stats.value(QLatin1String("speed")))
                    .value(QLatin1String("maximum"))).toDouble() / 3.6)));
    }

    // Calories is only available per exercise, not per lap, but it is required
    // by the TCX schema, so the following will set it to 0, if not present.
    lap.appendChild(doc.createElement(QLatin1String("Calories")))
        .appendChild(doc.createTextNode(QString::fromLatin1("%1")
            .arg(first(base.value(QLatin1String("calories"))).toUInt())));

    const QVariantMap hrStats = firstMap(stats.value(QLatin1String("heartrate")));
    if (!hrStats.isEmpty()) {
        lap.appendChild(doc.createElement(QLatin1String("AverageHeartRateBpm")))
            .appendChild(doc.createElement(QLatin1String("Value")))
                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                    .arg(first(hrStats.value(QLatin1String("average"))).toUInt())));
        lap.appendChild(doc.createElement(QLatin1String("MaximumHeartRateBpm")))
            .appendChild(doc.createElement(QLatin1String("Value")))
                .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                    .arg(first(hrStats.value(QLatin1String("maximum"))).toUInt())));
    }
    /// @todo Intensity must be one of: Active, Resting.
    lap.appendChild(doc.createElement(QLatin1String("Intensity")))
        .appendChild(doc.createTextNode(QString::fromLatin1("Active")));

    if (stats.contains(QLatin1String("cadence"))) {
        lap.appendChild(doc.createElement(QLatin1String("Cadence")))
            .appendChild(doc.createTextNode(QString::fromLatin1("%1")
                .arg(first(firstMap(stats.value(QLatin1String("cadence")))
                    .value(QLatin1String("average"))).toUInt())));
    }

    // TriggerMethod must be one of: Manual, Distance, Location, Time, HeartRate.
    QString triggerMethod;
    switch (first(base.value(QLatin1String("lap-type"))).toInt()) {
    case 1:  triggerMethod = QLatin1String("Distance"); break; // DISTANCE -> Distance
    case 2:  triggerMethod = QLatin1String("Time");     break; // DURATION -> Time
    case 3:  triggerMethod = QLatin1String("Location"); break; // LOCATION -> Location
    default: triggerMethod = QLatin1String("Manual");
    }
    lap.appendChild(doc.createElement(QLatin1String("TriggerMethod")))
        .appendChild(doc.createTextNode(triggerMethod));
}

QByteArray TrainingSession::unzip(const QByteArray &data,
                                  const int initialBufferSize) const
{
    Q_ASSERT(initialBufferSize > 0);
    QByteArray result;
    result.resize(initialBufferSize);

    // Prepare a zlib stream structure.
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
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

QString TrainingSession::writeGPX(const QString &fileNameFormat,
                                  QString outputDirName)
{
    if (outputDirName.isEmpty()) {
        outputDirName = QFileInfo(baseName).dir().absolutePath();
    }
    const QString fileName = QString::fromLatin1("%1/%2.gpx")
        .arg(outputDirName).arg(getOutputBaseFileName(fileNameFormat));
    writeGPX(fileName);
    return fileName;
}

bool TrainingSession::writeGPX(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qWarning() << "Failed to open" << QDir::toNativeSeparators(fileName);
        return false;
    }
    return writeGPX(file);
}

bool TrainingSession::writeGPX(QIODevice &device) const
{
    QDomDocument gpx = toGPX();
    if (gpx.isNull()) {
        qWarning() << "Failed to convert to GPX" << baseName;
        return false;
    }
    device.write(gpx.toByteArray());
    return true;
}

QStringList TrainingSession::writeHRM(const QString &fileNameFormat,
                                      QString outputDirName)
{
    if (outputDirName.isEmpty()) {
        outputDirName = QFileInfo(baseName).dir().absolutePath();
    }
    return writeHRM(QString::fromLatin1("%1/%2")
        .arg(outputDirName).arg(getOutputBaseFileName(fileNameFormat)));
}

QStringList TrainingSession::writeHRM(const QString &baseName) const
{
    QStringList fileNames;
    for (int rrDataOnly = 0; rrDataOnly <= (hrmOptions.testFlag(RrFiles) ? 1 : 0); ++rrDataOnly) {
        QStringList hrm = toHRM(rrDataOnly);
        if (hrm.isEmpty()) {
            qWarning() << "Failed to convert to HRM" << baseName;
            return QStringList();
        }

        for (int index = 0; index < hrm.length(); ++index) {
            const QString extension = QLatin1String((rrDataOnly) ? "rr.hrm" : "hrm");
            const QString fileName = (hrm.length() == 1)
                ? QString::fromLatin1("%1.%2").arg(baseName).arg(extension)
                : QString::fromLatin1("%1.%2.%3").arg(baseName).arg(index).arg(extension);
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
                qWarning() << "Failed to open" << QDir::toNativeSeparators(fileName);
            } else if (file.write(hrm.at(index).toLatin1())) {
                fileNames.append(fileName);
            }
        }
    }
    return fileNames;
}

QString TrainingSession::writeTCX(const QString &fileNameFormat,
                                  QString outputDirName)
{
    if (outputDirName.isEmpty()) {
        outputDirName = QFileInfo(baseName).dir().absolutePath();
    }
    const QString fileName = QString::fromLatin1("%1/%2.tcx")
        .arg(outputDirName).arg(getOutputBaseFileName(fileNameFormat));
    writeTCX(fileName);
    return fileName;
}

bool TrainingSession::writeTCX(const QString &fileName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qWarning() << "Failed to open" << QDir::toNativeSeparators(fileName);
        return false;
    }
    return writeTCX(file);
}

bool TrainingSession::writeTCX(QIODevice &device) const
{
    QDomDocument tcx = toTCX();
    if (tcx.isNull()) {
        qWarning() << "Failed to convert to TCX" << baseName;
        return false;
    }
    device.write(tcx.toByteArray());
    return true;
}

}}
