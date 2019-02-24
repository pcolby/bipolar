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

#ifndef __RESULTS_PAGE__
#define __RESULTS_PAGE__

#include <QStringList>
#include <QWizardPage>

class ConverterThread;
class QProgressBar;
class QPushButton;
class QTextEdit;

class ResultsPage : public QWizardPage {
    Q_OBJECT

public:
    explicit ResultsPage(QWidget *parent=0);
    virtual void initializePage();
    virtual bool isComplete() const;
    virtual bool validatePage();

protected:
    static ResultsPage * instance;
    QtMessageHandler previousMessageHandler;
    QProgressBar * progressBar;
    QPushButton * showDetailsButton;
    QTextEdit * detailsBox;
    ConverterThread * converter;

    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &message);

protected slots:
    void appendMessage(const QString &message, const QColor &color);
    void conversionFinished();
    void conversionStarted();
    void sessionStarted(const int index);
    void showDetails();

signals:
    void newMessage(const QString &message, const QColor &color);

};

#endif // __RESULTS_PAGE__
