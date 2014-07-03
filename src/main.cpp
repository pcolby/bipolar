/// @todo Add license.

#include "mainwindow.h"

#include <QApplication>
#include <QErrorMessage>
#include <QTranslator>

// Note, these values are used by the QSettings default constructor,
// so hould not change between versions.
#define APPLICATION_NAME    QLatin1String("Bipolar")
#define ORGANISATION_NAME   QLatin1String("Paul Colby")
#define ORGANISATION_DOMAIN QLatin1String("bipolar.colby.id.au")

int main(int argc, char *argv[]) {
    // Setup the primary Qt application object.
    QApplication app(argc, argv);
    app.setApplicationName(APPLICATION_NAME);
    app.setOrganizationName(ORGANISATION_NAME);
    app.setOrganizationDomain(ORGANISATION_DOMAIN);
  //app.setApplicationVersion(VersionInfo::getAppVersionStr());

    // Install the QErrorMessage class' Qt message handler.
    QErrorMessage::qtHandler();

    // Try to load a localised translator.
    QTranslator translator;
    if (translator.load(QLocale::system().name(),app.applicationDirPath()+QLatin1String("/../i18n")))
        app.installTranslator(&translator);

    // Instantiate the main window.
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
