// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __MAIN_WIZARD__
#define __MAIN_WIZARD__

#include <QWizard>

class MainWizard : public QWizard {
  Q_OBJECT

public:
    MainWizard(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void checkHook();

protected:
    virtual void showEvent(QShowEvent * event);

};

#endif // __MAIN_WIZARD__
