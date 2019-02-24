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

#include "tcxextensionstab.h"

#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

const QString TcxExtensionsTab::GarminActivityExtSettingsKey = QLatin1String("garminActivityExt");
const QString TcxExtensionsTab::GarminCourseExtSettingsKey   = QLatin1String("garminCourseExt");

const bool TcxExtensionsTab::GarminActivityExtDefaultSetting = true;
const bool TcxExtensionsTab::GarminCourseExtDefaultSetting   = true;

TcxExtensionsTab::TcxExtensionsTab(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    garminActivityExt = new QCheckBox(tr("Garmin Activity Extension"));
    garminCourseExt = new QCheckBox(tr("Garmin Course Extension"));
    load();

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(garminActivityExt);
    vBox->addWidget(garminCourseExt);
    setLayout(vBox);
}

void TcxExtensionsTab::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("tcx"));
    garminActivityExt->setChecked(settings.value(GarminActivityExtSettingsKey, GarminActivityExtDefaultSetting).toBool());
    garminCourseExt->setChecked(settings.value(GarminCourseExtSettingsKey, GarminCourseExtDefaultSetting).toBool());
}

void TcxExtensionsTab::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("tcx"));
    settings.setValue(GarminActivityExtSettingsKey, garminActivityExt->isChecked());
    settings.setValue(GarminCourseExtSettingsKey, garminCourseExt->isChecked());
}
