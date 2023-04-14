// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GPX_OPTIONS_DIALOG__
#define __GPX_OPTIONS_DIALOG__

#include <QDialog>

class GpxOptionsDialog : public QDialog {
    Q_OBJECT

public:
    GpxOptionsDialog(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

};

#endif // __GPX_OPTIONS_DIALOG__
