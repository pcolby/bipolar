// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __OUTPUTS_PAGE__
#define __OUTPUTS_PAGE__

#include <QComboBox>
#include <QWizardPage>

class OutputsPage : public QWizardPage {
    Q_OBJECT

public:
    explicit OutputsPage(QWidget *parent=0);
    virtual void initializePage();
    virtual bool isComplete() const;
    virtual bool validatePage();

protected:
    QComboBox * outputFolder;

protected slots:
    void browseForFolder();
    void checkBoxClicked();
    void formatChanged(const QString &format);
    void showAdvancedOptions(const QString &link);
    void showFileNameFormatHelp();

};

#endif // __OUTPUTS_PAGE__
