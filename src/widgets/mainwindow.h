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

#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QDateTime>
#include <QMainWindow>

class QTextEdit;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent=0, Qt::WindowFlags flags=0);

    void logMessage(QtMsgType type, const QMessageLogContext &context,
                    const QString &message,
                    const QDateTime &time = QDateTime::currentDateTime());

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QTextEdit *log;

};

#endif // __MAIN_WINDOW_H__
