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

#include <QObject>

class TestFixnum : public QObject {
    Q_OBJECT

private slots:
    void parseDouble_data();
    void parseDouble();
    void parseDoubles_data();
    void parseDoubles();

    void parseFloat_data();
    void parseFloat();
    void parseFloats_data();
    void parseFloats();

    void parseSigned32_data();
    void parseSigned32();
    void parseSigned32s_data();
    void parseSigned32s();

    void parseSigned64_data();
    void parseSigned64();
    void parseSigned64s_data();
    void parseSigned64s();

    void parseUnsigned32_data();
    void parseUnsigned32();
    void parseUnsigned32s_data();
    void parseUnsigned32s();

    void parseUnsigned64_data();
    void parseUnsigned64();
    void parseUnsigned64s_data();
    void parseUnsigned64s();

};
