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

#ifndef __HRM_EXTENSIONS_TAB__
#define __HRM_EXTENSIONS_TAB__

#include <QWidget>

class QCheckBox;

class HrmExtensionsTab : public QWidget {
    Q_OBJECT

public:
    static const QString LapNamesExtSettingsKey;

    static const bool LapNamesExtDefaultSetting;

    HrmExtensionsTab(QWidget *parent=0, Qt::WindowFlags flags=0);

public slots:
    void load();
    void save();

protected:
    QCheckBox * lapNamesExt;

};

#endif // __HRM_EXTENSIONS_TAB__
