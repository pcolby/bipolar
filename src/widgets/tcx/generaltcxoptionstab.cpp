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

#include "generaltcxoptionstab.h"

#include <QCheckBox>
#include <QVBoxLayout>

GeneralTcxOptions::GeneralTcxOptions(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QCheckBox * const utcOnly = new QCheckBox(tr("Convert timestamps to UTC"));
    utcOnly->setToolTip(tr("Convert all local timestamps to UTC"));
    utcOnly->setWhatsThis(tr("Check this box to have all TCX timestamps converted to UTC."));
    utcOnly->setEnabled(false); ///< Not implemented yet.

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(utcOnly);
    setLayout(vBox);
}
