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
#include <QStringList>

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

    bool parse(const QStringList &laps, const QStringList &routes,
               const QStringList &samples, const QStringList &zones);

private:

signals:
    void parseError(const QString &message, const QString &fileName = QString());

};

}}

#endif // __POLAR_V2_TRAINING_SESSION_H__
