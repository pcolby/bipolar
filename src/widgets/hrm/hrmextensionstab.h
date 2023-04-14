// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __HRM_EXTENSIONS_TAB__
#define __HRM_EXTENSIONS_TAB__

#include <QWidget>

class QCheckBox;

class HrmExtensionsTab : public QWidget {
    Q_OBJECT

public:
    static const QString LapNamesExtSettingsKey;

    static const bool LapNamesExtDefaultSetting;

    HrmExtensionsTab(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

protected:
    QCheckBox * lapNamesExt;

};

#endif // __HRM_EXTENSIONS_TAB__
