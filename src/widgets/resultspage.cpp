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

#include "resultspage.h"

#include "converterthread.h"

#include <QDebug>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QTime>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

ResultsPage * ResultsPage::instance = NULL;

ResultsPage::ResultsPage(QWidget *parent)
    : QWizardPage(parent), previousMessageHandler(NULL)
{
    setTitle(tr("Processing Training Sessions..."));
    setSubTitle(tr("Processing will begin in a moment."));

    QVBoxLayout * const vBox = new QVBoxLayout;

    progressBar = new QProgressBar();
    vBox->addWidget(progressBar);

    {
        QHBoxLayout * const hBox = new QHBoxLayout();
        showDetailsButton = new QPushButton(tr("Show details"));
        hBox->addWidget(showDetailsButton);
        hBox->addStretch();
        vBox->addItem(hBox);
    }

    detailsBox = new QTextEdit();
    detailsBox->setReadOnly(true);
    detailsBox->setVisible(false);
    connect(showDetailsButton, SIGNAL(clicked()), this, SLOT(showDetails()));
    vBox->addWidget(detailsBox);

    converter = new ConverterThread;
    connect(converter, SIGNAL(finished()), this, SLOT(conversionFinished()));
    connect(converter, SIGNAL(progress(int)), this, SLOT(sessionStarted(int)));
    connect(converter, SIGNAL(progress(int)), progressBar, SLOT(setValue(int)));
    connect(converter, SIGNAL(sessionBaseNamesChanged(int)), progressBar, SLOT(setMaximum(int)));
    connect(converter, SIGNAL(started()), this, SLOT(conversionStarted()));

    // This signal/slot indirection pipes all log events to the GUI thread.
    connect(this, SIGNAL(newMessage(QString)),
            this, SLOT(appendMessage(QString)), Qt::QueuedConnection);

    setLayout(vBox);
}

void ResultsPage::initializePage()
{
    Q_ASSERT(instance == NULL);
    instance = this;
    previousMessageHandler = qInstallMessageHandler(&ResultsPage::messageHandler);
    QTimer::singleShot(0, converter, SLOT(start()));
}

bool ResultsPage::isComplete() const
{
    return !(converter && converter->isRunning() && converter->isCancelled());
}

bool ResultsPage::validatePage()
{
    if ((converter) && (converter->isRunning())) {
        qDebug() << "conversion cancelled";
        converter->cancel();
        emit completeChanged();
        return false;
    }
    return true;
}

// Protected methods.

/**
 * @note This static function will be called by Qt from multiple threads. This
 *       function then redirects calls to current ResultsPage instance by
 *       emitting the newMessage signal. However, since this signal is being
 *       emitted from a thread that is not necessarily the one the instance
 *       lives in, any slots connected to the newMessage signal will need
 *       to specifiy Qt::QueuedConnection, since Qt::AutoConnection detection
 *       will examine the instance's thread, not the true current thread.
 */
void ResultsPage::messageHandler(QtMsgType type,
                                 const QMessageLogContext &context,
                                 const QString &message)
{
    Q_UNUSED(context)
    Q_ASSERT(instance != NULL);
    if (instance) {
        QString level(QLatin1String("invalid"));
        switch (type) {
        case QtDebugMsg:    level = QLatin1String("Debug");    break;
        case QtWarningMsg:  level = QLatin1String("Warning");  break;
        case QtCriticalMsg: level = QLatin1String("Critical"); break;
        case QtFatalMsg:    level = QLatin1String("Fatal");    break;
        }
        emit instance->newMessage(tr("%1 %2 %3")
            .arg(QTime::currentTime().toString())
            .arg(level).arg(message));
    }
}

// Protected slots.

void ResultsPage::appendMessage(const QString &message)
{
    detailsBox->append(message);
    if (detailsBox->verticalScrollBar()) {
        detailsBox->verticalScrollBar()->setValue(
            detailsBox->verticalScrollBar()->maximum());
    }
}

void ResultsPage::conversionFinished()
{
    if (converter->isCancelled()) {
        qDebug() << "processing stopped";
        setTitle(tr("Processing Cancelled"));
        setSubTitle(tr("Processing was cancelled at your request."));
        progressBar->setValue(progressBar->minimum());
        progressBar->setEnabled(false);
    } else {
        qDebug() << "processing finished";
        setTitle(tr("Processing Finished"));
        /// @todo Indicate if there were any conversion errors here.
        setSubTitle(tr("Processing completed successfully."));
        progressBar->setValue(progressBar->maximum());
    }
    setButtonText(QWizard::FinishButton, tr("Close"));
    emit completeChanged();
}

void ResultsPage::conversionStarted()
{
    qDebug() << "conversion started";
    setButtonText(QWizard::FinishButton, tr("Cancel"));
    emit completeChanged();
}

void ResultsPage::sessionStarted(const int index)
{
    Q_ASSERT(converter != NULL);
    Q_ASSERT(index < converter->sessionBaseNames().size());
    setSubTitle(QFileInfo(converter->sessionBaseNames().at(index)).fileName());
}

void ResultsPage::showDetails()
{
    showDetailsButton->setVisible(false);
    detailsBox->setVisible(true);
}
