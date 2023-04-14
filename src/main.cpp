// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwizard.h"
#include "os/versioninfo.h"

#ifdef Q_OS_WIN
#include "os/flowsynchook.h"
#include <QErrorMessage>
#endif

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QTranslator>

// Note, these values are used by the QSettings default constructor,
// so should not change between versions.
#define APPLICATION_NAME    QLatin1String("Bipolar")
#define ORGANISATION_NAME   QLatin1String("Paul Colby")
#define ORGANISATION_DOMAIN QLatin1String("bipolar.colby.id.au")

int main(int argc, char *argv[]) {
    // Setup the primary Qt application object.
    QApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setOrganizationName(ORGANISATION_NAME);
    app.setOrganizationDomain(ORGANISATION_DOMAIN);
    VersionInfo versionInfo;
    if (versionInfo.isValid()) {
        app.setApplicationVersion(versionInfo.fileVersionString());
    }

    // Default to the Oxygen theme, if no other theme is configured yet.
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName(QLatin1String("oxygen"));
    }

    // Try to load a localised translator.
    QTranslator translator;
    if (translator.load(QLocale::system().name(),app.applicationDirPath()+QLatin1String("/../i18n")))
        app.installTranslator(&translator);

#ifdef Q_OS_WIN
    // Install the hook now, if requested via the command line.
    if (app.arguments().contains(QLatin1Literal("-install-hook"))) {
        QErrorMessage::qtHandler(); // Expose any internal errors / warnings.
        const QDir fromDir = FlowSyncHook::installableHookDir();
        bool toDirFound = false;
        const QDir toDir = FlowSyncHook::flowSyncDir(&toDirFound);
        if (!fromDir.exists(QLatin1String("Qt5Network.dll"))) {
            QMessageBox::warning(NULL, QString(),
                app.tr("Installable hook not found."));
            return 1;
        }
        if (!toDirFound) {
            QMessageBox::warning(NULL, QString(),
                app.tr("Failed to locate Polar FlowSync installation."));
            return 2;
        }
        if (!toDir.exists(QLatin1String("Qt5Network.dll"))) {
            QMessageBox::warning(NULL, QString(),
                app.tr("Failed to locate network library in Polar FlowSync installation."));
            return 3;
        }
        const int fromVersion = FlowSyncHook::getVersion(fromDir);
        const int toVersion = FlowSyncHook::getVersion(toDir);
        if (fromVersion > toVersion) {
            if (!FlowSyncHook::install(fromDir, toDir)) {
                QMessageBox::warning(NULL, QString(),
                    app.tr("Failed to install hook DLL."));
                return 3;
            }
        }
        return 0;
    }
#endif

    // Instantiate the main window.
    MainWizard mainWizard;
    mainWizard.show();
    return app.exec();
}
