// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
