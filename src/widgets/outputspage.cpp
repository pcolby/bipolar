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
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

OutputsPage::OutputsPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle(tr("Output Options"));

    QFormLayout * const form = new QFormLayout();

    {
        outputFolder = new QComboBox();
        outputFolder->addItem(tr("Use the same folder as the input files"));

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

    {
        QPushButton * const infoButton = new QPushButton();
        infoButton->setFlat(true);
        infoButton->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
        infoButton->setToolTip(tr("Filename format information"));
        infoButton->setWhatsThis(tr("Click this button for information on the filename format."));
        connect(infoButton, SIGNAL(clicked()), this, SLOT(showFileNameFormatHelp()));

        QLineEdit * format = new QLineEdit();

        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(format);
        hBox->addWidget(infoButton);

        form->addRow(tr("Output Filename Format:"), hBox);

        registerField(QLatin1String("filenameFormat"), format);
    }

    {
        QCheckBox * const gpxCheckBox = new QCheckBox(tr("GPX"));
        QCheckBox * const hrmCheckBox = new QCheckBox(tr("HRM"));
        QCheckBox * const tcxCheckBox = new QCheckBox(tr("TCX"));

        QVBoxLayout * vBox = new QVBoxLayout();
        vBox->addWidget(gpxCheckBox);
        vBox->addWidget(hrmCheckBox);
        vBox->addWidget(tcxCheckBox);

        form->addRow(tr("Output Formats:"), vBox);

        registerField(QLatin1String("gpxEnabled"), gpxCheckBox);
        registerField(QLatin1String("hrmEnabled"), hrmCheckBox);
        registerField(QLatin1String("tcxEnabled"), tcxCheckBox);
    }

    setLayout(form);
}

void OutputsPage::initializePage()
{
    QSettings settings;

    {
        const QString folder = settings.value(QLatin1String("outputFolder")).toString();
        if (!folder.isEmpty()) {
            outputFolder->addItem(QDir::toNativeSeparators(folder), folder);
            outputFolder->setCurrentIndex(1);
        }
        const int outputFolderIndex = settings.value(QLatin1String("outputFolderIndex"), 0).toInt();
        if (outputFolderIndex < outputFolder->count()) {
            outputFolder->setCurrentIndex(outputFolderIndex);
        }
    }

    setField(QLatin1String("filenameFormat"), settings.value(QLatin1String("filenameFormat")));

    setField(QLatin1String("gpxEnabled"), settings.value(QLatin1String("gpxEnabled"), true));
    setField(QLatin1String("hrmEnabled"), settings.value(QLatin1String("hrmEnabled"), true));
    setField(QLatin1String("tcxEnabled"), settings.value(QLatin1String("tcxEnabled"), true));
}

bool OutputsPage::isComplete() const
{
    return true;//false;
}

// Public slots.

void OutputsPage::save()
{
    QSettings settings;
    settings.setValue(QLatin1String("outputFolder"),
        (outputFolder->count() > 1) ? outputFolder->itemData(1).toString() : QString());
    settings.setValue(QLatin1String("outputFolderIndex"), outputFolder->currentIndex());
    settings.setValue(QLatin1String("filenameFormat"), field(QLatin1String("filenameFormat")));
    settings.setValue(QLatin1String("gpxEnabled"), field(QLatin1String("gpxEnabled")));
    settings.setValue(QLatin1String("hrmEnabled"), field(QLatin1String("hrmEnabled")));
    settings.setValue(QLatin1String("tcxEnabled"), field(QLatin1String("tcxEnabled")));
}

// Protected slots.

void OutputsPage::browseForFolder()
{
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

void OutputsPage::showFileNameFormatHelp()
{
    QMessageBox * const dialog = new QMessageBox();
    dialog->setIcon(QMessageBox::Information);
    QFile file(QLatin1String(":/html/filename syntax.html"));
    file.open(QFile::ReadOnly);
    dialog->setText(QString::fromUtf8(file.readAll()));
    dialog->setWindowTitle(tr("Filename Format"));
    dialog->show();
}
