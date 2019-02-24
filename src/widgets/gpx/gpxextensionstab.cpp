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

#include "gpxextensionstab.h"

#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

const QString GpxExtensionsTab::CluetrustGpxExtSettingsKey       = QLatin1String("cluetrustGpxExt");
const QString GpxExtensionsTab::GarminAccelerationExtSettingsKey = QLatin1String("garminAccelerationExt");
const QString GpxExtensionsTab::GarminTrackPointExtSettingsKey   = QLatin1String("garminTrackPointExt");

const bool GpxExtensionsTab::CluetrustGpxExtDefaultSetting       = true;
const bool GpxExtensionsTab::GarminAccelerationExtDefaultSetting = true;
const bool GpxExtensionsTab::GarminTrackPointExtDefaultSetting   = true;

GpxExtensionsTab::GpxExtensionsTab(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    cluetrustGpxExt = new QCheckBox(tr("Cluetrust GPX Extension"));
    garminAccelerationExt = new QCheckBox(tr("Garmin Acceleration Extension"));
    garminTrackPointExt = new QCheckBox(tr("Garmin TrackPoint Extension"));
    load();

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(cluetrustGpxExt);
    vBox->addWidget(garminAccelerationExt);
    vBox->addWidget(garminTrackPointExt);
    setLayout(vBox);
}

void GpxExtensionsTab::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("gpx"));
    cluetrustGpxExt->setChecked(settings.value(CluetrustGpxExtSettingsKey, CluetrustGpxExtDefaultSetting).toBool());
    garminAccelerationExt->setChecked(settings.value(GarminAccelerationExtSettingsKey, GarminAccelerationExtDefaultSetting).toBool());
    garminTrackPointExt->setChecked(settings.value(GarminTrackPointExtSettingsKey, GarminTrackPointExtDefaultSetting).toBool());
}

void GpxExtensionsTab::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("gpx"));
    settings.setValue(CluetrustGpxExtSettingsKey, cluetrustGpxExt->isChecked());
    settings.setValue(GarminAccelerationExtSettingsKey, garminAccelerationExt->isChecked());
    settings.setValue(GarminTrackPointExtSettingsKey, garminTrackPointExt->isChecked());
}
