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
    setSubTitle(tr("Select the path(s) containing training sessions to convert."));

    addButton = new QPushButton();
    addButton->setFlat(true);
    addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    addButton->setToolTip(tr("Add folder"));

    removeButton = new QPushButton();
    removeButton->setEnabled(false);
    removeButton->setFlat(true);
    removeButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    removeButton->setToolTip(tr("Remove folder"));

    inputFoldersList = new QListWidget(this);
    inputFoldersList->setAlternatingRowColors(true);
    inputFoldersList->setSelectionMode(QListWidget::ExtendedSelection);
    load();

    QFormLayout * const form = new QFormLayout;
    {
        QVBoxLayout * const buttonsBox = new QVBoxLayout();
        buttonsBox->addWidget(addButton);
        buttonsBox->addWidget(removeButton);
        buttonsBox->addStretch();

        QHBoxLayout * const hBox = new QHBoxLayout();
        hBox->addWidget(inputFoldersList);
        hBox->addItem(buttonsBox);
        form->addRow(tr("Input Folders:"), hBox);
    }
    setLayout(form);

    connect(addButton, SIGNAL(clicked()), this, SLOT(browseForFolder()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeFolders()));
    connect(inputFoldersList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
}

//bool InputPage::isComplete() const {
//}

// Public slots.

void InputsPage::load()
{
    QSettings settings;

    {
        QStringList folders = settings.value(QLatin1String("inputFolders")).toStringList();
        if (folders.isEmpty()) {
            folders.append(hookInputFolder(true));
        }
        foreach (const QString &folder, folders) {
            QListWidgetItem * item = new QListWidgetItem(folder);
            /// @todo Icon.
            inputFoldersList->addItem(item);
        }
    }
}

void InputsPage::save()
{
    QSettings settings;

    QStringList folders;
    for (int index = 0; index < inputFoldersList->count(); ++index) {
        folders.append(inputFoldersList->item(index)->text());
    }
    if (!folders.isEmpty()) {
        settings.setValue(QLatin1String("inputFolders"), folders);
    }
}

// Protecte methods.

QString InputsPage::hookInputFolder(const bool native)
{
    QString folder =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        QLatin1String("/Polar/PolarFlowSync/export");
    return native ? QDir::toNativeSeparators(folder) : folder;
}

// Protected slots.

void InputsPage::browseForFolder()
{
    const QString dirName = QFileDialog::getExistingDirectory(this);
    if (!dirName.isEmpty()) {
        QListWidgetItem * const item = new QListWidgetItem();
        item->setText(QDir::toNativeSeparators(dirName));
        item->setData(Qt::UserRole, dirName);
        inputFoldersList->addItem(item);
    }
}

void InputsPage::removeFolders()
{
    foreach (QListWidgetItem * const item, inputFoldersList->selectedItems()) {
        delete item;
    }
}

void InputsPage::selectionChanged()
{
    removeButton->setEnabled(!inputFoldersList->selectedItems().isEmpty());
}
