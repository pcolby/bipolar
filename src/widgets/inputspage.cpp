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

#include "inputspage.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QVBoxLayout>

InputsPage::InputsPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle(tr("Input Options"));
    setSubTitle(tr("Add folders containing training sessions to process."));

    QPushButton * const addButton = new QPushButton();
    addButton->setFlat(true);
    addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    addButton->setToolTip(tr("Add folder"));
    addButton->setWhatsThis(tr("Click this button to add a folder containing "
                               "training session files to process."));

    removeButton = new QPushButton();
    removeButton->setEnabled(false);
    removeButton->setFlat(true);
    removeButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    removeButton->setToolTip(tr("Remove folder"));
    removeButton->setWhatsThis(tr("Click this button to remove the currently "
                                  "selected folders from the list."));

    inputFoldersList = new QListWidget(this);
    inputFoldersList->setAlternatingRowColors(true);
    inputFoldersList->setSelectionMode(QListWidget::ExtendedSelection);
    inputFoldersList->setWhatsThis(tr("Contains a list of folders that will be "
                                      "searched for training sessions to process."));

    {
        QVBoxLayout * const buttonsBox = new QVBoxLayout();
        buttonsBox->addWidget(addButton);
        buttonsBox->addWidget(removeButton);
        buttonsBox->addStretch();

        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(inputFoldersList);
        hBox->addItem(buttonsBox);
        setLayout(hBox);
    }

    connect(addButton, SIGNAL(clicked()), this, SLOT(browseForFolder()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeFolders()));
    connect(inputFoldersList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}

void InputsPage::initializePage()
{
    QSettings settings;
    QStringList folders = settings.value(QLatin1String("inputFolders")).toStringList();
    if (folders.isEmpty()) {
        folders.append(defaultInputFolder(true));
    }
    foreach (const QString &folder, folders) {
        addFolder(folder);
        inputFoldersList->sortItems();
    }
    emit completeChanged();
}

bool InputsPage::isComplete() const {
    // As long as we have at least one readable folder, we're ready to move on.
    bool isReadable = false;
    for (int index = 0; index < inputFoldersList->count(); ++index) {
        QDir dir(inputFoldersList->item(index)->data(Qt::UserRole).toString());
        if (dir.isReadable()) {
            isReadable = true;
        } else {
            QListWidgetItem * const item = inputFoldersList->item(index);
            item->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
            item->setToolTip(tr("Folder does not exist, or is not readable"));
        }
    }
    return isReadable;
}

bool InputsPage::validatePage()
{
    QStringList folders;
    for (int index = 0; index < inputFoldersList->count(); ++index) {
        folders.append(inputFoldersList->item(index)->text());
    }
    if (!folders.isEmpty()) {
        QSettings settings;
        settings.setValue(QLatin1String("inputFolders"), folders);
    }
    return true;
}

// Protected methods.

QListWidgetItem * InputsPage::addFolder(const QString &path)
{
    QListWidgetItem * const item = new QListWidgetItem();
    item->setData(Qt::UserRole, path);
    item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    item->setText(QDir::toNativeSeparators(path));
    inputFoldersList->addItem(item);
    return item;
}

QString InputsPage::defaultInputFolder(const bool native)
{
    QString folder =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        QLatin1String("/Polar/PolarFlowSync/export");
    return native ? QDir::toNativeSeparators(folder) : folder;
}

// Protected slots.

void InputsPage::browseForFolder()
{
    // Default to the hook export directory, if it exists, otherwise the home directory.
    QString initialDirectory = defaultInputFolder(false);
    if (!QFile::exists(initialDirectory)) {
        const QStringList homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        initialDirectory = (homeDir.isEmpty()) ? QString() : homeDir.first();
    }

    // Browse for the directory to add.
    const QString dirName = QFileDialog::getExistingDirectory(this, QString(), initialDirectory);
    if (!dirName.isEmpty()) {
        QListWidgetItem * const item = addFolder(dirName);
        inputFoldersList->sortItems();
        inputFoldersList->clearSelection();
        inputFoldersList->setCurrentItem(item);
    }
    emit completeChanged();
}

void InputsPage::removeFolders()
{
    foreach (QListWidgetItem * const item, inputFoldersList->selectedItems()) {
        delete item;
    }
    emit completeChanged();
}

void InputsPage::selectionChanged()
{
    removeButton->setEnabled(!inputFoldersList->selectedItems().isEmpty());
}
