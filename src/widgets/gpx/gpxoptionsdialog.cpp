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

#include "gpxoptionsdialog.h"

#include "generalgpxoptionstab.h"
#include "gpxextensionstab.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

GpxOptionsDialog::GpxOptionsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setWindowTitle(tr("GPX Options"));

  //GeneralGpxOptions * const generalOptionsTab = new GeneralGpxOptions();
    GpxExtensionsTab * const extensionsTab = new GpxExtensionsTab();
  //connect(this, SIGNAL(accepted()), generalOptionsTab, SLOT(save()));
    connect(this, SIGNAL(accepted()), extensionsTab, SLOT(save()));

    QTabWidget * const tabs = new QTabWidget();
  //tabs->addTab(generalOptionsTab, tr("General"));
    tabs->addTab(extensionsTab, tr("Extensions"));

    QDialogButtonBox * const buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(tabs);
    vBox->addWidget(buttons);
    setLayout(vBox);
}
