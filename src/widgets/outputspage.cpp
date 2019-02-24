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

#include "outputspage.h"

#include "gpx/gpxoptionsdialog.h"
#include "hrm/hrmoptionsdialog.h"
#include "tcx/tcxoptionsdialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>

OutputsPage::OutputsPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle(tr("Output Options"));
    setButtonText(QWizard::NextButton, tr("Convert"));

    QFormLayout * const form = new QFormLayout();

    {
        outputFolder = new QComboBox();
        outputFolder->addItem(tr("Use the same folder as the input files"));
        outputFolder->setWhatsThis(tr("Use this box to choose where output files "
                                      "should be written."));

        QPushButton * const browseButton = new QPushButton();
        browseButton->setFlat(true);
        browseButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
        browseButton->setText(tr("Browse..."));
        browseButton->setToolTip(tr("Choose output folder"));
        browseButton->setWhatsThis(tr("Click this button to choose a directory "
                                      "to write all output files to."));
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
        format->setWhatsThis(tr("Use this box to specifiy a format for output "
                                "file names. May be left empty."));

        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(format);
        hBox->addWidget(infoButton);

        form->addRow(tr("Output Filename Format:"), hBox);

        registerField(QLatin1String("outputFileNameFormat"), format);

        connect(format, SIGNAL(textChanged(QString)), this, SLOT(formatChanged(QString)));
    }

    {
        QCheckBox * const gpxCheckBox = new QCheckBox(tr("GPX"));
        QCheckBox * const hrmCheckBox = new QCheckBox(tr("HRM"));
        QCheckBox * const tcxCheckBox = new QCheckBox(tr("TCX"));
        gpxCheckBox->setToolTip(tr("Enable GPX output"));
        hrmCheckBox->setToolTip(tr("Enable HRM output"));
        tcxCheckBox->setToolTip(tr("Enable TCX output"));
        gpxCheckBox->setWhatsThis(tr("Check this box to enable GPX output."));
        hrmCheckBox->setWhatsThis(tr("Check this box to enable HRM output."));
        tcxCheckBox->setWhatsThis(tr("Check this box to enable TCX output."));

        QLabel * const advancedGpxLabel = new QLabel(QString::fromLatin1("<a href='gpx'>%1</a>").arg(tr("advanced...")));
        QLabel * const advancedHrmLabel = new QLabel(QString::fromLatin1("<a href='hrm'>%1</a>").arg(tr("advanced...")));
        QLabel * const advancedTcxLabel = new QLabel(QString::fromLatin1("<a href='tcx'>%1</a>").arg(tr("advanced...")));
        advancedGpxLabel->setToolTip(tr("Choose advanced options for GPX output"));
        advancedHrmLabel->setToolTip(tr("Choose advanced options for HRM output"));
        advancedTcxLabel->setToolTip(tr("Choose advanced options for TCX output"));
        advancedGpxLabel->setWhatsThis(tr("Click this link to choose advanced options for GPX output."));
        advancedGpxLabel->setWhatsThis(tr("Click this link to choose advanced options for HRM output."));
        advancedGpxLabel->setWhatsThis(tr("Click this link to choose advanced options for TCX output."));

        QGridLayout * const grid = new QGridLayout();
        #ifdef Q_OS_MAC
        grid->setHorizontalSpacing(12);
        #endif
        grid->addWidget(gpxCheckBox, 0, 0);
        grid->addWidget(hrmCheckBox, 1, 0);
        grid->addWidget(tcxCheckBox, 2, 0);
        grid->addWidget(advancedGpxLabel, 0, 1);
        grid->addWidget(advancedHrmLabel, 1, 1);
        grid->addWidget(advancedTcxLabel, 2, 1);
        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addItem(grid);
        hBox->addStretch();
        form->addRow(tr("Output Formats:"), hBox);

        registerField(QLatin1String("gpxEnabled"), gpxCheckBox);
        registerField(QLatin1String("hrmEnabled"), hrmCheckBox);
        registerField(QLatin1String("tcxEnabled"), tcxCheckBox);

        connect(gpxCheckBox, SIGNAL(clicked()), this, SLOT(checkBoxClicked()));
        connect(hrmCheckBox, SIGNAL(clicked()), this, SLOT(checkBoxClicked()));
        connect(tcxCheckBox, SIGNAL(clicked()), this, SLOT(checkBoxClicked()));

        connect(advancedGpxLabel, SIGNAL(linkActivated(QString)), this, SLOT(showAdvancedOptions(QString)));
        connect(advancedHrmLabel, SIGNAL(linkActivated(QString)), this, SLOT(showAdvancedOptions(QString)));
        connect(advancedTcxLabel, SIGNAL(linkActivated(QString)), this, SLOT(showAdvancedOptions(QString)));
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
        const int outputFolderIndex =
            settings.value(QLatin1String("outputFolderIndex"), 0).toInt();
        if (outputFolderIndex < outputFolder->count()) {
            outputFolder->setCurrentIndex(outputFolderIndex);
        }
    }

    setField(QLatin1String("outputFileNameFormat"),
             settings.value(QLatin1String("outputFileNameFormat"),
                            QLatin1String("$date $time $sessionName")));

    setField(QLatin1String("gpxEnabled"), settings.value(QLatin1String("gpxEnabled"), true));
    setField(QLatin1String("hrmEnabled"), settings.value(QLatin1String("hrmEnabled"), true));
    setField(QLatin1String("tcxEnabled"), settings.value(QLatin1String("tcxEnabled"), true));
}

