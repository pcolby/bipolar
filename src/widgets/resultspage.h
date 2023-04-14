// SPDX-FileCopyrightText: 2014-2019 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: GPL-3.0-or-later

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
