// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
