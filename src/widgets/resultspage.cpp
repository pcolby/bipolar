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

#include "resultspage.h"

#include "converterthread.h"

#ifdef Q_OS_WIN
#include "os/flowsynchook.h"
#endif

#include "os/versioninfo.h"

#include <QApplication>
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

#if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
#define qInfo qDebug
#endif

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
    connect(this, SIGNAL(newMessage(QString,QColor)),
            this, SLOT(appendMessage(QString,QColor)), Qt::QueuedConnection);

    setLayout(vBox);
}

void ResultsPage::initializePage()
{
    // Register this instance as the Qt message handler.
    Q_ASSERT(instance == NULL);
    instance = this;
    previousMessageHandler = qInstallMessageHandler(&ResultsPage::messageHandler);

    // Debug log the application version information.
    VersionInfo versionInfo;
    if (versionInfo.isValid()) {
        qDebug() << QApplication::applicationName()
                 << versionInfo.fileVersionString()
                 << versionInfo.fileInfo(QLatin1String("SpecialBuild"));
    }

    // Debug log the hook version information.
#ifdef Q_OS_WIN
    for (int index = 0; index < 2; ++index) {
        const QDir dir = (index == 0) ?
            FlowSyncHook::flowSyncDir() : FlowSyncHook::installableHookDir();
        const int version = FlowSyncHook::getVersion(dir);
        qDebug() << QDir::toNativeSeparators(dir.absolutePath())
                 << "hook version" << version;
    }
#endif

    // Once the application is ready, begin the processing.
    QTimer::singleShot(0, converter, SLOT(start()));
}

bool ResultsPage::isComplete() const
{
    return !(converter && converter->isRunning() && converter->isCancelled());
}

bool ResultsPage::validatePage()
{
    if ((converter) && (converter->isRunning())) {
        qDebug() << "Processing cancelled.";
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
        QColor color;
        switch (type) {
        #if (QT_VERSION < QT_VERSION_CHECK(5, 5, 0))
        case QtDebugMsg:    color = message.startsWith(QLatin1Char('"')) ? Qt::gray : Qt::black; break;
        #else
        case QtDebugMsg:    color = Qt::gray;    break;
        case QtInfoMsg:     color = Qt::black;   break; // QtInfoMsg was added in Qt 5.5.
        #endif
        case QtWarningMsg:  color = Qt::magenta; break;
        case QtCriticalMsg: color = Qt::red;     break;
        case QtFatalMsg:    color = Qt::red;     break;
        }
        emit instance->newMessage(tr("%1 %2")
            .arg(QTime::currentTime().toString()).arg(message), color);
    }
}

// Protected slots.

void ResultsPage::appendMessage(const QString &message, const QColor &color)
{
    if (color.isValid()) {
        detailsBox->setTextColor(color);
    }
    detailsBox->append(message);
    if (detailsBox->verticalScrollBar()) {
        detailsBox->verticalScrollBar()->setValue(
            detailsBox->verticalScrollBar()->maximum());
    }
}

void ResultsPage::conversionFinished()
{
    if (converter->isCancelled()) {
        qDebug() << "Processing stopped.";
        setTitle(tr("Processing Cancelled"));
        setSubTitle(tr("Processing was cancelled at your request."));
        progressBar->setValue(progressBar->minimum());
        progressBar->setEnabled(false);
    } else {
        qDebug() << "Processing finished.";
        setTitle(tr("Processing Finished"));
        if ((converter->sessions.failed == 0) &&
            (converter->sessions.processed == 0)) {
            setSubTitle(tr("Found no new training sessions to process."));
        } else {
            setSubTitle(tr("Successfully processed %1 of %2 new training sessions.")
                        .arg(converter->sessions.processed)
                        .arg(converter->sessions.processed + converter->sessions.failed));
        }
        progressBar->setValue(progressBar->maximum());
        qInfo()  << tr("Skipped %1 training sessions processed previsouly.")
                    .arg(converter->sessions.skipped).toUtf8().constData();
        qInfo()  << tr("Wrote %1 of %2 files for %3 of %4 new training sessions.")
                    .arg(converter->files.written)
                    .arg(converter->files.written + converter->files.failed)
                    .arg(converter->sessions.processed)
                    .arg(converter->sessions.processed + converter->sessions.failed)
                    .toUtf8().constData();
    }
    setButtonText(QWizard::FinishButton, tr("Close"));
    emit completeChanged();
}

void ResultsPage::conversionStarted()
{
    qDebug() << "Processing started.";
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
