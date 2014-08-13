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

#include "outputspage.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

OutputsPage::OutputsPage(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Output Options"));
    setSubTitle(tr("Select the blah blah blah."));

    QFormLayout * const form = new QFormLayout();

    {
        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(new QLineEdit());
        hBox->addWidget(new QPushButton(tr("Browse...")));
        QVBoxLayout * const vBox = new QVBoxLayout();
        vBox->addItem(hBox);
        vBox->addWidget(new QCheckBox(tr("Use input directory")));
        form->addRow(tr("Output Directory:"), vBox);
    }

    setLayout(form);
}

//bool InputPage::isComplete() const {
//}
