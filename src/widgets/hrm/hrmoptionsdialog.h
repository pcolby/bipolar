// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __HRM_OPTIONS_DIALOG__
#define __HRM_OPTIONS_DIALOG__

#include <QDialog>

class HrmOptionsDialog : public QDialog {
    Q_OBJECT

public:
    HrmOptionsDialog(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

};

#endif // __HRM_OPTIONS_DIALOG__
