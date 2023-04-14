// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GENERAL_TCX_OPTIONS_TAB__
#define __GENERAL_TCX_OPTIONS_TAB__

#include <QWidget>

class QCheckBox;

class GeneralTcxOptions : public QWidget {
    Q_OBJECT

public:
    static const QString UtcOnlySettingsKey;

    static const bool UtcOnlyDefaultSetting;

    GeneralTcxOptions(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

protected:
    QCheckBox * utcOnly;

};

#endif // __GENERAL_TCX_OPTIONS_TAB__
