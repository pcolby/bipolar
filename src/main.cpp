/*
    Copyright 2014 Paul Colby

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

#include "mainwindow.h"
#include "os/fileversioninfo.h"

#include <QApplication>
#include <QDebug>
#include <QErrorMessage>
#include <QTranslator>

// Note, these values are used by the QSettings default constructor,
// so hould not change between versions.
#define APPLICATION_NAME    QLatin1String("Bipolar")
#define ORGANISATION_NAME   QLatin1String("Paul Colby")
#define ORGANISATION_DOMAIN QLatin1String("bipolar.colby.id.au")

MainWindow * mainWindow = NULL;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (mainWindow) {
        mainWindow->logMessage(type, context, msg);
    }
}

int main(int argc, char *argv[]) {
    // Setup the primary Qt application object.
    QApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setOrganizationName(ORGANISATION_NAME);
    app.setOrganizationDomain(ORGANISATION_DOMAIN);
#ifdef Q_OS_WIN
    FileVersionInfo versionInfo;
    if (versionInfo.isValid()) {
        app.setApplicationVersion(versionInfo.fileVersionStrings().join(QLatin1Char('.')));
    }
#endif

    // Install the QErrorMessage class' Qt message handler.
    //QErrorMessage::qtHandler();
    qInstallMessageHandler(messageHandler);

    // Try to load a localised translator.
    QTranslator translator;
    if (translator.load(QLocale::system().name(),app.applicationDirPath()+QLatin1String("/../i18n")))
        app.installTranslator(&translator);

    // Instantiate the main window.
    mainWindow = new MainWindow;
    qDebug() << QApplication::applicationName() << QApplication::applicationVersion();
    mainWindow->show();
    return app.exec();
}
