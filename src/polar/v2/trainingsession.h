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

#ifndef __POLAR_V2_TRAINING_SESSION_H__
#define __POLAR_V2_TRAINING_SESSION_H__

#include <QDateTime>
#include <QDomDocument>
#include <QIODevice>
#include <QMap>
#include <QStringList>
#include <QVariant>

class TestTrainingSession;

namespace polar {
namespace v2 {

/**
 * @brief The TrainingSession class
 *
 * @note This class does not yet make any use of *-phases, *-rrsamples,
 *       *-sensors, nor *-statistics file, if present.
 */
class TrainingSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(int exerciseCount READ exerciseCount)

public:
    enum OutputFormat {
        GpxOutput = 0x0001,
        HrmOutput = 0x0002,
        TcxOutput = 0x0004,
        AllOutputs = GpxOutput|HrmOutput|TcxOutput
    };
    Q_DECLARE_FLAGS(OutputFormats, OutputFormat)

    enum GpxOption {
        CluetrustGpxDataExtension   = 0x0100,
        GarminAccelerationExtension = 0x0200,
        GarminTrackPointExtension   = 0x0400,
    };
    Q_DECLARE_FLAGS(GpxOptions, GpxOption)

    enum HrmOption {
        RrFiles  = 0x0001,
        LapNames = 0x0002,
    };
    Q_DECLARE_FLAGS(HrmOptions, HrmOption)

    enum TcxOption {
        ForceTcxUTC = 0x0001,
        GarminActivityExtension = 0x0100,
        GarminCourseExtension   = 0x0200,
    };
    Q_DECLARE_FLAGS(TcxOptions, TcxOption)

    TrainingSession(const QString &baseName);

    int exerciseCount() const;

    QStringList getOutputFileNames(const QString &fileNameFormat,
                                   const OutputFormats outputFormats,
                                   QString outputDirName = QString());

    bool isValid() const;

    bool parse();

    void setGpxOption(const GpxOption option, const bool enabled = true);
    void setHrmOption(const HrmOption option, const bool enabled = true);
    void setTcxOption(const TcxOption option, const bool enabled = true);
    void setGpxOptions(const GpxOptions options);
    void setHrmOptions(const HrmOptions options);
    void setTcxOptions(const TcxOptions options);

    QString writeGPX(const QString &fileNameFormat, QString outputDirName);
    bool writeGPX(const QString &fileName) const;
    bool writeGPX(QIODevice &device) const;

    QStringList writeHRM(const QString &fileNameFormat, QString outputDirName);
    QStringList writeHRM(const QString &baseName) const;

    QString writeTCX(const QString &fileNameFormat, QString outputDirName);
    bool writeTCX(const QString &fileName) const;
    bool writeTCX(QIODevice &device) const;

protected:
    QString baseName;
    QVariantMap parsedExercises;
    QVariantMap parsedPhysicalInformation;
    QVariantMap parsedSession;

    GpxOptions gpxOptions;
    HrmOptions hrmOptions;
    TcxOptions tcxOptions;

    static QString getPolarSportName(const quint64 &polarSportValue);
    static QString getTcxCadenceSensor(const quint64 &polarSportValue);
    static QString getTcxSport(const quint64 &polarSportValue);
    QString getOutputBaseFileName(const QString &format);

    static bool isGzipped(const QByteArray &data);
    static bool isGzipped(QIODevice &data);

    bool parse(const QString &exerciseId, const QMap<QString, QString> &fileNames);
    QVariantMap parseCreateExercise(QIODevice &data) const;
    QVariantMap parseCreateExercise(const QString &fileName) const;
    QVariantMap parseCreateSession(QIODevice &data) const;
    QVariantMap parseCreateSession(const QString &fileName) const;
    QVariantMap parseLaps(QIODevice &data) const;
    QVariantMap parseLaps(const QString &fileName) const;
    QVariantMap parsePhysicalInformation(QIODevice &data) const;
    QVariantMap parsePhysicalInformation(const QString &fileName) const;
    QVariantMap parseRoute(QIODevice &data) const;
    QVariantMap parseRoute(const QString &fileName) const;
    QVariantMap parseRRSamples(QIODevice &data) const;
    QVariantMap parseRRSamples(const QString &fileName) const;
    QVariantMap parseSamples(QIODevice &data) const;
    QVariantMap parseSamples(const QString &fileName) const;
    QVariantMap parseStatistics(QIODevice &data) const;
    QVariantMap parseStatistics(const QString &fileName) const;
    QVariantMap parseZones(QIODevice &data) const;
    QVariantMap parseZones(const QString &fileName) const;

    QDomDocument toGPX(const QDateTime &creationTime = QDateTime::currentDateTimeUtc()) const;

    QStringList toHRM(const bool rrDataOnly = false) const;

    QDomDocument toTCX(const QString &buildTime = QString()) const;

    QByteArray unzip(const QByteArray &data,
                     const int initialBufferSize = 10240) const;

private:
    friend class ::TestTrainingSession;

    void addLapStats(QDomDocument &doc, QDomElement &lap,
                     const QVariantMap &base, const QVariantMap &stats,
                     const quint64 duration = 0, const double distance = 0) const;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(TrainingSession::OutputFormats);
Q_DECLARE_OPERATORS_FOR_FLAGS(TrainingSession::GpxOptions);
Q_DECLARE_OPERATORS_FOR_FLAGS(TrainingSession::HrmOptions);
Q_DECLARE_OPERATORS_FOR_FLAGS(TrainingSession::TcxOptions);

}}

#endif // __POLAR_V2_TRAINING_SESSION_H__
