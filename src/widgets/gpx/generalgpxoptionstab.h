// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GENERAL_GPX_OPTIONS_TAB__
#define __GENERAL_GPX_OPTIONS_TAB__

#include <QWidget>

class QCheckBox;

class GeneralGpxOptions : public QWidget {
    Q_OBJECT

public:
    GeneralGpxOptions(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

};

#endif // __GENERAL_GPX_OPTIONS_TAB__
