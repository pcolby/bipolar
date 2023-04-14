// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __GENERAL_HRM_OPTIONS_TAB__
#define __GENERAL_HRM_OPTIONS_TAB__

#include <QWidget>

class QCheckBox;

class GeneralHrmOptions : public QWidget {
    Q_OBJECT

public:
    static const QString ExportRrFilesSettingsKey;

    static const bool ExportRrFilesDefaultSetting;

    GeneralHrmOptions(QWidget *parent=0, Qt::WindowFlags flags=Qt::WindowFlags());

public slots:
    void load();
    void save();

protected:
    QCheckBox * rrFiles;

};

#endif // __GENERAL_HRM_OPTIONS_TAB__
