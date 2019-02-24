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

#ifndef __INPUTS_PAGE__
#define __INPUTS_PAGE__

#include <QPushButton>
#include <QWizardPage>

class QListWidget;
class QListWidgetItem;
class QPushButton;

class InputsPage : public QWizardPage {
    Q_OBJECT

public:
    explicit InputsPage(QWidget *parent=0);
    virtual void initializePage();
    virtual bool isComplete() const;
    virtual bool validatePage();

protected:
    QListWidget * inputFoldersList;
    QPushButton * removeButton;

    QListWidgetItem * addFolder(const QString &path);
    static QString defaultInputFolder(const bool native);

protected slots:
    void browseForFolder();
    void removeFolders();
    void selectionChanged();

};

#endif // __INPUTS_PAGE__
