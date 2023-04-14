// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "generaltcxoptionstab.h"

#include <QCheckBox>
#include <QSettings>
#include <QVBoxLayout>

const QString GeneralTcxOptions::UtcOnlySettingsKey = QLatin1String("garminActivityExt");

const bool GeneralTcxOptions::UtcOnlyDefaultSetting = true;

GeneralTcxOptions::GeneralTcxOptions(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    utcOnly = new QCheckBox(tr("Convert timestamps to UTC"));
    utcOnly->setToolTip(tr("Convert all local timestamps to UTC"));
    utcOnly->setWhatsThis(tr("Check this box to have all TCX timestamps converted to UTC."));
    load();

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(utcOnly);
    setLayout(vBox);
}

void GeneralTcxOptions::load()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("tcx"));
    utcOnly->setChecked(settings.value(UtcOnlySettingsKey, UtcOnlyDefaultSetting).toBool());
}

void GeneralTcxOptions::save()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("tcx"));
    settings.setValue(UtcOnlySettingsKey, utcOnly->isChecked());
}
