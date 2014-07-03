#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

#ifdef DEBUG
#define VERSION_STRING_REGEX "([^.]+\\.[^.]+\\.[^.]+\\.[^.]+)"
#else
#define VERSION_STRING_REGEX "([^.]+\\.[^.]+\\.[^.]+)\\."
#endif // DEBUG

#define SETTINGS_GEOMETRY QLatin1String("geometry")

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
    // Set the main window title.
    QRegExp versionMatch(QLatin1String(VERSION_STRING_REGEX));
    const QString versionStr=(QApplication::applicationVersion().contains(versionMatch)) ? versionMatch.cap(1) : QString::null;
    setWindowTitle(tr("%1 %2").arg(QApplication::applicationName()).arg(versionStr));

    /// @todo Build the UI.

    // Restore the window's previous size and position.
    QSettings settings;
    QVariant geometry=settings.value(SETTINGS_GEOMETRY);
    if (geometry.isValid()) restoreGeometry(geometry.toByteArray());
    else setGeometry(40,40,800,550); // Default to 800x550, at position (40,40).
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Save the window's current size and position.
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY,saveGeometry());
    QMainWindow::closeEvent(event);
}
