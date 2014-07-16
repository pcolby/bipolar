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

#include <QObject>

class TestTrainingSession : public QObject {
    Q_OBJECT

private slots:
    void isGzipped_data();
    void isGzipped();

    void parseLaps_data();
    void parseLaps();

    void parseRoute_data();
    void parseRoute();

    void parseSamples_data();
    void parseSamples();

    void parseZones_data();
    void parseZones();

    void toGPX_data();
    void toGPX();

    void toTCX_data();
    void toTCX();

    void unzip_data();
    void unzip();

};
