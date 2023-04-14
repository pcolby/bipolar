// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __TCX_OPTIONS_DIALOG__
#define __TCX_OPTIONS_DIALOG__

#include <QDialog>

class TcxOptionsDialog : public QDialog {
    Q_OBJECT

public:
    TcxOptionsDialog(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

};

#endif // __TCX_OPTIONS_DIALOG__
