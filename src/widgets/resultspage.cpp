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

#include "resultspage.h"

#include <QProgressBar>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ResultsPage::ResultsPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle(tr("Converting..."));
    setButtonText(QWizard::FinishButton, tr("Close"));

    progressBar = new QProgressBar();

    showDetailsButton = new QPushButton(tr("Show details"));

    detailsBox = new QTextEdit();
    detailsBox->setReadOnly(true);
    detailsBox->setVisible(false);

    QVBoxLayout * const vBox = new QVBoxLayout;
    vBox->addWidget(progressBar);
    vBox->addWidget(showDetailsButton);
    vBox->addWidget(detailsBox);
    setLayout(vBox);

    connect(showDetailsButton, SIGNAL(clicked()), this, SLOT(showDetails()));
}

bool ResultsPage::isComplete() const
{
    return false;
}

// Protected slots.

void ResultsPage::showDetails()
{
    showDetailsButton->setVisible(false);
    detailsBox->setVisible(true);
}
