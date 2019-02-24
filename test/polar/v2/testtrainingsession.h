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

#include <QMap>
#include <QObject>

namespace polar { namespace v2 { class TrainingSession; }}

class TestTrainingSession : public QObject {
    Q_OBJECT

private:
    QString outputDirPath;
    QMap<QString, polar::v2::TrainingSession *> trainingSessions;

    polar::v2::TrainingSession * getTrainingSession(const QString &baseName);

private slots:
    void initTestCase();
    void cleanupTestCase();

    void getOutputBaseFileName_data();
    void getOutputBaseFileName();

    void getOutputFileNames_data();
    void getOutputFileNames();

    void isGzipped_data();
    void isGzipped();

    void parseCreateExercise_data();
    void parseCreateExercise();

    void parseCreateSession_data();
    void parseCreateSession();

    void parseLaps_data();
    void parseLaps();

    void parsePhysicalInformation_data();
    void parsePhysicalInformation();

    void parseRoute_data();
    void parseRoute();

    void parseRRSamples_data();
    void parseRRSamples();

    void parseSamples_data();
    void parseSamples();

    void parseStatistics_data();
    void parseStatistics();

    void parseZones_data();
    void parseZones();

    void toGPX_data();
    void toGPX();

    void toGPX_AllExtensions_data();
    void toGPX_AllExtensions();

    void toGPX_Cluetrust_data();
    void toGPX_Cluetrust();

    void toGPX_GarminAcceleration_data();
    void toGPX_GarminAcceleration();

    void toGPX_GarminTrackPoint_data();
    void toGPX_GarminTrackPoint();

    void toHRM_data();
    void toHRM();

    void toHRM_LapNames_data();
    void toHRM_LapNames();

    void toHRM_LapNames_RR_data();
    void toHRM_LapNames_RR();

    void toHRM_RR_data();
    void toHRM_RR();

    void toTCX_data();
    void toTCX();

    void toTCX_AllExtensions_data();
    void toTCX_AllExtensions();

    void toTCX_GarminActivity_data();
    void toTCX_GarminActivity();

    void toTCX_GarminCourse_data();
    void toTCX_GarminCourse();

    void toTCX_UTC_data();
    void toTCX_UTC();

    void unzip_data();
    void unzip();

};
