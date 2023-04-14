// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GPX_EXTENSIONS_TAB__
#define __GPX_EXTENSIONS_TAB__

#include <QWidget>

class QCheckBox;

class GpxExtensionsTab : public QWidget {
    Q_OBJECT

public:
    static const QString CluetrustGpxExtSettingsKey;
    static const QString GarminAccelerationExtSettingsKey;
    static const QString GarminTrackPointExtSettingsKey;

    static const bool CluetrustGpxExtDefaultSetting;
    static const bool GarminAccelerationExtDefaultSetting;
    static const bool GarminTrackPointExtDefaultSetting;

    GpxExtensionsTab(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

protected:
    QCheckBox * cluetrustGpxExt;
    QCheckBox * garminAccelerationExt;
    QCheckBox * garminTrackPointExt;

};

#endif // __GPX_EXTENSIONS_TAB__
