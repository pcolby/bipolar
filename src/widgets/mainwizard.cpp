/*
    Copyright 2014-2019 Paul Colby

    This file is part of Bipolar.

    Bipolar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Biplar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bipolar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwizard.h"
#include "inputspage.h"
#include "outputspage.h"
#include "resultspage.h"

#ifdef Q_OS_WIN
#include "os/flowsynchook.h"
#include <QMessageBox>
#endif

#include "os/versioninfo.h"

#include <QApplication>
#include <QTimer>

MainWizard::MainWizard(QWidget *parent, Qt::WindowFlags flags): QWizard(parent,flags) {
    setWindowTitle(tr("%1 %2")
        .arg(QApplication::applicationName())
        .arg(QStringList(QApplication::applicationVersion().split(QLatin1Char('.')).mid(0, 3)).join(QLatin1Char('.'))));
    const VersionInfo versionInfo;
    const QString specialBuild = (versionInfo.isValid()) ?
        versionInfo.fileInfo(QLatin1String("SpecialBuild")) : QString();
    if (!specialBuild.isEmpty()) {
        setWindowTitle(windowTitle() + QLatin1Char(' ') + specialBuild);
    }

    setOption(QWizard::NoBackButtonOnLastPage);
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    setOption(QWizard::NoCancelButtonOnLastPage);
    #endif

    addPage(new InputsPage());
    addPage(new OutputsPage());
    addPage(new ResultsPage());
}

void MainWizard::checkHook()
{
#ifdef Q_OS_WIN
    const QDir hookDir = FlowSyncHook::installableHookDir();
    const int availableVersion = FlowSyncHook::getVersion(hookDir);
    if (availableVersion <= 0) {
        return;
    }

    bool flowSyncDirFound = false;
    const QDir flowSyncDir = FlowSyncHook::flowSyncDir(&flowSyncDirFound);
    if ((!flowSyncDirFound) || (!flowSyncDir.exists())) {
        QMessageBox::information(this, QString(),
            tr("Unable to check if the Bipolar hook has been installed,\n"
               "because the Polar FlowSync application could not be located."));
    } else {
        const int installedVersion = FlowSyncHook::getVersion(flowSyncDir);

        QString message;
        if (installedVersion <= 0) {
            message = tr("The Bipolar hook does not appear to be installed.\n\n"
                         "Would you like to install it now?");
        } else if (installedVersion < availableVersion) {
            message = tr("This version of Bipolar includes a newer FlowSync hook.\n\n"
                         "Would you like to install it now?");
        }

        if ((!message.isEmpty()) &&
            (QMessageBox::question(this, QString(), message,
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::Yes
            ) && (!FlowSyncHook::install(hookDir, flowSyncDir))) {
            QMessageBox::warning(this, QString(),
                tr("Failed to install Bipolar hook into Polar FlowSync.\n\n"
                   "You may need to re-run this application as an administrator,\n"
                   "and/or exit Polar FlowSync before trying again.\n"
                   ));
        }
    }
#endif
}

void MainWizard::showEvent(QShowEvent * event)
{
    QWizard::showEvent(event);
    QTimer::singleShot(0, this, SLOT(checkHook()));
}
