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

#include "generalhrmoptionstab.h"

#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

const QString GeneralHrmOptions::ExportRrFilesSettingsKey = QLatin1String("rrFiles");

const bool GeneralHrmOptions::ExportRrFilesDefaultSetting = true;

GeneralHrmOptions::GeneralHrmOptions(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    rrFiles = new QCheckBox(tr("Export separate R-R files"));
    rrFiles->setToolTip(tr("Generate spearate HRM files containing R-R data"));
    rrFiles->setWhatsThis(tr("Check this box to generate matching HRM files containing "
                             "heart rate variablility data whenever exporting to HRM."));
    load();

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(rrFiles);
    setLayout(vBox);
}

void GeneralHrmOptions::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("hrm"));
    rrFiles->setChecked(settings.value(ExportRrFilesSettingsKey, ExportRrFilesDefaultSetting).toBool());
}

void GeneralHrmOptions::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("hrm"));
    settings.setValue(ExportRrFilesSettingsKey, rrFiles->isChecked());
}