bool OutputsPage::isComplete() const
{
    // Make sure the output filename format is unique enough.
    const QString format = field(QLatin1String("outputFileNameFormat")).toString();
    const bool uniqueFormat =
       ((format.contains(QLatin1String("$baseName"))) ||
        (format.contains(QLatin1String("$sessionId")))||
        (format.contains(QLatin1String("$date")) && format.contains(QLatin1String("$time"))));
    if (!uniqueFormat) {
        return false;
    }

    // Return true, as long as at least one ouput format is enabled.
    return ((field(QLatin1String("gpxEnabled")).toBool()) ||
            (field(QLatin1String("hrmEnabled")).toBool()) ||
            (field(QLatin1String("tcxEnabled")).toBool()));
}

bool OutputsPage::validatePage()
{
    QSettings settings;
    settings.setValue(QLatin1String("outputFolder"),
        (outputFolder->count() > 1) ? outputFolder->itemData(1).toString() : QString());
    settings.setValue(QLatin1String("outputFolderIndex"), outputFolder->currentIndex());
    settings.setValue(QLatin1String("outputFileNameFormat"), field(QLatin1String("outputFileNameFormat")));
    settings.setValue(QLatin1String("gpxEnabled"), field(QLatin1String("gpxEnabled")));
    settings.setValue(QLatin1String("hrmEnabled"), field(QLatin1String("hrmEnabled")));
    settings.setValue(QLatin1String("tcxEnabled"), field(QLatin1String("tcxEnabled")));
    return true;
}

// Protected slots.

void OutputsPage::browseForFolder()
{
    const QStringList homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    const QString dirName = QFileDialog::getExistingDirectory(this, QString(),
        (homeDir.isEmpty()) ? QString() : homeDir.first());
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

void OutputsPage::checkBoxClicked()
{
    emit completeChanged();
}

void OutputsPage::formatChanged(const QString &format)
{
    Q_UNUSED(format);
    emit completeChanged();
}

void OutputsPage::showAdvancedOptions(const QString &link)
{
    if (link == QLatin1String("gpx")) {
        GpxOptionsDialog dialog;
        dialog.exec();
    } else if (link == QLatin1String("hrm")) {
        HrmOptionsDialog dialog;
        dialog.exec();
    } else if (link == QLatin1String("tcx")) {
        TcxOptionsDialog dialog;
        dialog.exec();
    } else {
        Q_ASSERT(false);
    }
}

void OutputsPage::showFileNameFormatHelp()
{
    QMessageBox * const dialog = new QMessageBox(this);
    dialog->setIcon(QMessageBox::Information);
    dialog->setText(tr(
        "This version of Bipolar supports the following expressions:\n\n%1")
        .arg(QLatin1String(
            "$baseName, $date, $dateExt, $dateUTC, $dateExtUTC, $time, $timeExt, "
            "$timeUTC, $timeExtUTC, $sessionId, $sessionName, $userId, $username")));
    dialog->setInformativeText(tr("See the <a href=\"%1\">Bipolar wiki</a> for more details.")
        .arg(QLatin1String("https://github.com/pcolby/bipolar/wiki/Output-Filenames#filename-format-syntax")));
    dialog->setWindowTitle(tr("Filename Format"));
    dialog->setModal(false);
    dialog->show();
}
