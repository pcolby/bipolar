// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __TCX_EXTENSIONS_TAB__
#define __TCX_EXTENSIONS_TAB__

#include <QWidget>

class QCheckBox;

class TcxExtensionsTab : public QWidget {
    Q_OBJECT

public:
    static const QString GarminActivityExtSettingsKey;
    static const QString GarminCourseExtSettingsKey;

    static const bool GarminActivityExtDefaultSetting;
    static const bool GarminCourseExtDefaultSetting;

    TcxExtensionsTab(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

protected:
    QCheckBox * garminActivityExt;
    QCheckBox * garminCourseExt;

};

#endif // __TCX_EXTENSIONS_TAB__
