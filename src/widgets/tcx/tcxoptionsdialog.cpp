// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tcxoptionsdialog.h"

#include "generaltcxoptionstab.h"
#include "tcxextensionstab.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

TcxOptionsDialog::TcxOptionsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setWindowTitle(tr("TCX Options"));

    GeneralTcxOptions * const generalOptionsTab = new GeneralTcxOptions();
    TcxExtensionsTab * const extensionsTab = new TcxExtensionsTab();
    connect(this, SIGNAL(accepted()), generalOptionsTab, SLOT(save()));
    connect(this, SIGNAL(accepted()), extensionsTab, SLOT(save()));

    QTabWidget * const tabs = new QTabWidget();
    tabs->addTab(generalOptionsTab, tr("General"));
    tabs->addTab(extensionsTab, tr("Extensions"));

    QDialogButtonBox * const buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout * const vBox = new QVBoxLayout();
    vBox->addWidget(tabs);
    vBox->addWidget(buttons);
    setLayout(vBox);
}
