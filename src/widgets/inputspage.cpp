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
    setSubTitle(tr("Add folders containing training sessions to be converted."));

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

bool InputsPage::isComplete() const {
    // As long as we have at least one readable folder, we're ready to move on.
    for (int index = 0; index < inputFoldersList->count(); ++index) {
        QDir dir(inputFoldersList->item(index)->data(Qt::UserRole).toString());
        if (dir.isReadable()) {
            return true;
        }
    }
    return false;
}

// Public slots.

void InputsPage::load()
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

void InputsPage::save()
{
    QStringList folders;
    for (int index = 0; index < inputFoldersList->count(); ++index) {
        folders.append(inputFoldersList->item(index)->text());
    }
    if (!folders.isEmpty()) {
        QSettings settings;
        settings.setValue(QLatin1String("inputFolders"), folders);
    }
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
    const QString dirName = QFileDialog::getExistingDirectory(this);
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
