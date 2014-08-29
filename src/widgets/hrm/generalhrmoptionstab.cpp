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

#include "generalhrmoptionstab.h"

#include <QCheckBox>
#include <QVBoxLayout>

GeneralHrmOptions::GeneralHrmOptions(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QCheckBox * const rrFiles = new QCheckBox(tr("Export separate R-R files"));
    rrFiles->setToolTip(tr("Generate spearate HRM files containing R-R data"));
    rrFiles->setWhatsThis(tr("Check this box to generate matching HRM files containing "
                             "heart rate variablility data whenever exporting to HRM."));
    rrFiles->setChecked(true);  ///< Currently always "on".
    rrFiles->setEnabled(false); ///< Currently always "on".

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(rrFiles);
    setLayout(vBox);
}
