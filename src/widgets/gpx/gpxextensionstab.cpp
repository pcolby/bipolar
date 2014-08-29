/*
    Copyright 2014 Paul Colby

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
#include <QVBoxLayout>

GpxExtensionsTab::GpxExtensionsTab(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QCheckBox * cluetrustGpxExt = new QCheckBox(tr("Cluetrust GPX Extension"));
    QCheckBox * garminTrackPointExt = new QCheckBox(tr("Garmin TrackPoint Extension"));
    cluetrustGpxExt->setEnabled(false); ///< Not implemented yet.
    garminTrackPointExt->setEnabled(false); ///< Not implemented yet.

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(cluetrustGpxExt);
    vBox->addWidget(garminTrackPointExt);
    setLayout(vBox);
}

void GpxExtensionsTab::load()
{

}

void GpxExtensionsTab::save()
{

}
