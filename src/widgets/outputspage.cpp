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
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QPushButton>

OutputsPage::OutputsPage(QWidget *parent) : QWizardPage(parent) {
    setTitle(tr("Output Options"));

    QFormLayout * const form = new QFormLayout();

    outputFolder = new QComboBox();
    outputFolder->addItem(tr("Use the same folder as the input files"));

    {
        QPushButton * const browseButton = new QPushButton();
        browseButton->setFlat(true);
        browseButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
        browseButton->setText(tr("Browse..."));
        browseButton->setToolTip(tr("Choose output folder"));
        connect(browseButton, SIGNAL(clicked()), this, SLOT(browseForFolder()));

        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(outputFolder, 1);
        hBox->addWidget(browseButton);
        form->addRow(tr("Output Folder:"), hBox);
    }

    setLayout(form);
}

bool OutputsPage::isComplete() const {
    return true;//false;
}

// Protected slots.

void OutputsPage::browseForFolder() {
    const QString dirName = QFileDialog::getExistingDirectory(this);
    if (!dirName.isEmpty()) {
        if (outputFolder->count() < 2) {
            outputFolder->addItem(QDir::toNativeSeparators(dirName), dirName);
        } else {
            outputFolder->setItemText(1, QDir::toNativeSeparators(dirName));
            outputFolder->setItemData(1, dirName);
        }
        outputFolder->setCurrentIndex(1);
    }
}
