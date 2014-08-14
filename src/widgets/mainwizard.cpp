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

#include "mainwizard.h"
#include "inputspage.h"
#include "outputspage.h"
#include <QApplication>

MainWizard::MainWizard(QWidget *parent, Qt::WindowFlags flags): QWizard(parent,flags) {
    setWindowTitle(tr("%1 %2")
        .arg(QApplication::applicationName())
        .arg(QStringList(QApplication::applicationVersion().split(QLatin1Char('.')).mid(0, 3)).join(QLatin1Char('.'))));

    InputsPage * const inputsPage = new InputsPage();
    OutputsPage * const outputsPage = new OutputsPage();

    addPage(inputsPage);
    addPage(outputsPage);

    setButtonText(FinishButton, tr("Convert"));

    connect(this, SIGNAL(accepted()), inputsPage, SLOT(save()));
  //connect(this, SIGNAL(accepted()), outputsPage, SLOT(save()));
}
