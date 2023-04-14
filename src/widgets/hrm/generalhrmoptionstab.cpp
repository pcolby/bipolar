// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
