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

#ifndef __POLAR_V2_TRAINING_SESSION_H__
#define __POLAR_V2_TRAINING_SESSION_H__

#include <QIODevice>
#include <QMap>
#include <QStringList>
#include <QVariant>

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

public:
    TrainingSession(const QString &baseName = QString());

    bool isValid() const;

    bool parse(const QString &baseName = QString());

    bool writeGPX(const QString &fileName = QString());
    bool writeGPX(const QIODevice &device);
    bool writeGPX(const bool separateFiles);

    /**
     * @brief writeTCX
     * @param fileName
     * @param sport     Optional TCX sport for each acitivity. Each entry should
     *                  be one of "Running", "Biking" or "Other". If not set,
     *                  the function will attempt to detect the value based on
     *                  information such as the presence of bicycle cadence data.
     * @return
     */
    bool writeTCX(const QStringList &sport = QStringList());
    bool writeTCX(const QString &fileName, const QStringList &sport = QStringList());
    bool writeTCX(const QIODevice &device, const QStringList &sport = QStringList());
    bool writeTCX(const bool separateFiles, const QStringList &sport = QStringList());

protected:
    QString baseName;
    QVariantMap parsedExercises;

    bool parse(const QString &exerciseId, const QMap<QString, QString> &fileNames);
    QVariantMap parseLaps(QIODevice &data) const;
    QVariantMap parseLaps(const QString &fileName) const;
    QVariantMap parseRoute(QIODevice &data) const;
    QVariantMap parseRoute(const QString &fileName) const;
    QVariantMap parseSamples(QIODevice &data) const;
    QVariantMap parseSamples(const QString &fileName) const;
    QVariantMap parseZones(QIODevice &data) const;
    QVariantMap parseZones(const QString &fileName) const;

private:
    friend class TestTrainingSession;

signals:
    void parseError(const QString &message,
                    const QString &fileName = QString()) const;

};

}}

#endif // __POLAR_V2_TRAINING_SESSION_H__
