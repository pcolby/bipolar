// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
