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
