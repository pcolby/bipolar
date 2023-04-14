// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hrmextensionstab.h"

#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

const QString HrmExtensionsTab::LapNamesExtSettingsKey = QLatin1String("lapNamesExt");

const bool HrmExtensionsTab::LapNamesExtDefaultSetting = true;

HrmExtensionsTab::HrmExtensionsTab(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    lapNamesExt = new QCheckBox(tr("Include \"LapNames\" section"));
    load();

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(lapNamesExt);
    setLayout(vBox);
}

void HrmExtensionsTab::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("hrm"));
    lapNamesExt->setChecked(settings.value(LapNamesExtSettingsKey, LapNamesExtDefaultSetting).toBool());
}

void HrmExtensionsTab::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("hrm"));
    settings.setValue(LapNamesExtSettingsKey, lapNamesExt->isChecked());
}
