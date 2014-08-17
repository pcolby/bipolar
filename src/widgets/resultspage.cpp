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
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QTime>
#include <QTextEdit>
#include <QVBoxLayout>

ResultsPage * ResultsPage::instance = NULL;

ResultsPage::ResultsPage(QWidget *parent)
    : QWizardPage(parent), previousMessageHandler(NULL)
{
    setTitle(tr("Converting..."));

    progressBar = new QProgressBar();

    showDetailsButton = new QPushButton(tr("Show details"));

    detailsBox = new QTextEdit();
    detailsBox->setReadOnly(true);
   //detailsBox->setVisible(false); /// @todo Reinstate this post-dev.
    connect(showDetailsButton, SIGNAL(clicked()), this, SLOT(showDetails()));

    QVBoxLayout * const vBox = new QVBoxLayout;
    vBox->addWidget(progressBar);
    vBox->addWidget(showDetailsButton);
    vBox->addWidget(detailsBox);
    setLayout(vBox);

    converter = new ConverterThread;
    connect(converter, SIGNAL(finished()), this, SLOT(conversionFinished()));
    connect(converter, SIGNAL(progress(int)), progressBar, SLOT(setValue(int)));
    connect(converter, SIGNAL(sessionBaseNamesChanged(int)), progressBar, SLOT(setMaximum(int)));
    connect(converter, SIGNAL(started()), this, SLOT(conversionStarted()));
}

void ResultsPage::initializePage()
{
    Q_ASSERT(instance == NULL);
    instance = this;
    previousMessageHandler = qInstallMessageHandler(&ResultsPage::messageHandler);

    qDebug() << sessionBaseNames.size() << "training sessions to examine";
    progressBar->setRange(0, sessionBaseNames.size());
    progressBar->reset();

    converter->start();
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

void ResultsPage::messageHandler(QtMsgType type,
                                 const QMessageLogContext &context,
                                 const QString &message)
{
    Q_ASSERT(instance != NULL);
    if (instance) {
        instance->onMessage(type, context, message);
    }
}

void ResultsPage::onMessage(QtMsgType type, const QMessageLogContext &context,
                            const QString &message)
{
    Q_UNUSED(context)
    QString level(QLatin1String("invalid"));
    switch (type) {
    case QtDebugMsg:    level = QLatin1String("Debug");    break;
    case QtWarningMsg:  level = QLatin1String("Warning");  break;
    case QtCriticalMsg: level = QLatin1String("Critical"); break;
    case QtFatalMsg:    level = QLatin1String("Fatal");    break;
    }
    detailsBox->append(tr("%1 %2 %3")
        .arg(QTime::currentTime().toString())
        .arg(level).arg(message));
    if (detailsBox->verticalScrollBar()) {
        detailsBox->verticalScrollBar()->setValue(
            detailsBox->verticalScrollBar()->maximum());
    }
}

// Protected slots.

void ResultsPage::conversionFinished()
{
    if (converter->isCancelled()) {
        qDebug() << "conversion stopped";
        setTitle(tr("Conversion Cancelled"));
        progressBar->setValue(progressBar->minimum());
        progressBar->setEnabled(false);
    } else {
        qDebug() << "conversion finished";
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

void ResultsPage::showDetails()
{
    showDetailsButton->setVisible(false);
    detailsBox->setVisible(true);
}
