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

#include <QApplication>
#include <QSettings>
#include <QTextEdit>

#define SETTINGS_GEOMETRY QLatin1String("geometry")

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
    // Set the main window title.
    setWindowTitle(tr("%1 %2")
        .arg(QApplication::applicationName())
        .arg(QStringList(QApplication::applicationVersion().split(QLatin1Char('.')).mid(0, 3)).join(QLatin1Char('.'))));

    log = new QTextEdit;
    log->setReadOnly(true);
    setCentralWidget(log);

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

void MainWindow::logMessage(QtMsgType type, const QMessageLogContext &context,
                            const QString &message, const QDateTime &time)
{
    if (log) {
        QString level(QLatin1String("invalid"));
        switch (type) {
        case QtDebugMsg:    level = QLatin1String("Debug");    break;
        case QtWarningMsg:  level = QLatin1String("Warning");  break;
        case QtCriticalMsg: level = QLatin1String("Critical"); break;
        case QtFatalMsg:    level = QLatin1String("Fatal");    break;
        }
        log->append(tr("%1 %2 %3").arg(time.toString()).arg(level).arg(message));
    }
}
